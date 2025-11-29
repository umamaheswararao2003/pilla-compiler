#include "lexer/Lexer.h"
#include <cctype> // this for some functions like isdigit, isalpha

// constructor
Lexer::Lexer(const std::string& source)
    : sourcecode(source), currentPos(0), line(1), column(1) {}

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
    
    tokenStartLine = line;
    tokenStartColumn = column;

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
        case '=': return makeToken(Tokentype::ASSIGN, "=");
        case ',': return makeToken(Tokentype::COMMA, ",");
        case '"': return string();
        case '\'': return character();
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
        column++;
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
    return Token{type, lexeme, tokenStartLine, tokenStartColumn};
}

void Lexer::skipWhitespace() {
    while(true){
        if (isAtEnd()) return;
        char c = peek();
        switch(c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line++;
                column = 0;
                advance();
                break;
            default:
                return;
        }
    }
}

Token Lexer::number() {
    size_t start = currentPos - 1;
    bool isFloat = false;

    while(std::isdigit(peek())) {
        advance();
    }

    // Look for a fractional part.
    if (peek() == '.' && std::isdigit(sourcecode[currentPos + 1])) {
        isFloat = true;
        // Consume the "."
        advance();

        while (std::isdigit(peek())) {
            advance();
        }
    }

    std::string numLexeme = sourcecode.substr(start, currentPos - start);
    return makeToken(isFloat ? Tokentype::FLOAT_LITERAL : Tokentype::NUMBER, numLexeme);
}

Token Lexer::string() {
    size_t start = currentPos - 1; 
    // include opening quote for now? usually we don't want quotes in value but for lexeme we might
    // The previous implementation of makeToken takes the lexeme. 
    // Let's keep the quotes in the lexeme for now, or strip them. 
    // Standard practice: token type STRING_LITERAL, lexeme includes quotes or value is stripped.
    // Let's include quotes in lexeme to be safe, parser can strip.
    
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            line++;
            column = 0;
        }
        advance();
    }

    if (isAtEnd()) {
        // Unterminated string
        return makeToken(Tokentype::UNKNOWN, "Unterminated string");
    }

    // The closing "
    advance();

    std::string value = sourcecode.substr(start, currentPos - start);
    return makeToken(Tokentype::STRING_LITERAL, value);
}

Token Lexer::character() {
    size_t start = currentPos - 1;
    
    if (peek() == '\'') {
        // Empty char literal? '' -> usually invalid or null char
        return makeToken(Tokentype::UNKNOWN, "Empty character literal");
    }
    
    // Handle escape sequences if necessary, for now simple char
    if (peek() == '\\') {
        advance(); // skip \ 
        // advance again for the escaped char
    }
    advance(); 

    if (peek() != '\'') {
        return makeToken(Tokentype::UNKNOWN, "Unterminated character literal");
    }

    advance(); // closing '

    std::string value = sourcecode.substr(start, currentPos - start);
    return makeToken(Tokentype::CHAR_LITERAL, value);
}

Token Lexer::identifier() {
    size_t start = currentPos - 1;
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }
    std::string idLexeme = sourcecode.substr(start, currentPos - start);

    //check if keyword
    if(idLexeme == "int") {
        return makeToken(Tokentype::KW_INT, idLexeme);
    } else if (idLexeme == "return") {
        return makeToken(Tokentype::KW_RETURN, idLexeme);
    } else if (idLexeme == "float") {
        return makeToken(Tokentype::KW_FLOAT, idLexeme);
    } else if (idLexeme == "char") {
        return makeToken(Tokentype::KW_CHAR, idLexeme);
    } else if (idLexeme == "string") {
        return makeToken(Tokentype::KW_STRING, idLexeme);
    } else if (idLexeme == "double") {
        return makeToken(Tokentype::KW_DOUBLE, idLexeme);
    } else {
        return makeToken(Tokentype::IDENTIFIER, idLexeme);
    }
}