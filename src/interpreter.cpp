#include "interpreter.h"
#include <iostream>
#include <stdexcept>

void Interpreter::run(const std::vector<std::unique_ptr<Stmt>>& statements) {
    executeBlock(statements);
}

void Interpreter::executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements) {
    for (const auto& stmt : statements) {
        execute(stmt.get());
    }
}

void Interpreter::execute(const Stmt* stmt) {
    if (auto varDecl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        variables[varDecl->name] = evaluate(varDecl->value.get());
        return;
    }

    if (auto assign = dynamic_cast<const AssignStmt*>(stmt)) {
        if (variables.find(assign->name) == variables.end()) {
            throw std::runtime_error(
                "Interpreter error: cannot assign to undeclared variable '" + assign->name +
                "'. Use 'set var' to declare it first.");
        }
        variables[assign->name] = evaluate(assign->value.get());
        return;
    }

    if (auto log = dynamic_cast<const LogStmt*>(stmt)) {
        printValue(evaluate(log->value.get()));
        std::cout << std::endl;
        return;
    }

    if (auto ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
        if (isTruthy(evaluate(ifStmt->condition.get()))) {
            executeBlock(ifStmt->thenBranch);
        } else {
            executeBlock(ifStmt->elseBranch);
        }
        return;
    }

    throw std::runtime_error("Interpreter error: unknown statement type");
}

bool Interpreter::isTruthy(const Value& value) {
    if (auto b = std::get_if<bool>(&value)) return *b;
    if (auto i = std::get_if<long long>(&value)) return *i != 0;
    if (auto s = std::get_if<std::string>(&value)) return !s->empty();
    return false;
}

void Interpreter::printValue(const Value& value) {
    if (auto i = std::get_if<long long>(&value)) {
        std::cout << *i;
    } else if (auto s = std::get_if<std::string>(&value)) {
        std::cout << *s;
    } else if (auto b = std::get_if<bool>(&value)) {
        std::cout << (*b ? "true" : "false");
    }
}

Value Interpreter::evaluate(const Expr* expr) {
    if (auto num = dynamic_cast<const NumberExpr*>(expr)) {
        return num->value;
    }

    if (auto str = dynamic_cast<const StringExpr*>(expr)) {
        return str->value;
    }

    if (auto var = dynamic_cast<const VariableExpr*>(expr)) {
        auto it = variables.find(var->name);
        if (it == variables.end()) {
            throw std::runtime_error("Interpreter error: undefined variable '" + var->name + "'");
        }
        return it->second;
    }

    if (auto bin = dynamic_cast<const BinaryExpr*>(expr)) {
        Value left = evaluate(bin->left.get());
        Value right = evaluate(bin->right.get());
        const std::string& op = bin->op;

        // String concatenation: if either side is a string and op is '+'
        if (op == "+" && (std::holds_alternative<std::string>(left) ||
                          std::holds_alternative<std::string>(right))) {
            auto toStr = [](const Value& v) -> std::string {
                if (auto s = std::get_if<std::string>(&v)) return *s;
                if (auto i = std::get_if<long long>(&v)) return std::to_string(*i);
                if (auto b = std::get_if<bool>(&v)) return *b ? "true" : "false";
                return "";
            };
            return toStr(left) + toStr(right);
        }

        // Equality: works on any matching types
        if (op == "==" || op == "!=") {
            bool equal;
            if (left.index() != right.index()) {
                equal = false;
            } else if (std::holds_alternative<long long>(left)) {
                equal = std::get<long long>(left) == std::get<long long>(right);
            } else if (std::holds_alternative<std::string>(left)) {
                equal = std::get<std::string>(left) == std::get<std::string>(right);
            } else {
                equal = std::get<bool>(left) == std::get<bool>(right);
            }
            return op == "==" ? equal : !equal;
        }

        // Everything else (+, -, *, /, <, >, <=, >=) requires numbers
        if (!std::holds_alternative<long long>(left) || !std::holds_alternative<long long>(right)) {
            throw std::runtime_error("Interpreter error: operator '" + op + "' requires numbers");
        }

        long long l = std::get<long long>(left);
        long long r = std::get<long long>(right);

        if (op == "+") return l + r;
        if (op == "-") return l - r;
        if (op == "*") return l * r;
        if (op == "/") {
            if (r == 0) throw std::runtime_error("Interpreter error: division by zero");
            return l / r;
        }
        if (op == "<")  return l < r;
        if (op == ">")  return l > r;
        if (op == "<=") return l <= r;
        if (op == ">=") return l >= r;

        throw std::runtime_error("Interpreter error: unknown operator '" + op + "'");
    }

    throw std::runtime_error("Interpreter error: unknown expression type");
}
