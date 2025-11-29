#ifndef PILLA_PARSER_H
#define PILLA_PARSER_H

#include "lexer/Token.h"
#include "parser/AST.h"
#include <vector>
#include <memory>

// parser takes a vecor of tokens and construct a AST

class Parser {
    public:

    // parser for given stream of tokens
    Parser(const std::vector<Token>& tokens);

    // returns the root node of thr AST
    std::unique_ptr<ProgramAST> parse();

    private:
    std::vector<Token> tokens;
    size_t current =0;

    // grammar parsing methods , implementing recursive descent parser

    // parses function
    std::unique_ptr<FunctionAST> parseFunction();

    // parse statement
    std::unique_ptr<StmtAST> parseStatement();
                            
    // parse variable declaration
    std::unique_ptr<VariableDeclAST> parseVariableDecl();

    // parse return statement
    std::unique_ptr<ReturnStmtAST> parseReturnStatement();

    // parse an expression
    std::unique_ptr<ExprAST> parseExpression();

    // parse an additive expression
    std::unique_ptr<ExprAST> parseAdditiveExpression();

    // parse primary expression 
    std::unique_ptr<ExprAST> parsePrimary();

    // parse type
    std::string parseType();

    // -----some utility functions

    // does token match the expected type 
    bool match(Tokentype type);

    // gives the error msg if not matched and gives the matched token 
    Token consume(Tokentype type, const std::string& message);

    // current token
    Token peek();
    // previous token 
    Token previous();

    // chech if we are at the end of the token stream
    bool isAtEnd();


};

#endif //PILLA_PARSER_H

