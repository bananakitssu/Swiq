#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& source) : source(source) {}

char Lexer::current() const {
    if (pos >= source.size()) return '\0';
    return source[pos];
}

char Lexer::peek() const {
    if (pos + 1 >= source.size()) return '\0';
    return source[pos + 1];
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
    if (value == "if") return Token{TokenType::IF, value};
    if (value == "else") return Token{TokenType::ELSE, value};

    return Token{TokenType::IDENTIFIER, value};
}

Token Lexer::readString() {
    advance(); // consume opening quote
    std::string value;
    while (!isAtEnd() && current() != '"') {
        value += current();
        advance();
    }
    advance(); // consume closing quote
    return Token{TokenType::STRING, value};
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

        if (c == '"') {
            tokens.push_back(readString());
            continue;
        }

        switch (c) {
            case '=':
                advance();
                if (current() == '=') { advance(); tokens.push_back({TokenType::EQUALS_EQUALS, "=="}); }
                else tokens.push_back({TokenType::EQUALS, "="});
                break;
            case '!':
                advance();
                if (current() == '=') { advance(); tokens.push_back({TokenType::NOT_EQUALS, "!="}); }
                break;
            case '<':
                advance();
                if (current() == '=') { advance(); tokens.push_back({TokenType::LESS_EQUAL, "<="}); }
                else tokens.push_back({TokenType::LESS, "<"});
                break;
            case '>':
                advance();
                if (current() == '=') { advance(); tokens.push_back({TokenType::GREATER_EQUAL, ">="}); }
                else tokens.push_back({TokenType::GREATER, ">"});
                break;
            case '+': tokens.push_back({TokenType::PLUS, "+"}); advance(); break;
            case '-': tokens.push_back({TokenType::MINUS, "-"}); advance(); break;
            case '*': tokens.push_back({TokenType::STAR, "*"}); advance(); break;
            case '/': tokens.push_back({TokenType::SLASH, "/"}); advance(); break;
            case '(': tokens.push_back({TokenType::LPAREN, "("}); advance(); break;
            case ')': tokens.push_back({TokenType::RPAREN, ")"}); advance(); break;
            case '{': tokens.push_back({TokenType::LBRACE, "{"}); advance(); break;
            case '}': tokens.push_back({TokenType::RBRACE, "}"}); advance(); break;
            case ';': tokens.push_back({TokenType::SEMICOLON, ";"}); advance(); break;
            default:
                advance();
                break;
        }
    }

    tokens.push_back({TokenType::END_OF_FILE, ""});
    return tokens;
}
