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

// null / none / nil — all parse to this same node. They're interchangeable
// spellings of "no value", not three different concepts.
struct NullExpr : Expr {
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

// A hex literal like 0x1F. Stored as raw text, not a number — it stays
// "just text" until .ConvertToNumber() is called on it explicitly.
struct HexExpr : Expr {
    std::string raw;
    explicit HexExpr(std::string raw) : raw(std::move(raw)) {}
};

// object.method(args)  (e.g. someHex.ConvertToNumber())
// Distinct from MemberExpr: a MemberExpr is a static dotted path
// (Swiq.__ENV__...), while a MethodCallExpr actually invokes something.
struct MethodCallExpr : Expr {
    std::unique_ptr<Expr> object;
    std::string method;
    std::vector<std::unique_ptr<Expr>> args;
    int line;

    MethodCallExpr(std::unique_ptr<Expr> object, std::string method,
                   std::vector<std::unique_ptr<Expr>> args, int line)
        : object(std::move(object)), method(std::move(method)), args(std::move(args)), line(line) {}
};

// object.property  (e.g. Swiq.__ENV__.__VERSION__.__BUILD_NUMBER__)
struct MemberExpr : Expr {
    std::unique_ptr<Expr> object;
    std::string property;
    int line;

    MemberExpr(std::unique_ptr<Expr> object, std::string property, int line)
        : object(std::move(object)), property(std::move(property)), line(line) {}
};

// [expr, expr, expr, ...]
struct ArrayExpr : Expr {
    std::vector<std::unique_ptr<Expr>> elements;
    explicit ArrayExpr(std::vector<std::unique_ptr<Expr>> elements) : elements(std::move(elements)) {}
};

// array[index]
struct IndexExpr : Expr {
    std::unique_ptr<Expr> array;
    std::unique_ptr<Expr> index;
    int line;

    IndexExpr(std::unique_ptr<Expr> array, std::unique_ptr<Expr> index, int line)
        : array(std::move(array)), index(std::move(index)), line(line) {}
};

// name(arg, arg, ...)
struct CallExpr : Expr {
    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;
    int line;

    CallExpr(std::string callee, std::vector<std::unique_ptr<Expr>> args, int line)
        : callee(std::move(callee)), args(std::move(args)), line(line) {}
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

// set arr[index] = <expr>;
struct IndexAssignStmt : Stmt {
    std::unique_ptr<Expr> array;
    std::unique_ptr<Expr> index;
    std::unique_ptr<Expr> value;
    int line;

    IndexAssignStmt(std::unique_ptr<Expr> array, std::unique_ptr<Expr> index, std::unique_ptr<Expr> value, int line)
        : array(std::move(array)), index(std::move(index)), value(std::move(value)), line(line) {}
};

// log(<expr>);
struct LogStmt : Stmt {
    std::unique_ptr<Expr> value;
    explicit LogStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {}
};

// A bare expression used as a statement, e.g. calling a function for its side effects: push(arr, 5);
struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
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

// for (init; condition; post) { body }
struct ForStmt : Stmt {
    std::unique_ptr<Stmt> init;       // e.g. "set var i = 0;"
    std::unique_ptr<Expr> condition;  // e.g. "i < 5"
    std::unique_ptr<Stmt> post;       // e.g. "set i = i + 1"
    std::vector<std::unique_ptr<Stmt>> body;

    ForStmt(std::unique_ptr<Stmt> init, std::unique_ptr<Expr> condition,
            std::unique_ptr<Stmt> post, std::vector<std::unique_ptr<Stmt>> body)
        : init(std::move(init)), condition(std::move(condition)),
          post(std::move(post)), body(std::move(body)) {}
};

// func name(params)[captures] { body }
// `captures` lists outer variables this function is allowed to see.
// They're passed in by current value when the function is called, and
// written back to the outer scope when the function returns — so they
// behave like simple two-way closures, but only for names explicitly
// listed. Anything not listed is invisible to the function body.
struct FuncDeclStmt : Stmt {
    std::string name;
    std::vector<std::string> params;
    std::vector<std::string> captures;
    std::vector<std::unique_ptr<Stmt>> body;

    FuncDeclStmt(std::string name, std::vector<std::string> params,
                 std::vector<std::string> captures, std::vector<std::unique_ptr<Stmt>> body)
        : name(std::move(name)), params(std::move(params)),
          captures(std::move(captures)), body(std::move(body)) {}
};

// return [<expr>];
struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value; // nullptr if bare "return;"
    int line;

    ReturnStmt(std::unique_ptr<Expr> value, int line) : value(std::move(value)), line(line) {}
};
