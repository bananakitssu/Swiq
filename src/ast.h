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

struct BoolExpr : Expr {
    bool value;
    explicit BoolExpr(bool value) : value(value) {}
};

struct VariableExpr : Expr {
    std::string name;
    int line;
    VariableExpr(std::string name, int line) : name(std::move(name)), line(line) {}
};

struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;
    int line;

    BinaryExpr(std::unique_ptr<Expr> left, std::string op, std::unique_ptr<Expr> right, int line)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)), line(line) {}
};

// object.property  (e.g. Swiq.__ENV__.__VERSION__.__BUILD_NUMBER__)
struct MemberExpr : Expr {
    std::unique_ptr<Expr> object;
    std::string property;
    int line;

    MemberExpr(std::unique_ptr<Expr> object, std::string property, int line)
        : object(std::move(object)), property(std::move(property)), line(line) {}
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

// set x = <expr>;
struct AssignStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> value;
    int line;
    AssignStmt(std::string name, std::unique_ptr<Expr> value, int line)
        : name(std::move(name)), value(std::move(value)), line(line) {}
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

// while (<cond>) { ... }
struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;

    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> body)
        : condition(std::move(condition)), body(std::move(body)) {}
};
