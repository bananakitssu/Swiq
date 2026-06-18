#include "interpreter.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>

void Interpreter::run(const std::vector<std::unique_ptr<Stmt>>& statements) {
    registerFunctions(statements);
    executeBlock(statements);
}

void Interpreter::registerFunctions(const std::vector<std::unique_ptr<Stmt>>& statements) {
    for (const auto& stmt : statements) {
        if (auto func = dynamic_cast<const FuncDeclStmt*>(stmt.get())) {
            functions[func->name] = func;
        }
    }
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
                "Interpreter error at line " + std::to_string(assign->line) +
                ": cannot assign to undeclared variable '" + assign->name +
                "'. Use 'set var' to declare it first.");
        }
        variables[assign->name] = evaluate(assign->value.get());
        return;
    }

    if (auto idxAssign = dynamic_cast<const IndexAssignStmt*>(stmt)) {
        Value arrVal = evaluate(idxAssign->array.get());
        auto arrPtr = std::get_if<std::shared_ptr<ArrayObject>>(&arrVal.data);
        if (!arrPtr) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(idxAssign->line) +
                                      ": cannot index into a non-array value");
        }
        Value idxVal = evaluate(idxAssign->index.get());
        auto idxNum = std::get_if<long long>(&idxVal.data);
        if (!idxNum) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(idxAssign->line) +
                                      ": array index must be a number");
        }
        auto& elements = (*arrPtr)->elements;
        if (*idxNum < 0 || (size_t)*idxNum >= elements.size()) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(idxAssign->line) +
                                      ": array index out of bounds");
        }
        elements[(size_t)*idxNum] = evaluate(idxAssign->value.get());
        return;
    }

    if (auto log = dynamic_cast<const LogStmt*>(stmt)) {
        printValue(evaluate(log->value.get()));
        std::cout << std::endl;
        return;
    }

    if (auto exprStmt = dynamic_cast<const ExprStmt*>(stmt)) {
        evaluate(exprStmt->expr.get());
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

    if (auto whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
        while (isTruthy(evaluate(whileStmt->condition.get()))) {
            executeBlock(whileStmt->body);
        }
        return;
    }

    if (auto forStmt = dynamic_cast<const ForStmt*>(stmt)) {
        execute(forStmt->init.get());
        while (isTruthy(evaluate(forStmt->condition.get()))) {
            executeBlock(forStmt->body);
            execute(forStmt->post.get());
        }
        return;
    }

    if (dynamic_cast<const FuncDeclStmt*>(stmt)) {
        // Already registered in registerFunctions(); nothing to do at execution time.
        return;
    }

    if (auto retStmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        Value val = retStmt->value ? evaluate(retStmt->value.get()) : Value();
        throw ReturnSignal{val};
    }

    throw std::runtime_error("Interpreter error: unknown statement type");
}

void Interpreter::printValue(const Value& value) {
    std::cout << valueToDisplayString(value);
}

// Builds a dotted path string like "Swiq.__ENV__.__VERSION__.__BUILD_NUMBER__"
std::string Interpreter::getMemberPath(const Expr* expr) {
    if (auto var = dynamic_cast<const VariableExpr*>(expr)) {
        return var->name;
    }
    if (auto mem = dynamic_cast<const MemberExpr*>(expr)) {
        return getMemberPath(mem->object.get()) + "." + mem->property;
    }
    throw std::runtime_error("Interpreter error: invalid member access expression");
}

Value Interpreter::callBuiltin(const std::string& name, std::vector<Value>& args, int line) {
    if (name == "len") {
        if (args.size() != 1) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": len() expects exactly 1 argument");
        }
        if (auto a = std::get_if<std::shared_ptr<ArrayObject>>(&args[0].data)) {
            return Value((long long)(*a)->elements.size());
        }
        if (auto s = std::get_if<std::string>(&args[0].data)) {
            return Value((long long)s->size());
        }
        throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                  ": len() requires an array or string");
    }

    if (name == "push") {
        if (args.size() != 2) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": push() expects exactly 2 arguments (array, value)");
        }
        auto a = std::get_if<std::shared_ptr<ArrayObject>>(&args[0].data);
        if (!a) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": push() requires an array as its first argument");
        }
        if ((*a)->fixed) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": push() cannot grow a fixed-size array created with AllocatedArray()");
        }
        (*a)->elements.push_back(args[1]);
        return Value((long long)(*a)->elements.size());
    }

    if (name == "AllocatedArray") {
        if (args.size() != 1) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": AllocatedArray() expects exactly 1 argument (size)");
        }
        auto sizeNum = std::get_if<long long>(&args[0].data);
        if (!sizeNum) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": AllocatedArray() requires a number as its size");
        }
        if (*sizeNum < 0) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": AllocatedArray() size cannot be negative");
        }
        auto obj = std::make_shared<ArrayObject>();
        obj->fixed = true;
        obj->elements.assign((size_t)*sizeNum, Value()); // every slot starts as null
        return Value(obj);
    }

    if (name == "readFile") {
        if (args.size() != 1) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": readFile() expects exactly 1 argument (path)");
        }
        auto path = std::get_if<std::string>(&args[0].data);
        if (!path) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": readFile() requires a string path");
        }
        std::ifstream file(*path);
        if (!file) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": could not open file '" + *path + "'");
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return Value(buffer.str());
    }

    throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                              ": unknown function '" + name + "'");
}

Value Interpreter::callFunction(const FuncDeclStmt* func, std::vector<Value> args, int callLine) {
    if (args.size() != func->params.size()) {
        throw std::runtime_error("Interpreter error at line " + std::to_string(callLine) +
                                  ": function '" + func->name + "' expects " +
                                  std::to_string(func->params.size()) + " argument(s), got " +
                                  std::to_string(args.size()));
    }

    // Functions run in a fresh, isolated scope. Only parameters and any
    // explicitly listed [captures] are visible — everything else from the
    // outer scope is invisible by default.
    auto savedVars = std::move(variables);
    variables = std::unordered_map<std::string, Value>();

    // Bring in captured variables by their current value.
    for (const auto& name : func->captures) {
        auto it = savedVars.find(name);
        if (it == savedVars.end()) {
            variables = std::move(savedVars); // restore before throwing
            throw std::runtime_error("Interpreter error at line " + std::to_string(callLine) +
                                      ": function '" + func->name + "' captures undefined variable '" +
                                      name + "'");
        }
        variables[name] = it->second;
    }

    // Bind parameters (params take priority if a name overlaps a capture).
    for (size_t i = 0; i < func->params.size(); i++) {
        variables[func->params[i]] = args[i];
    }

    Value result;
    try {
        executeBlock(func->body);
        result = Value(); // no explicit return statement was hit
    } catch (ReturnSignal& r) {
        result = r.value;
    } catch (...) {
        variables = std::move(savedVars);
        throw;
    }

    // Write captured variables' final values back to the outer scope —
    // this is what makes them behave like real (if limited) closures.
    for (const auto& name : func->captures) {
        savedVars[name] = variables[name];
    }

    variables = std::move(savedVars);
    return result;
}

Value Interpreter::evaluate(const Expr* expr) {
    if (auto num = dynamic_cast<const NumberExpr*>(expr)) {
        return Value(num->value);
    }

    if (auto str = dynamic_cast<const StringExpr*>(expr)) {
        return Value(str->value);
    }

    if (auto boolean = dynamic_cast<const BoolExpr*>(expr)) {
        return Value(boolean->value);
    }

    if (dynamic_cast<const NullExpr*>(expr)) {
        return Value(); // null / none / nil all evaluate to the same thing
    }

    if (auto hex = dynamic_cast<const HexExpr*>(expr)) {
        // Hex literals are stored as plain text (e.g. "0x1F"), not a number.
        // This is intentional: + on them concatenates like any other string,
        // and arithmetic on them fails until .ConvertToNumber() is called.
        return Value(hex->raw);
    }

    if (auto arr = dynamic_cast<const ArrayExpr*>(expr)) {
        auto obj = std::make_shared<ArrayObject>();
        for (const auto& elemExpr : arr->elements) {
            obj->elements.push_back(evaluate(elemExpr.get()));
        }
        return Value(obj); // fixed defaults to false: [...] literals are growable
    }

    if (auto idx = dynamic_cast<const IndexExpr*>(expr)) {
        Value arrVal = evaluate(idx->array.get());
        auto arrPtr = std::get_if<std::shared_ptr<ArrayObject>>(&arrVal.data);
        if (!arrPtr) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(idx->line) +
                                      ": cannot index into a non-array value");
        }
        Value idxVal = evaluate(idx->index.get());
        auto idxNum = std::get_if<long long>(&idxVal.data);
        if (!idxNum) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(idx->line) +
                                      ": array index must be a number");
        }
        auto& elements = (*arrPtr)->elements;
        if (*idxNum < 0 || (size_t)*idxNum >= elements.size()) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(idx->line) +
                                      ": array index out of bounds");
        }
        return elements[(size_t)*idxNum];
    }

    if (auto mcall = dynamic_cast<const MethodCallExpr*>(expr)) {
        Value obj = evaluate(mcall->object.get());

        if (mcall->method == "ConvertToNumber") {
            if (!mcall->args.empty()) {
                throw std::runtime_error("Interpreter error at line " + std::to_string(mcall->line) +
                                          ": ConvertToNumber() takes no arguments");
            }
            auto s = std::get_if<std::string>(&obj.data);
            if (!s) {
                throw std::runtime_error("Interpreter error at line " + std::to_string(mcall->line) +
                                          ": ConvertToNumber() can only be called on a hex (or numeric string) value");
            }
            try {
                long long parsed;
                if (s->size() > 2 && s->rfind("0x", 0) == 0) {
                    parsed = std::stoll(s->substr(2), nullptr, 16);
                } else if (s->size() > 2 && s->rfind("0X", 0) == 0) {
                    parsed = std::stoll(s->substr(2), nullptr, 16);
                } else {
                    parsed = std::stoll(*s, nullptr, 10);
                }
                return Value(parsed);
            } catch (const std::exception&) {
                throw std::runtime_error("Interpreter error at line " + std::to_string(mcall->line) +
                                          ": '" + *s + "' is not a valid number for ConvertToNumber()");
            }
        }

        throw std::runtime_error("Interpreter error at line " + std::to_string(mcall->line) +
                                  ": unknown method '" + mcall->method + "'");
    }

    if (auto call = dynamic_cast<const CallExpr*>(expr)) {
        std::vector<Value> args;
        for (const auto& argExpr : call->args) {
            args.push_back(evaluate(argExpr.get()));
        }

        if (call->callee == "len" || call->callee == "push" || call->callee == "AllocatedArray" ||
            call->callee == "readFile") {
            return callBuiltin(call->callee, args, call->line);
        }

        auto it = functions.find(call->callee);
        if (it == functions.end()) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(call->line) +
                                      ": undefined function '" + call->callee + "'");
        }
        return callFunction(it->second, std::move(args), call->line);
    }

    if (auto var = dynamic_cast<const VariableExpr*>(expr)) {
        auto it = variables.find(var->name);
        if (it == variables.end()) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(var->line) +
                                      ": undefined variable '" + var->name + "'");
        }
        return it->second;
    }

    if (auto mem = dynamic_cast<const MemberExpr*>(expr)) {
        std::string path = getMemberPath(mem);

        // Read-only built-in environment values. Not stored in `variables`,
        // so they can never be assigned to — the grammar for 'set' only
        // allows a plain identifier or array index on the left, never a
        // dotted member path.
        static const std::unordered_map<std::string, Value> envMap = {
            {"Swiq.__ENV__.__VERSION__.__BUILD_NUMBER__", Value((long long)1)},
            {"Swiq.__ENV__.__VERSION__.__BUILD_YEAR__", Value((long long)2026)},
        };

        auto it = envMap.find(path);
        if (it != envMap.end()) {
            return it->second;
        }

        throw std::runtime_error("Interpreter error at line " + std::to_string(mem->line) +
                                  ": unknown or inaccessible property '" + path + "'");
    }

    if (auto bin = dynamic_cast<const BinaryExpr*>(expr)) {
        Value left = evaluate(bin->left.get());
        Value right = evaluate(bin->right.get());
        const std::string& op = bin->op;

        if (op == "+" && (std::holds_alternative<std::string>(left.data) ||
                          std::holds_alternative<std::string>(right.data))) {
            return Value(valueToDisplayString(left) + valueToDisplayString(right));
        }

        if (op == "==" || op == "!=") {
            bool equal;
            if (left.data.index() != right.data.index()) {
                equal = false;
            } else if (std::holds_alternative<long long>(left.data)) {
                equal = std::get<long long>(left.data) == std::get<long long>(right.data);
            } else if (std::holds_alternative<std::string>(left.data)) {
                equal = std::get<std::string>(left.data) == std::get<std::string>(right.data);
            } else if (std::holds_alternative<bool>(left.data)) {
                equal = std::get<bool>(left.data) == std::get<bool>(right.data);
            } else if (std::holds_alternative<std::monostate>(left.data)) {
                equal = true; // null == null (and none/nil, since they're all the same value)
            } else {
                // Arrays compare by identity (same underlying array)
                equal = std::get<std::shared_ptr<ArrayObject>>(left.data) ==
                        std::get<std::shared_ptr<ArrayObject>>(right.data);
            }
            return Value(op == "==" ? equal : !equal);
        }

        if (!std::holds_alternative<long long>(left.data) || !std::holds_alternative<long long>(right.data)) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(bin->line) +
                                      ": operator '" + op + "' requires numbers");
        }

        long long l = std::get<long long>(left.data);
        long long r = std::get<long long>(right.data);

        if (op == "+") return Value(l + r);
        if (op == "-") return Value(l - r);
        if (op == "*") return Value(l * r);
        if (op == "/") {
            if (r == 0) throw std::runtime_error("Interpreter error at line " + std::to_string(bin->line) +
                                                   ": division by zero");
            return Value(l / r);
        }
        if (op == "<")  return Value(l < r);
        if (op == ">")  return Value(l > r);
        if (op == "<=") return Value(l <= r);
        if (op == ">=") return Value(l >= r);

        throw std::runtime_error("Interpreter error at line " + std::to_string(bin->line) +
                                  ": unknown operator '" + op + "'");
    }

    throw std::runtime_error("Interpreter error: unknown expression type");
}
