// ast.h
#pragma once
#include <string>
#include <vector>
#include <memory>

// Base class for all expressions
struct Expr {
    virtual ~Expr() = default;
};

// A number literal, e.g. 5
struct NumberExpr : Expr {
    int value;
    explicit NumberExpr(int value) : value(value) {}
};

// A variable reference, e.g. x
struct VariableExpr : Expr {
    std::string name;
    explicit VariableExpr(std::string name) : name(std::move(name)) {}
};

// A binary operation, e.g. x + 2
struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    std::string op; // for now just "+"
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> left, std::string op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
};

// Base class for all statements
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

// log(<expr>);
struct LogStmt : Stmt {
    std::unique_ptr<Expr> value;

    explicit LogStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {}
};
