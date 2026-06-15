#pragma once
#include <string>
#include <vector>

enum class TokenType {
    SET, VAR, IF, ELSE,
    IDENTIFIER, NUMBER, STRING,
    EQUALS, EQUALS_EQUALS, NOT_EQUALS,
    LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
    PLUS, MINUS, STAR, SLASH,
    LPAREN, RPAREN, LBRACE, RBRACE,
    SEMICOLON,
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t pos = 0;

    char current() const;
    char peek() const; // look one char ahead
    void advance();
    bool isAtEnd() const;

    Token readNumber();
    Token readIdentifierOrKeyword();
    Token readString();
    void skipWhitespace();
};
