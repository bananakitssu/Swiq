#include "parser.h"
#include <stdexcept>
#include <fstream>
#include <sstream>

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

Token Parser::current() const {
    return tokens[pos];
}

Token Parser::peekAt(int offset) const {
    size_t idx = pos + offset;
    if (idx >= tokens.size()) return tokens.back();
    return tokens[idx];
}

Token Parser::peekNext() const {
    return peekAt(1);
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
        if (check(TokenType::AT)) {
            parseImportAndAppend(statements);
        } else {
            statements.push_back(parseStatement());
        }
    }
    return statements;
}

// @import "path/to/file.swiq";
// Reads, lexes, and parses the target file right now, then splices its
// statements directly into `target` — as if the file's contents had been
// pasted in at this exact spot. Only valid at the top level of a file
// (not inside a function/if/while body). Supports nested imports.
void Parser::parseImportAndAppend(std::vector<std::unique_ptr<Stmt>>& target) {
    expect(TokenType::AT, "expected '@'");
    Token kw = expect(TokenType::IDENTIFIER, "expected 'import' after '@'");
    if (kw.value != "import") {
        throw std::runtime_error("Parse error at line " + std::to_string(kw.line) +
                                  ": expected 'import' after '@', got '" + kw.value + "'");
    }
    Token pathTok = expect(TokenType::STRING, "expected a file path string after '@import'");
    expect(TokenType::SEMICOLON, "expected ';'");

    std::ifstream file(pathTok.value);
    if (!file) {
        throw std::runtime_error("Parse error at line " + std::to_string(pathTok.line) +
                                  ": could not open imported file '" + pathTok.value + "'");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();

    Lexer importLexer(buffer.str());
    std::vector<Token> importTokens = importLexer.tokenize();
    Parser importParser(importTokens);
    std::vector<std::unique_ptr<Stmt>> importedStatements = importParser.parse(); // handles nested @imports too

    for (auto& stmt : importedStatements) {
        target.push_back(std::move(stmt));
    }
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (check(TokenType::SET)) {
        if (peekAt(1).type == TokenType::VAR) {
            return parseVarDecl();
        }
        if (peekAt(1).type == TokenType::IDENTIFIER && peekAt(2).type == TokenType::LBRACKET) {
            return parseIndexAssign();
        }
        return parseAssign();
    }

    if (check(TokenType::IF)) {
        return parseIfStmt();
    }

    if (check(TokenType::WHILE)) {
        return parseWhileStmt();
    }

    if (check(TokenType::FOR)) {
        return parseForStmt();
    }

    if (check(TokenType::FUNC)) {
        return parseFuncDecl();
    }

    if (check(TokenType::RETURN)) {
        return parseReturnStmt();
    }

    if (check(TokenType::IDENTIFIER) && current().value == "log") {
        return parseLogStmt();
    }

    // Generic expression used as a statement, e.g. push(arr, 5); or x.ConvertToNumber();
    // (the value, if any, is just discarded)
    if (check(TokenType::IDENTIFIER)) {
        auto expr = parseExpr();
        expect(TokenType::SEMICOLON, "expected ';'");
        return std::make_unique<ExprStmt>(std::move(expr));
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

// Used only inside `for (...; ...; set i = i + 1)` — same as parseAssign but no trailing ';'
std::unique_ptr<Stmt> Parser::parseAssignNoSemicolon() {
    Token setTok = expect(TokenType::SET, "expected 'set'");
    Token name = expect(TokenType::IDENTIFIER, "expected variable name");
    expect(TokenType::EQUALS, "expected '='");
    auto value = parseExpr();
    return std::make_unique<AssignStmt>(name.value, std::move(value), setTok.line);
}

// set arr[index] = <expr>;
std::unique_ptr<Stmt> Parser::parseIndexAssign() {
    Token setTok = expect(TokenType::SET, "expected 'set'");
    Token name = expect(TokenType::IDENTIFIER, "expected array name");
    auto arrayExpr = std::make_unique<VariableExpr>(name.value, name.line);
    expect(TokenType::LBRACKET, "expected '['");
    auto indexExpr = parseExpr();
    expect(TokenType::RBRACKET, "expected ']'");
    expect(TokenType::EQUALS, "expected '='");
    auto value = parseExpr();
    expect(TokenType::SEMICOLON, "expected ';'");
    return std::make_unique<IndexAssignStmt>(std::move(arrayExpr), std::move(indexExpr), std::move(value), setTok.line);
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

// for (set var i = 0; i < 5; set i = i + 1) { ... }
std::unique_ptr<Stmt> Parser::parseForStmt() {
    expect(TokenType::FOR, "expected 'for'");
    expect(TokenType::LPAREN, "expected '('");

    std::unique_ptr<Stmt> init;
    if (peekAt(1).type == TokenType::VAR) {
        init = parseVarDecl(); // consumes its own ';'
    } else {
        init = parseAssign();  // consumes its own ';'
    }

    auto condition = parseExpr();
    expect(TokenType::SEMICOLON, "expected ';' after for-loop condition");

    auto post = parseAssignNoSemicolon();

    expect(TokenType::RPAREN, "expected ')'");
    auto body = parseBlock();

    return std::make_unique<ForStmt>(std::move(init), std::move(condition), std::move(post), std::move(body));
}

// func name(params)[captures] { body }
std::unique_ptr<Stmt> Parser::parseFuncDecl() {
    expect(TokenType::FUNC, "expected 'func'");
    Token name = expect(TokenType::IDENTIFIER, "expected function name");
    expect(TokenType::LPAREN, "expected '('");

    std::vector<std::string> params;
    if (!check(TokenType::RPAREN)) {
        params.push_back(expect(TokenType::IDENTIFIER, "expected parameter name").value);
        while (check(TokenType::COMMA)) {
            advance();
            params.push_back(expect(TokenType::IDENTIFIER, "expected parameter name").value);
        }
    }
    expect(TokenType::RPAREN, "expected ')'");

    std::vector<std::string> captures;
    if (check(TokenType::LBRACKET)) {
        advance();
        if (!check(TokenType::RBRACKET)) {
            captures.push_back(expect(TokenType::IDENTIFIER, "expected captured variable name").value);
            while (check(TokenType::COMMA)) {
                advance();
                captures.push_back(expect(TokenType::IDENTIFIER, "expected captured variable name").value);
            }
        }
        expect(TokenType::RBRACKET, "expected ']'");
    }

    auto body = parseBlock();
    return std::make_unique<FuncDeclStmt>(name.value, std::move(params), std::move(captures), std::move(body));
}

// return [<expr>];
std::unique_ptr<Stmt> Parser::parseReturnStmt() {
    Token retTok = expect(TokenType::RETURN, "expected 'return'");
    std::unique_ptr<Expr> value;
    if (!check(TokenType::SEMICOLON)) {
        value = parseExpr();
    }
    expect(TokenType::SEMICOLON, "expected ';'");
    return std::make_unique<ReturnStmt>(std::move(value), retTok.line);
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

// Handles member access (a.b), method calls (a.b(args)), and indexing
// (a[b]) — chainable: a.b[0].c()
std::unique_ptr<Expr> Parser::parsePostfix() {
    auto expr = parsePrimary();
    while (true) {
        if (check(TokenType::DOT)) {
            Token dotTok = advance();
            Token prop = expect(TokenType::IDENTIFIER, "expected property or method name after '.'");

            if (check(TokenType::LPAREN)) {
                advance(); // consume '('
                std::vector<std::unique_ptr<Expr>> args;
                if (!check(TokenType::RPAREN)) {
                    args.push_back(parseExpr());
                    while (check(TokenType::COMMA)) {
                        advance();
                        args.push_back(parseExpr());
                    }
                }
                expect(TokenType::RPAREN, "expected ')'");
                expr = std::make_unique<MethodCallExpr>(std::move(expr), prop.value, std::move(args), dotTok.line);
            } else {
                expr = std::make_unique<MemberExpr>(std::move(expr), prop.value, dotTok.line);
            }
        } else if (check(TokenType::LBRACKET)) {
            Token brTok = advance();
            auto indexExpr = parseExpr();
            expect(TokenType::RBRACKET, "expected ']'");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(indexExpr), brTok.line);
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (check(TokenType::NUMBER)) {
        Token tok = advance();
        return std::make_unique<NumberExpr>(std::stoll(tok.value));
    }

    if (check(TokenType::HEXNUMBER)) {
        Token tok = advance();
        return std::make_unique<HexExpr>(tok.value);
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

    if (check(TokenType::NULLVAL)) {
        advance();
        return std::make_unique<NullExpr>();
    }

    if (check(TokenType::LBRACKET)) {
        advance(); // consume '['
        std::vector<std::unique_ptr<Expr>> elements;
        if (!check(TokenType::RBRACKET)) {
            elements.push_back(parseExpr());
            while (check(TokenType::COMMA)) {
                advance();
                elements.push_back(parseExpr());
            }
        }
        expect(TokenType::RBRACKET, "expected ']'");
        return std::make_unique<ArrayExpr>(std::move(elements));
    }

    if (check(TokenType::IDENTIFIER)) {
        Token tok = advance();
        if (check(TokenType::LPAREN)) {
            advance(); // consume '('
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::RPAREN)) {
                args.push_back(parseExpr());
                while (check(TokenType::COMMA)) {
                    advance();
                    args.push_back(parseExpr());
                }
            }
            expect(TokenType::RPAREN, "expected ')'");
            return std::make_unique<CallExpr>(tok.value, std::move(args), tok.line);
        }
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
