#pragma once
#include "ast.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <variant>

// Value can be a number, string, bool, or a reference-counted array of Values.
// The array alternative uses shared_ptr so arrays have reference semantics
// (mutating an array affects every variable referencing it, matching how
// most languages treat array/list assignment).
struct Value {
    std::variant<long long, std::string, bool, std::shared_ptr<std::vector<Value>>> data;

    Value() : data((long long)0) {}
    Value(long long v) : data(v) {}
    Value(int v) : data((long long)v) {}
    Value(std::string v) : data(std::move(v)) {}
    Value(bool v) : data(v) {}
    Value(std::shared_ptr<std::vector<Value>> v) : data(std::move(v)) {}
};

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
    bool isTruthy(const Value& value);
    void printValue(const Value& value);
    std::string getMemberPath(const Expr* expr);
    std::string valueToDisplayString(const Value& value);
};
