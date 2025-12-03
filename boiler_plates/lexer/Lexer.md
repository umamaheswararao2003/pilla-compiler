# Lexer Component

## Purpose
The Lexer (Lexical Analyzer) reads source code as a string and converts it into a sequence of tokens. It handles whitespace, identifies keywords, operators, literals, and identifiers.

## Files
- `include/lexer/Lexer.h` - Lexer class declaration
- `src/lexer/Lexer.cpp` - Lexer implementation

## Boilerplate Code

### Lexer.h
```cpp
#ifndef PILLA_LEXER_H
#define PILLA_LEXER_H

#include "lexer/Token.h"
#include <string>
#include <vector>

class Lexer {
public:
    // Constructor: takes source code as input
    Lexer(const std::string& source);
    
    // Main entry point: scans all tokens
    std::vector<Token> scanTokens();

private:
    std::string sourcecode;  // Source code to tokenize
    size_t currentPos;       // Current position in source
    int line;                // Current line number
    int column;              // Current column number
    int tokenStartLine;      // Line where current token starts
    int tokenStartColumn;    // Column where current token starts
    
    // Core scanning methods
    Token scanToken();       // Scan a single token
    
    // Character navigation
    char advance();          // Move to next character
    char peek();             // Look at current character without consuming
    bool isAtEnd();          // Check if at end of source
    
    // Token creation
    Token makeToken(Tokentype type, const std::string& lexeme);
    
    // Specialized scanners
    Token number();          // Scan number literals (int/float)
    Token string();          // Scan string literals
    Token character();       // Scan character literals
    Token identifier();      // Scan identifiers and keywords
    
    // Utility
    void skipWhitespace();   // Skip whitespace and newlines
};

#endif // PILLA_LEXER_H
```

### Lexer.cpp (Key Methods)
```cpp
#include "lexer/Lexer.h"
#include <cctype>

// Constructor
Lexer::Lexer(const std::string& source)
    : sourcecode(source), currentPos(0), line(1), column(1) {}

// Main scanning loop
std::vector<Token> Lexer::scanTokens() {
    std::vector<Token> tokens;
    Token token = scanToken();
    while (token.type != Tokentype::E_O_F) {
        tokens.push_back(token);
        token = scanToken();
    }
    tokens.push_back(token);  // Add EOF token
    return tokens;
}

// Scan a single token
Token Lexer::scanToken() {
    skipWhitespace();
    
    tokenStartLine = line;
    tokenStartColumn = column;

    if (isAtEnd()) {
        return makeToken(Tokentype::E_O_F, "SIGNING_OFF");
    }

    char c = advance();

    // Single-character tokens
    switch (c) {
        case '(': return makeToken(Tokentype::LPAR, "(");
        case ')': return makeToken(Tokentype::RPAR, ")");
        case '{': return makeToken(Tokentype::LBRACE, "{");
        case '}': return makeToken(Tokentype::RBRACE, "}");
        case ';': return makeToken(Tokentype::SEMICOLON, ";");
        case '+': return makeToken(Tokentype::PLUS, "+");
        case '=': return makeToken(Tokentype::ASSIGN, "=");
        case ',': return makeToken(Tokentype::COMMA, ",");
        case '"': return string();
        case '\'': return character();
    }

    // Numbers
    if (std::isdigit(c)) {
        return number();
    }

    // Identifiers and keywords
    if (std::isalpha(c) || c == '_') {
        return identifier();
    }

    return makeToken(Tokentype::UNKNOWN, std::string(1, c));
}

// Advance to next character
char Lexer::advance() {
    if (!isAtEnd()) {
        currentPos++;
        column++;
    }
    return sourcecode[currentPos - 1];
}

// Check if at end
bool Lexer::isAtEnd() {
    return currentPos >= sourcecode.length();
}

// Peek at current character
char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return sourcecode[currentPos];
}

// Create a token
Token Lexer::makeToken(Tokentype type, const std::string& lexeme) {
    return Token{type, lexeme, tokenStartLine, tokenStartColumn};
}

// Skip whitespace
void Lexer::skipWhitespace() {
    while (true) {
        if (isAtEnd()) return;
        char c = peek();
        switch (c) {
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

// Scan number (int or float)
Token Lexer::number() {
    size_t start = currentPos - 1;
    bool isFloat = false;

    while (std::isdigit(peek())) {
        advance();
    }

    // Check for decimal point
    if (peek() == '.' && std::isdigit(sourcecode[currentPos + 1])) {
        isFloat = true;
        advance();  // Consume '.'
        while (std::isdigit(peek())) {
            advance();
        }
    }

    std::string numLexeme = sourcecode.substr(start, currentPos - start);
    return makeToken(isFloat ? Tokentype::FLOAT_LITERAL : Tokentype::NUMBER, numLexeme);
}

// Scan identifier or keyword
Token Lexer::identifier() {
    size_t start = currentPos - 1;
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }
    std::string idLexeme = sourcecode.substr(start, currentPos - start);

    // Check if it's a keyword
    if (idLexeme == "int") return makeToken(Tokentype::KW_INT, idLexeme);
    if (idLexeme == "float") return makeToken(Tokentype::KW_FLOAT, idLexeme);
    if (idLexeme == "double") return makeToken(Tokentype::KW_DOUBLE, idLexeme);
    if (idLexeme == "char") return makeToken(Tokentype::KW_CHAR, idLexeme);
    if (idLexeme == "string") return makeToken(Tokentype::KW_STRING, idLexeme);
    if (idLexeme == "void") return makeToken(Tokentype::KW_VOID, idLexeme);
    if (idLexeme == "return") return makeToken(Tokentype::KW_RETURN, idLexeme);
    
    return makeToken(Tokentype::IDENTIFIER, idLexeme);
}
```

## How It Connects to Other Components

### Input
- **Source Code**: String containing the program to compile

### Output
- **Token Stream**: Vector of Token objects passed to Parser

### Data Flow
```
main.cpp → reads file → Lexer.scanTokens() → [Tokens] → Parser
```

## Key Concepts

1. **Scanning**: Process of reading characters and grouping them into tokens
2. **Lookahead**: Using `peek()` to check next character without consuming it
3. **Position Tracking**: Maintaining line and column for error reporting
4. **Keyword Recognition**: Checking identifiers against keyword list
5. **Literal Parsing**: Handling different literal types (int, float, string, char)
