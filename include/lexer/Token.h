#ifndef PILLA_TOKEN_H
#define PILLA_TOKEN_H

#include <string>

// types of tokens we want to recognise
enum class Tokentype {
    // single character
    LPAR, RPAR,LBRACE, RBRACE,LESS_THAN,
    GRE_THAN, SEMICOLON, PLUS, POUND,

    // NUMBERS AND NAMES
    NUMBER, IDENTIFIER,

    // KEYWORDS
    KW_INT, KW_RETURN,
    KW_INCLUDE,

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

    // helper function to print into console 
    void print() const;
};

#endif //PILLA_TOKEN_H
