#ifndef PILLA_TOKEN_H
#define PILLA_TOKEN_H

#include <string>

// types of tokens we want to recognise
enum class Tokentype {
    // single character
    LPAR, RPAR,LBRACE, RBRACE,LESS_THAN,
    GRE_THAN, SEMICOLON, PLUS, MINUS, MULTIPLY, DIVIDE, MODULO, POUND,ASSIGN, COMMA,

    // multi-character operators
    EQUAL_EQUAL, NOT_EQUAL, LESS_EQUAL, GREATER_EQUAL,

    // NUMBERS AND NAMES
    NUMBER, IDENTIFIER, FLOAT_LITERAL, CHAR_LITERAL, STRING_LITERAL,

    // KEYWORDS
    KW_INT, KW_RETURN, KW_FLOAT, KW_CHAR, KW_STRING, KW_DOUBLE,
    KW_VOID, KW_IF, KW_ELSE, KW_WHILE, KW_FOR,

    // OTHER
    UNKNOWN,
    E_O_F
};

// helper function to get string representation of token type
std::string tokenTypeToString(Tokentype type);

// represent a single token scanned from src
struct Token {
    Tokentype type;
    std::string lexeme;
    int line;
    int column;

    // helper function to print into console 
    void print() const;
};

#endif //PILLA_TOKEN_H
