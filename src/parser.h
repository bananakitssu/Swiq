#pragma once
#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    std::vector<std::unique_ptr<Stmt>> parse();

private:
    std::vector<Token> tokens;
    size_t pos = 0;

    Token current() const;
    Token peekAt(int offset) const;
    Token peekNext() const;
    Token advance();
    bool check(TokenType type) const;
    Token expect(TokenType type, const std::string& errorMsg);

    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseVarDecl();
    std::unique_ptr<Stmt> parseAssign();
    std::unique_ptr<Stmt> parseIndexAssign();
    std::unique_ptr<Stmt> parseAssignNoSemicolon(); // for "for" loop post-clause
    std::unique_ptr<Stmt> parseLogStmt();
    std::unique_ptr<Stmt> parseIfStmt();
    std::unique_ptr<Stmt> parseWhileStmt();
    std::unique_ptr<Stmt> parseForStmt();
    std::unique_ptr<Stmt> parseFuncDecl();
    std::unique_ptr<Stmt> parseReturnStmt();
    std::unique_ptr<Stmt> parseDeleteStmt();
    std::unique_ptr<Stmt> parseArchiveStmt();
    std::unique_ptr<Stmt> parseRestoreStmt();
    std::unique_ptr<Stmt> parseTypeDecl(bool isInterface); // set type T = {...}; / set interface R = {...};
    std::unique_ptr<Stmt> parseResetStmt();
    TypeField parseTypeField(); // one "name: BaseType<modifier> = default" entry
    void parseImportAndAppend(std::vector<std::unique_ptr<Stmt>>& target); // @import "file";
    std::vector<std::unique_ptr<Stmt>> parseBlock();

    std::unique_ptr<Expr> parseExpr();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseAdditive();
    std::unique_ptr<Expr> parseMultiplicative();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePostfix();
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseTypedObject(); // { field: value, ... }<TypeName>
};
