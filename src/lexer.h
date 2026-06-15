// lexer.h
#pragma once
#include <string>
#include <vector>

enum class TokenType {
    SET,        // "set"
    VAR,        // "var"
    IDENTIFIER, // x, log, etc.
    NUMBER,     // 5, 2
    EQUALS,     // =
    PLUS,       // +
    LPAREN,     // (
    RPAREN,     // )
    SEMICOLON,  // ;
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value; // the actual text, e.g. "x" or "5"
};

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t pos = 0;

    char current() const;
    void advance();
    bool isAtEnd() const;

    Token readNumber();
    Token readIdentifierOrKeyword();
    void skipWhitespace();
};
