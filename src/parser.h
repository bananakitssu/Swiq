// parser.h
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
    Token advance();
    bool check(TokenType type) const;
    Token expect(TokenType type, const std::string& errorMsg);

    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseVarDecl();
    std::unique_ptr<Stmt> parseLogStmt();

    std::unique_ptr<Expr> parseExpr();
    std::unique_ptr<Expr> parsePrimary();
};
