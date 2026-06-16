#pragma once
#include <string>
#include <vector>

enum class TokenType {
    SET, VAR, IF, ELSE, WHILE, TRUE, FALSE, FUNC, RETURN, FOR,
    IDENTIFIER, NUMBER, STRING,
    EQUALS, EQUALS_EQUALS, NOT_EQUALS,
    LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
    PLUS, MINUS, STAR, SLASH,
    DOT, COMMA,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    SEMICOLON,
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
    int line = 1;
};

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t pos = 0;
    int line = 1;

    char current() const;
    char peek() const;
    void advance();
    bool isAtEnd() const;

    Token readNumber();
    Token readIdentifierOrKeyword();
    Token readString();
    void skipWhitespace();
    void skipComment();
};
