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
    Token peekNext() const;
    Token advance();
    bool check(TokenType type) const;
    Token expect(TokenType type, const std::string& errorMsg);

    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseVarDecl();
    std::unique_ptr<Stmt> parseAssign();
    std::unique_ptr<Stmt> parseLogStmt();
    std::unique_ptr<Stmt> parseIfStmt();
    std::vector<std::unique_ptr<Stmt>> parseBlock();

    std::unique_ptr<Expr> parseExpr();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseAdditive();
    std::unique_ptr<Expr> parseMultiplicative();
    std::unique_ptr<Expr> parsePrimary();
};
