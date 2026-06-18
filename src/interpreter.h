#pragma once
#include "ast.h"
#include "value.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

class Interpreter {
public:
    void run(const std::vector<std::unique_ptr<Stmt>>& statements);

private:
    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, const FuncDeclStmt*> functions;

    // Thrown internally to unwind out of a function body on `return`.
    struct ReturnSignal {
        Value value;
    };

    void registerFunctions(const std::vector<std::unique_ptr<Stmt>>& statements);
    void execute(const Stmt* stmt);
    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements);
    Value evaluate(const Expr* expr);
    Value callFunction(const FuncDeclStmt* func, std::vector<Value> args, int callLine);
    Value callBuiltin(const std::string& name, std::vector<Value>& args, int line);
    void printValue(const Value& value);
    std::string getMemberPath(const Expr* expr);
};
