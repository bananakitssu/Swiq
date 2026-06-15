// parser.cpp
#include "parser.h"
#include <stdexcept>

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

Token Parser::current() const {
    return tokens[pos];
}

Token Parser::advance() {
    Token tok = tokens[pos];
    if (pos < tokens.size() - 1) pos++;
    return tok;
}

bool Parser::check(TokenType type) const {
    return current().type == type;
}

Token Parser::expect(TokenType type, const std::string& errorMsg) {
    if (!check(type)) {
        throw std::runtime_error("Parse error: " + errorMsg);
    }
    return advance();
}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;

    while (!check(TokenType::END_OF_FILE)) {
        statements.push_back(parseStatement());
    }

    return statements;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (check(TokenType::SET)) {
        return parseVarDecl();
    }

    if (check(TokenType::IDENTIFIER) && current().value == "log") {
        return parseLogStmt();
    }

    throw std::runtime_error("Parse error: unexpected token '" + current().value + "'");
}

// set var x = <expr>;
std::unique_ptr<Stmt> Parser::parseVarDecl() {
    expect(TokenType::SET, "expected 'set'");
    expect(TokenType::VAR, "expected 'var'");
    Token name = expect(TokenType::IDENTIFIER, "expected variable name");
    expect(TokenType::EQUALS, "expected '='");
    auto value = parseExpr();
    expect(TokenType::SEMICOLON, "expected ';'");

    return std::make_unique<VarDeclStmt>(name.value, std::move(value));
}

// log(<expr>);
std::unique_ptr<Stmt> Parser::parseLogStmt() {
    expect(TokenType::IDENTIFIER, "expected 'log'"); // consumes "log"
    expect(TokenType::LPAREN, "expected '('");
    auto value = parseExpr();
    expect(TokenType::RPAREN, "expected ')'");
    expect(TokenType::SEMICOLON, "expected ';'");

    return std::make_unique<LogStmt>(std::move(value));
}

// expr := primary (+ primary)*
std::unique_ptr<Expr> Parser::parseExpr() {
    auto left = parsePrimary();

    while (check(TokenType::PLUS)) {
        advance(); // consume '+'
        auto right = parsePrimary();
        left = std::make_unique<BinaryExpr>(std::move(left), "+", std::move(right));
    }

    return left;
}

// primary := NUMBER | IDENTIFIER
std::unique_ptr<Expr> Parser::parsePrimary() {
    if (check(TokenType::NUMBER)) {
        Token tok = advance();
        return std::make_unique<NumberExpr>(std::stoi(tok.value));
    }

    if (check(TokenType::IDENTIFIER)) {
        Token tok = advance();
        return std::make_unique<VariableExpr>(tok.value);
    }

    throw std::runtime_error("Parse error: expected expression, got '" + current().value + "'");
}
