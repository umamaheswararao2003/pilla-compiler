#include "parser/Parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

// main entry point 
std::unique_ptr<ProgramAST> Parser::parse() {
    try{
        // for now only one function
        auto function = parseFunction();
        return std::make_unique<ProgramAST>(std::move(function));
    } catch(const std::exception& e) {
        std::cerr << "erroe :" << e.what() << std::endl;
        return nullptr;
    }
}

// grammar parsing methods 

std::unique_ptr<FunctionAST> Parser::parseFunction() {
    consume(Tokentype::KW_INT, "expected 'int' .");
    Token name = consume(Tokentype::IDENTIFIER, "expected function name.");
    consume(Tokentype::LPAR,"Expected '('.");
    consume(Tokentype::RPAR,"Expected ')'.");
    consume(Tokentype::LBRACE,"Expected '{'.");

    // foe now only one statement to parse
    auto body = parseStatement();

    consume(Tokentype::RBRACE, "Expected '}'.");

    if(!isAtEnd()) {
        throw std::runtime_error("expected end of the file after function");
    }

    return std::make_unique<FunctionAST>(name.lexeme, std::move(body));
}

// base for parsing statements

std::unique_ptr<StmtAST> Parser::parseStatement() {
    //return
    if (peek().type == Tokentype::KW_RETURN) {
        return parseReturnStatement();
    }
    throw std::runtime_error("expected a 'return' statement");
}

std::unique_ptr<ReturnStmtAST> Parser::parseReturnStatement() {
    consume(Tokentype::KW_RETURN, "expected 'return'");
    auto expression = parseExpression();
    consume(Tokentype::SEMICOLON, "Expected ';' after return value.");
    return std::make_unique<ReturnStmtAST>(std::move(expression));
}

// base for parsing expressions
std::unique_ptr<ExprAST> Parser::parseExpression(){
    // for now only operator
    return parseAdditiveExpression();
}

std::unique_ptr<ExprAST> Parser::parseAdditiveExpression() {
    auto left = parsePrimary();

    // check for + operator
    while (match(Tokentype::PLUS)) {
        Token op = previous();
        auto right = parsePrimary();

        // binary expression node
        left = std::make_unique<BinaryExprAST>(op.type, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<ExprAST> Parser::parsePrimary() {
    if(match(Tokentype::NUMBER )) {
        // convert the number to a long
        long value = std::stol(previous().lexeme);
        return std::make_unique<NumberExprAST>(value);
    }

    throw std::runtime_error(" expected a number" + previous().lexeme);
}


// some needed functions

bool Parser::match(Tokentype type) {
    if (isAtEnd()) return false;
    if (peek().type == type) {
        current++;
        return true;
    }
    return false;
}

Token Parser::consume(Tokentype type, const std::string& message) {
    if (isAtEnd()) {
        throw std::runtime_error(" unexpected end of file"+ message);
    }

    if (peek().type == type) {
        current++;
        return previous();
    }
    throw std::runtime_error("syntax error :" + message +
                            " but Found" + peek().lexeme);
}

Token Parser::peek() {
    return tokens[current];
}

Token Parser::previous() {
    return tokens[current-1];
}

bool Parser::isAtEnd() {
    return peek().type == Tokentype::E_O_F;
}


