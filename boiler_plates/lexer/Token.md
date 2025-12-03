# Token Component

## Purpose
The Token component defines the structure of lexical tokens and their types. Tokens are the smallest meaningful units in the source code (keywords, identifiers, operators, literals, etc.).

## Files
- `include/lexer/Token.h` - Token structure and type definitions
- `src/lexer/Token.cpp` - Token type to string conversion

## Boilerplate Code

### Token.h
```cpp
#ifndef PILLA_TOKEN_H
#define PILLA_TOKEN_H

#include <string>
#include <map>

// Enum defining all token types
enum class Tokentype {
    // Delimiters
    LPAR,           // (
    RPAR,           // )
    LBRACE,         // {
    RBRACE,         // }
    SEMICOLON,      // ;
    COMMA,          // ,
    
    // Operators
    PLUS,           // +
    ASSIGN,         // =
    LESS_THAN,      // <
    GRE_THAN,       // >
    POUND,          // #
    
    // Literals
    NUMBER,         // Integer literal
    FLOAT_LITERAL,  // Float literal
    STRING_LITERAL, // String literal
    CHAR_LITERAL,   // Character literal
    IDENTIFIER,     // Variable/function names
    
    // Keywords
    KW_INT,
    KW_FLOAT,
    KW_DOUBLE,
    KW_CHAR,
    KW_STRING,
    KW_VOID,
    KW_RETURN,
    
    // Special
    UNKNOWN,
    E_O_F           // End of file
};

// Token structure
struct Token {
    Tokentype type;      // Type of token
    std::string lexeme;  // Actual text from source
    int line;            // Line number in source
    int column;          // Column number in source
};

// Helper function to convert token type to string
std::string tokenTypeToString(Tokentype type);

#endif // PILLA_TOKEN_H
```

### Token.cpp
```cpp
#include "lexer/Token.h"

std::string tokenTypeToString(Tokentype type) {
    static std::map<Tokentype, std::string> tokenTypeStrings = {
        {Tokentype::LPAR, "LPAR"},
        {Tokentype::RPAR, "RPAR"},
        {Tokentype::LBRACE, "LBRACE"},
        {Tokentype::RBRACE, "RBRACE"},
        {Tokentype::SEMICOLON, "SEMICOLON"},
        {Tokentype::COMMA, "COMMA"},
        {Tokentype::PLUS, "PLUS"},
        {Tokentype::ASSIGN, "ASSIGN"},
        {Tokentype::LESS_THAN, "LESS_THAN"},
        {Tokentype::GRE_THAN, "GRE_THAN"},
        {Tokentype::POUND, "POUND"},
        {Tokentype::NUMBER, "NUMBER"},
        {Tokentype::FLOAT_LITERAL, "FLOAT_LITERAL"},
        {Tokentype::STRING_LITERAL, "STRING_LITERAL"},
        {Tokentype::CHAR_LITERAL, "CHAR_LITERAL"},
        {Tokentype::IDENTIFIER, "IDENTIFIER"},
        {Tokentype::KW_INT, "KW_INT"},
        {Tokentype::KW_FLOAT, "KW_FLOAT"},
        {Tokentype::KW_DOUBLE, "KW_DOUBLE"},
        {Tokentype::KW_CHAR, "KW_CHAR"},
        {Tokentype::KW_STRING, "KW_STRING"},
        {Tokentype::KW_VOID, "KW_VOID"},
        {Tokentype::KW_RETURN, "KW_RETURN"},
        {Tokentype::UNKNOWN, "UNKNOWN"},
        {Tokentype::E_O_F, "EOF"}
    };
    
    try {
        return tokenTypeStrings.at(type);
    } catch (const std::out_of_range&) {
        return "INVALID_TOKEN";
    }
}
```

## How It Connects to Other Components

### Used By
- **Lexer**: Creates Token objects during lexical analysis
- **Parser**: Consumes Token objects to build the AST

### Data Flow
```
Source Code → Lexer → [Token, Token, ...] → Parser
```

## Key Concepts

1. **Token Type**: Categorizes the token (keyword, operator, identifier, etc.)
2. **Lexeme**: The actual text from the source code
3. **Location**: Line and column numbers for error reporting
4. **Enum Class**: Type-safe token type enumeration
