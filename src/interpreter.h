#pragma once
#include "ast.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <variant>

using Value = std::variant<long long, std::string, bool>;

class Interpreter {
public:
    void run(const std::vector<std::unique_ptr<Stmt>>& statements);

private:
    std::unordered_map<std::string, Value> variables;

    void execute(const Stmt* stmt);
    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements);
    Value evaluate(const Expr* expr);
    bool isTruthy(const Value& value);
    void printValue(const Value& value);
    std::string getMemberPath(const Expr* expr);
};
