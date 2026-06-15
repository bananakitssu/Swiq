// interpreter.h
#pragma once
#include "ast.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

class Interpreter {
public:
    void run(const std::vector<std::unique_ptr<Stmt>>& statements);

private:
    std::unordered_map<std::string, int> variables;

    void execute(const Stmt* stmt);
    int evaluate(const Expr* expr);
};
