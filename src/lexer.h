#pragma once
#include <string>
#include <vector>

enum class TokenType {
    SET, VAR, LOCAL, GLOBAL, IF, ELSE, WHILE, TRUE, FALSE, FUNC, OVERRIDE, RETURN, FOR, NULLVAL, TYPE, INTERFACE, RESET, DELETE, ARCHIVE, RESTORE, AS, CATCH, TRY, SWITCHER, IS, DESTROY, AND,
    IDENTIFIER, NUMBER, FLOATNUM, HEXNUMBER, STRING,
    EQUALS, EQUALS_EQUALS, NOT_EQUALS,
    LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
    PLUS, MINUS, STAR, SLASH,
    DOT, COMMA, AT, COLON,
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
    Token readHexNumber();
    Token readIdentifierOrKeyword();
    Token readString();
    void skipWhitespace();
    void skipComment();
};
