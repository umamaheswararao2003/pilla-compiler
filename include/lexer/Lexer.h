#ifndef PILLA_LEXER_H
#define PILLA_LEXER_H

#include "lexer/Token.h"
#include <string>
#include <vector>

// reponsible for breaking down the source code 

class Lexer {
    public:
    //lexer for souce code
    Lexer(const std::string& source);
    // scans souce code and returns vector of all strings
    std::vector<Token> scanTokens();

    private:
    std::string sourcecode;
    size_t currentPos;
    int line;
    int column;
    int tokenStartLine;
    int tokenStartColumn;

    // -- helpers methods ---

    // scans and returns next token from source
    Token scanToken();
    // gives present character and advances the position
    char advance();
    //check if reached end of the souce code 
    bool isAtEnd();
    // returns the character without consuming 
    char peek();

    // create a token object 
    Token makeToken(Tokentype type , const std::string& lexeme);

    // skips over white spaces 
    void skipWhitespace();

    // number literal (int or float)
    Token number();

    // string literal
    Token string();

    // character literal
    Token character();

    // scan identifier or a literal 
    Token identifier();

};

#endif //PILLA_LEXER_H