#include "parser/Parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

// main entry point 
std::unique_ptr<ProgramAST> Parser::parse() {
    std::vector<std::unique_ptr<FunctionAST>> functions;
    try{
        while (!isAtEnd()) {
            functions.push_back(parseFunction());
        }
        return std::make_unique<ProgramAST>(std::move(functions));
    } catch(const std::exception& e) {
        std::cerr << "erroe :" << e.what() << std::endl;
        return nullptr;
    }
}

// grammar parsing methods 

std::unique_ptr<FunctionAST> Parser::parseFunction() {
    consume(Tokentype::KW_INT, "expected 'int' .");
    std::string returnType = "int"; // Currently only int supported

    Token name = consume(Tokentype::IDENTIFIER, "expected function name.");
    consume(Tokentype::LPAR,"Expected '('.");

    std::vector<std::pair<std::string, std::string>> parameters;
    if (!match(Tokentype::RPAR)) {
        do {
            consume(Tokentype::KW_INT, "Expected parameter type 'int'.");
            Token paramName = consume(Tokentype::IDENTIFIER, "Expected parameter name.");
            parameters.push_back({"int", paramName.lexeme});
        } while (match(Tokentype::COMMA));
        consume(Tokentype::RPAR, "Expected ')'.");
    }

    consume(Tokentype::LBRACE,"Expected '{'.");

    std::vector<std::unique_ptr<StmtAST>> body;
    while (!match(Tokentype::RBRACE) && !isAtEnd()) {
        body.push_back(parseStatement());
    }

    return std::make_unique<FunctionAST>(returnType, name.lexeme, std::move(parameters), std::move(body));
}

// base for parsing statements

std::unique_ptr<StmtAST> Parser::parseStatement() {
    // variable declaration
    if (peek().type == Tokentype::KW_INT) {
        return parseVariableDecl();
    }
    //return
    if (peek().type == Tokentype::KW_RETURN) {
        return parseReturnStatement();
    }
    throw std::runtime_error("expected a statement");
}

std::unique_ptr<VariableDeclAST> Parser::parseVariableDecl() {
    consume(Tokentype::KW_INT, "Expected 'int'.");
    Token name = consume(Tokentype::IDENTIFIER, "Expected variable name.");
    std::unique_ptr<ExprAST> initializer = nullptr;
    if (match(Tokentype::ASSIGN)) {
        initializer = parseExpression();
    }
    consume(Tokentype::SEMICOLON, "Expected ';' after variable declaration.");
    return std::make_unique<VariableDeclAST>("int", name.lexeme, std::move(initializer));
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

    if (match(Tokentype::IDENTIFIER)) {
        std::string name = previous().lexeme;
        if (match(Tokentype::LPAR)) {
            // Function call
            std::vector<std::unique_ptr<ExprAST>> args;
            if (!match(Tokentype::RPAR)) {
                do {
                    args.push_back(parseExpression());
                } while (match(Tokentype::COMMA));
                consume(Tokentype::RPAR, "Expected ')' after arguments.");
            }
            return std::make_unique<CallExprAST>(name, std::move(args));
        } else {
            // Variable usage
            return std::make_unique<VariableExprAST>(name);
        }
    }

    throw std::runtime_error(" expected expression, found " + peek().lexeme);
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


