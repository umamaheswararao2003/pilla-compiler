#include "lexer/Token.h"
#include <iostream>
#include <iomanip>
#include <map>

namespace {
    // convert tokentype enums to printable strings
    std::map<Tokentype, std::string> tokenTypeStrings = {
        {Tokentype::LPAR, "LPAR"},
        {Tokentype::RPAR, "RPAR"},
        {Tokentype::LBRACE, "LBRACE"},
        {Tokentype::ASSIGN, "ASSIGN"},
        {Tokentype::COMMA, "COMMA"},
        {Tokentype::RBRACE, "RBRACE"},
        {Tokentype::LESS_THAN, "LESS_THAN"},
        {Tokentype::GRE_THAN, "GRE_THAN"},
        {Tokentype::SEMICOLON, "SEMICOLON"},
        {Tokentype::PLUS, "PLUS"},
        {Tokentype::POUND, "POUND"},
        {Tokentype::NUMBER, "NUMBER"},
        {Tokentype::FLOAT_LITERAL, "FLOAT_LITERAL"},
        {Tokentype::CHAR_LITERAL, "CHAR_LITERAL"},
        {Tokentype::STRING_LITERAL, "STRING_LITERAL"},
        {Tokentype::IDENTIFIER, "IDENTIFIER"},
        {Tokentype::KW_INT, "KW_INT"},
        {Tokentype::KW_FLOAT, "KW_FLOAT"},
        {Tokentype::KW_CHAR, "KW_CHAR"},
        {Tokentype::KW_STRING, "KW_STRING"},
        {Tokentype::KW_DOUBLE, "KW_DOUBLE"},
        {Tokentype::KW_RETURN, "KW_RETURN"},
        {Tokentype::UNKNOWN, "UNKNOWN"},
        {Tokentype::E_O_F, "EOF"}
    };
}

// HELPER FUNCTION IMPLEMENTATION

std::string tokenTypeToString(Tokentype type) {
    try{
        return tokenTypeStrings.at(type);
    } catch (const std::out_of_range&) {
        return "INVALID_RA_BABU";
    }
}

// TOKEN:: PRINT
void Token::print() const {
    std::cout << "type: " << std::left << std::setw(16) << tokenTypeToString(type)
              << "\tLexem: '" << lexeme << "'"
              << "\tLoc: " << line << ":" << column << "\n";
}