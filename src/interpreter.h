#pragma once
#include "ast.h"
#include "value.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <memory>

class Interpreter {
public:
    void run(const std::vector<std::unique_ptr<Stmt>>& statements);

private:
    std::unordered_map<std::string, Value> variables;
    std::unordered_set<std::string> local_variables;
    std::unordered_map<std::string, Value> archived_variables;
    std::unordered_map<std::string, Value> default_variables;
    std::unordered_map<std::string, const FuncDeclStmt*> functions;
    std::unordered_map<std::string, bool> protected_variables;
    
    void executeTryStmt(const TryStmt* stmt);

    // Registered type/interface declarations, keyed by name.
    // Stored as plain copies since TypeDeclStmt owns its fields and the
    // AST outlives the interpreter run.
    std::unordered_map<std::string, const TypeDeclStmt*> types;

    // Thrown internally to unwind out of a function body on `return`.
    struct ReturnSignal {
        Value value;
    };

    void registerFunctions(const std::vector<std::unique_ptr<Stmt>>& statements);
    void registerTypes(const std::vector<std::unique_ptr<Stmt>>& statements);
    void execute(const Stmt* stmt);
    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements);
    Value evaluate(const Expr* expr);
    Value callFunction(const FuncDeclStmt* func, std::vector<Value> args, int callLine);
    Value callBuiltin(const std::string& name, std::vector<Value>& args, int line);
    void printValue(const Value& value);
    std::string getMemberPath(const Expr* expr);

    // Type-system helpers
    Value constructTypedObject(const TypedObjectExpr* expr);
    bool fieldMatchesType(const Value& val, const TypeField& field) const;
    std::string typeNameOf(const Value& val) const;
};
