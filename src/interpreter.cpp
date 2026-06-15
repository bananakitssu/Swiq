// interpreter.cpp
#include "interpreter.h"
#include <iostream>
#include <stdexcept>

void Interpreter::run(const std::vector<std::unique_ptr<Stmt>>& statements) {
    for (const auto& stmt : statements) {
        execute(stmt.get());
    }
}

void Interpreter::execute(const Stmt* stmt) {
    if (auto varDecl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        int value = evaluate(varDecl->value.get());
        variables[varDecl->name] = value;
        return;
    }

    if (auto log = dynamic_cast<const LogStmt*>(stmt)) {
        int value = evaluate(log->value.get());
        std::cout << value << std::endl;
        return;
    }

    throw std::runtime_error("Interpreter error: unknown statement type");
}

int Interpreter::evaluate(const Expr* expr) {
    if (auto num = dynamic_cast<const NumberExpr*>(expr)) {
        return num->value;
    }

    if (auto var = dynamic_cast<const VariableExpr*>(expr)) {
        auto it = variables.find(var->name);
        if (it == variables.end()) {
            throw std::runtime_error("Interpreter error: undefined variable '" + var->name + "'");
        }
        return it->second;
    }

    if (auto bin = dynamic_cast<const BinaryExpr*>(expr)) {
        int left = evaluate(bin->left.get());
        int right = evaluate(bin->right.get());

        if (bin->op == "+") return left + right;

        throw std::runtime_error("Interpreter error: unknown operator '" + bin->op + "'");
    }

    throw std::runtime_error("Interpreter error: unknown expression type");
}
