#include "lexer/Lexer.h"
#include <cctype> // this for some functions like isdigit, isalpha

// constructor
Lexer::Lexer(const std::string& source)
    : sourcecode(source), currentPos(0) {}

std::vector<Token> Lexer::scanTokens() {
    std::vector<Token> tokens;
    Token token = scanToken();
    while (token.type != Tokentype::E_O_F) {
        tokens.push_back(token);
        token = scanToken();
    }
    tokens.push_back(token);
    return tokens;
}

Token Lexer::scanToken() {
    skipWhitespace();

    if(isAtEnd()) {
        return makeToken(Tokentype::E_O_F, "SIGNING_OFF");
    }

    char c = advance();

    // Check for single-character tokens
    switch (c) {
        case '(': return makeToken(Tokentype::LPAR, "(");
        case ')': return makeToken(Tokentype::RPAR, ")");
        case '{': return makeToken(Tokentype::LBRACE, "{");
        case '}': return makeToken(Tokentype::RBRACE, "}");
        case ';': return makeToken(Tokentype::SEMICOLON, ";");
        case '+': return makeToken(Tokentype::PLUS, "+");
        case '#': return makeToken(Tokentype::POUND, "#");
        case '<': return makeToken(Tokentype::LESS_THAN, "<");
        case '>': return makeToken(Tokentype::GRE_THAN, ">");
    }

    // Check for numbers
    if (std::isdigit(c)) {
        return number();
    }

    // Check for identifiers and keywords
    if (std::isalpha(c) || c == '_') {
        return identifier();
    }

    // If we don't recognize the character
    return makeToken(Tokentype::UNKNOWN, std::string(1, c));

}

char Lexer::advance() {
    if(!isAtEnd()) {
        currentPos++;
    }
    return sourcecode[currentPos - 1];
}

bool Lexer::isAtEnd() {
    return currentPos >= sourcecode.length();
}

char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return sourcecode[currentPos];
}

Token Lexer::makeToken(Tokentype type, const std::string& lexeme) {
    return Token{type, lexeme};
}

void Lexer::skipWhitespace() {
    while(true){
        if (isAtEnd()) return;
        char c = peek();
        switch(c) {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                advance();
                break;
            default:
                return;
        }
    }
}

Token Lexer::number() {
    size_t start = currentPos - 1;
    while(std::isdigit(peek())) {
        advance();
    }

    std::string numLexeme = sourcecode.substr(start, currentPos - start);
    return makeToken(Tokentype::NUMBER, numLexeme);
}

Token Lexer::identifier() {
    size_t start = currentPos - 1;
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }
    std::string idLexeme = sourcecode.substr(start, currentPos - start);

    //check if keyword
    if (idLexeme == "int") return makeToken(Tokentype::KW_INT, idLexeme);
    if (idLexeme == "return") return makeToken(Tokentype::KW_RETURN, idLexeme);
    if (idLexeme == "include") return makeToken(Tokentype::KW_INCLUDE, idLexeme);
    
    // It's a regular identifier
    return makeToken(Tokentype::IDENTIFIER, idLexeme);
}