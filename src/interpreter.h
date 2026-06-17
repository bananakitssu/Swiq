#pragma once
#include "ast.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <variant>

// Forward-declared so Value can hold a pointer to it before it's fully defined.
struct ArrayObject;

// Value can be a number, string, bool, null, or a reference-counted array.
// The array alternative uses shared_ptr so arrays have reference semantics
// (mutating an array affects every variable referencing it, matching how
// most languages treat array/list assignment).
struct Value {
    std::variant<long long, std::string, bool, std::shared_ptr<ArrayObject>, std::monostate> data;

    Value() : data(std::monostate{}) {}              // default value = null
    Value(long long v) : data(v) {}
    Value(int v) : data((long long)v) {}
    Value(std::string v) : data(std::move(v)) {}
    Value(bool v) : data(v) {}
    Value(std::shared_ptr<ArrayObject> v) : data(std::move(v)) {}
};

// The actual array storage. `fixed` distinguishes a growable array created
// from a [...] literal (fixed = false, push() works) from a fixed-size
// array created with AllocatedArray(n) (fixed = true, push() is rejected,
// and indexing can never go past the size it was allocated with).
struct ArrayObject {
    std::vector<Value> elements;
    bool fixed = false;
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
