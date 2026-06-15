#pragma once
#include <string>
#include <vector>
#include <memory>

struct Expr {
    virtual ~Expr() = default;
};

struct NumberExpr : Expr {
    long long value;
    explicit NumberExpr(long long value) : value(value) {}
};

struct StringExpr : Expr {
    std::string value;
    explicit StringExpr(std::string value) : value(std::move(value)) {}
};

struct VariableExpr : Expr {
    std::string name;
    explicit VariableExpr(std::string name) : name(std::move(name)) {}
};

struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> left, std::string op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
};

struct Stmt {
    virtual ~Stmt() = default;
};

// set var x = <expr>;
struct VarDeclStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> value;
    VarDeclStmt(std::string name, std::unique_ptr<Expr> value)
        : name(std::move(name)), value(std::move(value)) {}
};

// set x = <expr>;  (reassignment, no 'var')
struct AssignStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> value;
    AssignStmt(std::string name, std::unique_ptr<Expr> value)
        : name(std::move(name)), value(std::move(value)) {}
};

// log(<expr>);
struct LogStmt : Stmt {
    std::unique_ptr<Expr> value;
    explicit LogStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {}
};

// if (<cond>) { ... } [else { ... }]
struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> thenBranch;
    std::vector<std::unique_ptr<Stmt>> elseBranch;

    IfStmt(std::unique_ptr<Expr> condition,
           std::vector<std::unique_ptr<Stmt>> thenBranch,
           std::vector<std::unique_ptr<Stmt>> elseBranch)
        : condition(std::move(condition)),
          thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)) {}
};
