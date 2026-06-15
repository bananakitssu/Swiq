// lexer.cpp
#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& source) : source(source) {}

char Lexer::current() const {
    if (pos >= source.size()) return '\0';
    return source[pos];
}

void Lexer::advance() {
    pos++;
}

bool Lexer::isAtEnd() const {
    return pos >= source.size();
}

void Lexer::skipWhitespace() {
    while (!isAtEnd() && std::isspace(current())) {
        advance();
    }
}

Token Lexer::readNumber() {
    std::string value;
    while (!isAtEnd() && std::isdigit(current())) {
        value += current();
        advance();
    }
    return Token{TokenType::NUMBER, value};
}

Token Lexer::readIdentifierOrKeyword() {
    std::string value;
    while (!isAtEnd() && (std::isalnum(current()) || current() == '_')) {
        value += current();
        advance();
    }

    if (value == "set") return Token{TokenType::SET, value};
    if (value == "var") return Token{TokenType::VAR, value};

    return Token{TokenType::IDENTIFIER, value};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;

        char c = current();

        if (std::isdigit(c)) {
            tokens.push_back(readNumber());
            continue;
        }

        if (std::isalpha(c) || c == '_') {
            tokens.push_back(readIdentifierOrKeyword());
            continue;
        }

        switch (c) {
            case '=': tokens.push_back({TokenType::EQUALS, "="}); advance(); break;
            case '+': tokens.push_back({TokenType::PLUS, "+"}); advance(); break;
            case '(': tokens.push_back({TokenType::LPAREN, "("}); advance(); break;
            case ')': tokens.push_back({TokenType::RPAREN, ")"}); advance(); break;
            case ';': tokens.push_back({TokenType::SEMICOLON, ";"}); advance(); break;
            default:
                advance(); // skip unknown chars for now
                break;
        }
    }

    tokens.push_back({TokenType::END_OF_FILE, ""});
    return tokens;
}
