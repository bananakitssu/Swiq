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
    if (current() == '\n') line++;
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

void Lexer::skipComment() {
    // '#' or '//' line comments
    if (current() == '#' || (current() == '/' && peek() == '/')) {
        while (!isAtEnd() && current() != '\n') advance();
        return;
    }
    // '/* ... */' block comments
    if (current() == '/' && peek() == '*') {
        advance(); advance(); // consume /*
        while (!isAtEnd() && !(current() == '*' && peek() == '/')) advance();
        if (!isAtEnd()) { advance(); advance(); } // consume */
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
    if (value == "while") return Token{TokenType::WHILE, value};
    if (value == "true") return Token{TokenType::TRUE, value};
    if (value == "false") return Token{TokenType::FALSE, value};

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

    while (true) {
        skipWhitespace();

        // Skip any sequence of comments/whitespace
        while (!isAtEnd() && (current() == '#' ||
               (current() == '/' && (peek() == '/' || peek() == '*')))) {
            skipComment();
            skipWhitespace();
        }

        if (isAtEnd()) break;

        int tokLine = line;
        char c = current();
        Token tok;

        if (std::isdigit(c)) {
            tok = readNumber();
        } else if (std::isalpha(c) || c == '_') {
            tok = readIdentifierOrKeyword();
        } else if (c == '"') {
            tok = readString();
        } else {
            switch (c) {
                case '=':
                    advance();
                    if (current() == '=') { advance(); tok = {TokenType::EQUALS_EQUALS, "=="}; }
                    else tok = {TokenType::EQUALS, "="};
                    break;
                case '!':
                    advance();
                    if (current() == '=') { advance(); tok = {TokenType::NOT_EQUALS, "!="}; }
                    else tok = {TokenType::END_OF_FILE, ""}; // unsupported '!' alone, ignore
                    break;
                case '<':
                    advance();
                    if (current() == '=') { advance(); tok = {TokenType::LESS_EQUAL, "<="}; }
                    else tok = {TokenType::LESS, "<"};
                    break;
                case '>':
                    advance();
                    if (current() == '=') { advance(); tok = {TokenType::GREATER_EQUAL, ">="}; }
                    else tok = {TokenType::GREATER, ">"};
                    break;
                case '+': tok = {TokenType::PLUS, "+"}; advance(); break;
                case '-': tok = {TokenType::MINUS, "-"}; advance(); break;
                case '*': tok = {TokenType::STAR, "*"}; advance(); break;
                case '/': tok = {TokenType::SLASH, "/"}; advance(); break;
                case '.': tok = {TokenType::DOT, "."}; advance(); break;
                case '(': tok = {TokenType::LPAREN, "("}; advance(); break;
                case ')': tok = {TokenType::RPAREN, ")"}; advance(); break;
                case '{': tok = {TokenType::LBRACE, "{"}; advance(); break;
                case '}': tok = {TokenType::RBRACE, "}"}; advance(); break;
                case ';': tok = {TokenType::SEMICOLON, ";"}; advance(); break;
                default:
                    advance();
                    continue; // skip unknown char entirely, no token
            }
        }

        tok.line = tokLine;
        tokens.push_back(tok);
    }

    tokens.push_back({TokenType::END_OF_FILE, "", line});
    return tokens;
}
