#include "parser.h"
#include <stdexcept>

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

Token Parser::current() const {
    return tokens[pos];
}

Token Parser::peekNext() const {
    if (pos + 1 < tokens.size()) return tokens[pos + 1];
    return tokens.back();
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
        throw std::runtime_error("Parse error at line " + std::to_string(current().line) + ": " + errorMsg);
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
        if (peekNext().type == TokenType::VAR) {
            return parseVarDecl();
        }
        return parseAssign();
    }

    if (check(TokenType::IF)) {
        return parseIfStmt();
    }

    if (check(TokenType::WHILE)) {
        return parseWhileStmt();
    }

    if (check(TokenType::IDENTIFIER) && current().value == "log") {
        return parseLogStmt();
    }

    throw std::runtime_error("Parse error at line " + std::to_string(current().line) +
                              ": unexpected token '" + current().value + "'");
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

// set x = <expr>;
std::unique_ptr<Stmt> Parser::parseAssign() {
    Token setTok = expect(TokenType::SET, "expected 'set'");
    Token name = expect(TokenType::IDENTIFIER, "expected variable name");
    expect(TokenType::EQUALS, "expected '='");
    auto value = parseExpr();
    expect(TokenType::SEMICOLON, "expected ';'");
    return std::make_unique<AssignStmt>(name.value, std::move(value), setTok.line);
}

// log(<expr>);
std::unique_ptr<Stmt> Parser::parseLogStmt() {
    expect(TokenType::IDENTIFIER, "expected 'log'");
    expect(TokenType::LPAREN, "expected '('");
    auto value = parseExpr();
    expect(TokenType::RPAREN, "expected ')'");
    expect(TokenType::SEMICOLON, "expected ';'");
    return std::make_unique<LogStmt>(std::move(value));
}

// if (<cond>) { ... } [else { ... }]
std::unique_ptr<Stmt> Parser::parseIfStmt() {
    expect(TokenType::IF, "expected 'if'");
    expect(TokenType::LPAREN, "expected '('");
    auto condition = parseExpr();
    expect(TokenType::RPAREN, "expected ')'");
    auto thenBranch = parseBlock();

    std::vector<std::unique_ptr<Stmt>> elseBranch;
    if (check(TokenType::ELSE)) {
        advance();
        elseBranch = parseBlock();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

// while (<cond>) { ... }
std::unique_ptr<Stmt> Parser::parseWhileStmt() {
    expect(TokenType::WHILE, "expected 'while'");
    expect(TokenType::LPAREN, "expected '('");
    auto condition = parseExpr();
    expect(TokenType::RPAREN, "expected ')'");
    auto body = parseBlock();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::vector<std::unique_ptr<Stmt>> Parser::parseBlock() {
    expect(TokenType::LBRACE, "expected '{'");
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenType::RBRACE)) {
        statements.push_back(parseStatement());
    }
    expect(TokenType::RBRACE, "expected '}'");
    return statements;
}

std::unique_ptr<Expr> Parser::parseExpr() {
    return parseEquality();
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto left = parseComparison();
    while (check(TokenType::EQUALS_EQUALS) || check(TokenType::NOT_EQUALS)) {
        Token opTok = advance();
        auto right = parseComparison();
        left = std::make_unique<BinaryExpr>(std::move(left), opTok.value, std::move(right), opTok.line);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto left = parseAdditive();
    while (check(TokenType::LESS) || check(TokenType::GREATER) ||
           check(TokenType::LESS_EQUAL) || check(TokenType::GREATER_EQUAL)) {
        Token opTok = advance();
        auto right = parseAdditive();
        left = std::make_unique<BinaryExpr>(std::move(left), opTok.value, std::move(right), opTok.line);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseAdditive() {
    auto left = parseMultiplicative();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        Token opTok = advance();
        auto right = parseMultiplicative();
        left = std::make_unique<BinaryExpr>(std::move(left), opTok.value, std::move(right), opTok.line);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseMultiplicative() {
    auto left = parsePostfix();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        Token opTok = advance();
        auto right = parsePostfix();
        left = std::make_unique<BinaryExpr>(std::move(left), opTok.value, std::move(right), opTok.line);
    }
    return left;
}

// Handles member access: expr.identifier.identifier...
std::unique_ptr<Expr> Parser::parsePostfix() {
    auto expr = parsePrimary();
    while (check(TokenType::DOT)) {
        Token dotTok = advance();
        Token prop = expect(TokenType::IDENTIFIER, "expected property name after '.'");
        expr = std::make_unique<MemberExpr>(std::move(expr), prop.value, dotTok.line);
    }
    return expr;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (check(TokenType::NUMBER)) {
        Token tok = advance();
        return std::make_unique<NumberExpr>(std::stoll(tok.value));
    }

    if (check(TokenType::STRING)) {
        Token tok = advance();
        return std::make_unique<StringExpr>(tok.value);
    }

    if (check(TokenType::TRUE)) {
        advance();
        return std::make_unique<BoolExpr>(true);
    }

    if (check(TokenType::FALSE)) {
        advance();
        return std::make_unique<BoolExpr>(false);
    }

    if (check(TokenType::IDENTIFIER)) {
        Token tok = advance();
        return std::make_unique<VariableExpr>(tok.value, tok.line);
    }

    if (check(TokenType::LPAREN)) {
        advance();
        auto expr = parseExpr();
        expect(TokenType::RPAREN, "expected ')'");
        return expr;
    }

    throw std::runtime_error("Parse error at line " + std::to_string(current().line) +
                              ": expected expression, got '" + current().value + "'");
}
