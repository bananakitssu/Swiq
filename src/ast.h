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

struct FloatExpr : Expr {
    double value;
    explicit FloatExpr(double value) : value(value) {}
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

// -expr  (currently only "-" is supported)
struct UnaryExpr : Expr {
    std::string op;
    std::unique_ptr<Expr> operand;
    int line;

    UnaryExpr(std::string op, std::unique_ptr<Expr> operand, int line)
        : op(std::move(op)), operand(std::move(operand)), line(line) {}
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

// { field: value, field: value }<TypeName>
// Always tagged with a declared type or interface name — there's no such
// thing as an untagged/anonymous object literal in Swiq.
struct TypedObjectExpr : Expr {
    std::string typeName;
    std::vector<std::pair<std::string, std::unique_ptr<Expr>>> fields;
    int line;

    TypedObjectExpr(std::string typeName, std::vector<std::pair<std::string, std::unique_ptr<Expr>>> fields, int line)
        : typeName(std::move(typeName)), fields(std::move(fields)), line(line) {}
};

struct Stmt {
    virtual ~Stmt() = default;
};

// set var x = <expr>;
struct VarDeclStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> value;
    bool isProtected;
    bool isLocal;
    int line;
    VarDeclStmt(std::string name, std::unique_ptr<Expr> value, bool isProtected, bool isLocal, int line)
        : name(std::move(name)), value(std::move(value)), isProtected(isProtected), isLocal(isLocal), line(line) {}
};

// set x = <expr>;
struct AssignStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> value;
    int line;
    int assign_type;
    AssignStmt(std::string name, std::unique_ptr<Expr> value, int line, int assign_type)
        : name(std::move(name)), value(std::move(value)), line(line), assign_type(assign_type) {}
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

// try { ... } or try { ... } catch (Error as e) { ... }
struct TryStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> tryBlock;
    std::string errorVarName;
    std::vector<std::unique_ptr<Stmt>> catchBlock;
    int line;

    TryStmt(std::vector<std::unique_ptr<Stmt>> tryBlock,
            std::string errorVarName,
            std::vector<std::unique_ptr<Stmt>> catchBlock,
            int line)
        : tryBlock(std::move(tryBlock)),
          errorVarName(std::move(errorVarName)),
          catchBlock(std::move(catchBlock)),
          line(line) {}
};

struct SwitcherCase {
    std::unique_ptr<Expr> matchValue;
    std::vector<std::unique_ptr<Stmt>> body;
};

struct SwitcherStmt : Stmt {
    std::unique_ptr<Expr> controlExpr;
    std::vector<SwitcherCase> cases;
    int line;

    SwitcherStmt(std::unique_ptr<Expr> controlExpr, std::vector<SwitcherCase> cases, int line)
        : controlExpr(std::move(controlExpr)), cases(std::move(cases)), line(line) {}
};

struct DestroyStmt : Stmt {
    int line;
    explicit DestroyStmt(int line) : line(line) {}
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
    bool overriding;
    bool capturingAll;
    bool isProtected;
    int line;

    FuncDeclStmt(std::string name, std::vector<std::string> params,
                 std::vector<std::string> captures, std::vector<std::unique_ptr<Stmt>> body, bool overriding, bool capturingAll, bool isProtected, int line)
        : name(std::move(name)), params(std::move(params)),
          captures(std::move(captures)), body(std::move(body)), overriding(std::move(overriding)), capturingAll(std::move(capturingAll)), isProtected(std::move(isProtected)), line(std::move(line)) {}
};

// return [<expr>];
struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value; // nullptr if bare "return;"
    int line;

    ReturnStmt(std::unique_ptr<Expr> value, int line) : value(std::move(value)), line(line) {}
};

// reset x
struct ResetStmt : Stmt {
    std::string name;
    int line;
    
    ResetStmt(std::string name, int line) : name(std::move(name)), line(line) {}
};

// delete x
struct DeleteStmt : Stmt {
    std::string name;
    int line;
    
    DeleteStmt(std::string name, int line) : name(std::move(name)), line(line) {}
};

// archive x
struct ArchiveStmt : Stmt {
    std::string name;
    int line;
    
    ArchiveStmt(std::string name, int line) : name(std::move(name)), line(line) {}
};

// restore x
struct RestoreStmt : Stmt {
    std::string name;
    int line;
    
    RestoreStmt(std::string name, int line) : name(std::move(name)), line(line) {}
};

// One field inside a `type` or `interface` declaration, e.g.
// "x: Number<integerOnly>" or "x: Number<integerOnly> = 5".
struct TypeField {
    std::string name;
    std::string baseType;       // "Number", "String", "Boolean", or "Array"
    std::string modifier;       // "" or "integerOnly" or "floatOnly" (Number only)
    std::unique_ptr<Expr> defaultValue; // nullptr for `type` fields, or for `interface` fields with no default
};

// set type T = { x: Number<integerOnly> };
// set interface R = { x: Number<integerOnly> = 5 };
//
// `type` fields never have defaults — a field left unset at construction
// becomes null. `interface` fields can have defaults, so every field
// always resolves to a real value, never null, once defaults are applied.
struct TypeDeclStmt : Stmt {
    std::string name;
    std::vector<TypeField> fields;
    bool isInterface;

    TypeDeclStmt(std::string name, std::vector<TypeField> fields, bool isInterface)
        : name(std::move(name)), fields(std::move(fields)), isInterface(isInterface) {}
};
