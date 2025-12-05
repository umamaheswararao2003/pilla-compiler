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
    std::string returnType = parseType();

    Token name = consume(Tokentype::IDENTIFIER, "expected function name.");
    consume(Tokentype::LPAR,"Expected '('.");

    std::vector<std::pair<std::string, std::string>> parameters;
    if (!match(Tokentype::RPAR)) {
        do {
            std::string paramType = parseType();
            Token paramName = consume(Tokentype::IDENTIFIER, "Expected parameter name.");
            parameters.push_back({paramType, paramName.lexeme});
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
    // Check if current token is a type keyword
    Tokentype type = peek().type;
    if (type == Tokentype::KW_INT || type == Tokentype::KW_FLOAT || 
        type == Tokentype::KW_DOUBLE || type == Tokentype::KW_CHAR || 
        type == Tokentype::KW_STRING) {
        return parseVariableDecl();
    }
    //return
    if (peek().type == Tokentype::KW_RETURN) {
        return parseReturnStatement();
    }
    // if statement
    if (peek().type == Tokentype::KW_IF) {
        return parseIfStatement();
    }

    if (match(Tokentype::KW_WHILE)) {
        return parseWhileStatement();
    }

    if (match(Tokentype::KW_FOR)) {
        return parseForStatement();
    }

    // Fallback to expression statement
    return parsePrintStatement();
}

std::unique_ptr<StmtAST> Parser::parsePrintStatement() {
    auto expr = parseExpression();
    consume(Tokentype::SEMICOLON, "Expected ';' after expression.");
    return std::make_unique<PrintStmtAST>(std::move(expr));
}

std::unique_ptr<VariableDeclAST> Parser::parseVariableDecl() {
    std::string type = parseType();
    Token name = consume(Tokentype::IDENTIFIER, "Expected variable name.");
    std::unique_ptr<ExprAST> initializer = nullptr;
    if (match(Tokentype::ASSIGN)) {
        initializer = parseExpression();
    }
    consume(Tokentype::SEMICOLON, "Expected ';' after variable declaration.");
    return std::make_unique<VariableDeclAST>(type, name.lexeme, std::move(initializer));
}

std::unique_ptr<ReturnStmtAST> Parser::parseReturnStatement() {
    consume(Tokentype::KW_RETURN, "expected 'return'");
    auto expression = parseExpression();
    consume(Tokentype::SEMICOLON, "Expected ';' after return value.");
    return std::make_unique<ReturnStmtAST>(std::move(expression));
}

std::unique_ptr<IfStmtAST> Parser::parseIfStatement() {
    consume(Tokentype::KW_IF, "Expected 'if'");
    consume(Tokentype::LPAR, "Expected '(' after 'if'");
    auto condition = parseExpression();
    consume(Tokentype::RPAR, "Expected ')' after condition");
    
    consume(Tokentype::LBRACE, "Expected '{' after if condition");
    std::vector<std::unique_ptr<StmtAST>> thenBranch;
    while (!match(Tokentype::RBRACE) && !isAtEnd()) {
        thenBranch.push_back(parseStatement());
    }
    
    std::vector<std::unique_ptr<StmtAST>> elseBranch;
    if (match(Tokentype::KW_ELSE)) {
        consume(Tokentype::LBRACE, "Expected '{' after 'else'");
        while (!match(Tokentype::RBRACE) && !isAtEnd()) {
            elseBranch.push_back(parseStatement());
        }
    }
    
    return std::make_unique<IfStmtAST>(
        std::move(condition), 
        std::move(thenBranch), 
        std::move(elseBranch)
    );
}

std::unique_ptr<WhileStmtAST> Parser::parseWhileStatement() {
    consume(Tokentype::LPAR, "Expected '(' after 'while'");
    auto condition = parseExpression();
    consume(Tokentype::RPAR, "Expected ')' after condition");
    consume(Tokentype::LBRACE, "Expected '{' after while condition");
    std::vector<std::unique_ptr<StmtAST>> body;
    while (!match(Tokentype::RBRACE) && !isAtEnd()) {
        body.push_back(parseStatement());
    }

    return std::make_unique<WhileStmtAST>(std::move(condition), std::move(body));
}

std::unique_ptr<ForStmtAST> Parser::parseForStatement() {
    consume(Tokentype::LPAR, "Expected '(' after 'for'");
    std::unique_ptr<StmtAST> initializer = nullptr;
    if(!match(Tokentype::SEMICOLON)) {
        if(peek().type == Tokentype::KW_INT || 
            peek().type == Tokentype::KW_FLOAT || 
            peek().type == Tokentype::KW_DOUBLE) {
                initializer = parseVariableDecl();
        } else {
            auto expr = parseExpression();
            consume(Tokentype::SEMICOLON, "Expected ';' after initializer");
        }
    }

    //condition
    std::unique_ptr<ExprAST> condition = nullptr;
    if(!match(Tokentype::SEMICOLON)) {
        condition = parseExpression();
        consume(Tokentype::SEMICOLON, "Expected ';' after condition");
    }

    // increment
    std::unique_ptr<ExprAST> increment = nullptr;
    if(!match(Tokentype::RPAR)) {
        increment = parseExpression();
        consume(Tokentype::RPAR, "Expected ')' after increment");
    }

    // body
    consume(Tokentype::LBRACE, "Expected '{' after for statement");
    std::vector<std::unique_ptr<StmtAST>> body;
    while (!match(Tokentype::RBRACE) && !isAtEnd()) {
        body.push_back(parseStatement());
    }

    return std::make_unique<ForStmtAST>(
        std::move(initializer),
        std::move(condition),
        std::move(increment),
        std::move(body)
    );
}

// Helper function to get operator precedence
// Higher number = higher precedence
int Parser::getOperatorPrecedence(Tokentype op) {
    switch (op) {
        case Tokentype::MULTIPLY:
        case Tokentype::DIVIDE:
        case Tokentype::MODULO:
            return 4;  // Highest precedence
        case Tokentype::PLUS:
        case Tokentype::MINUS:
            return 3;  // Medium-high precedence
        case Tokentype::EQUAL_EQUAL:
        case Tokentype::NOT_EQUAL:
        case Tokentype::LESS_THAN:
        case Tokentype::GRE_THAN:
        case Tokentype::LESS_EQUAL:
        case Tokentype::GREATER_EQUAL:
            return 2;  // Lower precedence (comparison)
        case Tokentype::ASSIGN:
            return 1;  // Lowest precedence (assignment)
        default:
            return 0;  // Not a binary operator
    }
}

// Helper function to check if token is a binary operator
bool Parser::isBinaryOperator(Tokentype type) {
    return getOperatorPrecedence(type) > 0;
}

// base for parsing expressions
std::unique_ptr<ExprAST> Parser::parseExpression(){
    return parseBinaryExpression(0);
}

// Unified binary expression parser using precedence climbing
std::unique_ptr<ExprAST> Parser::parseBinaryExpression(int minPrecedence) {
    auto left = parsePrimary();
    
    while (!isAtEnd() && isBinaryOperator(peek().type)) {
        Tokentype opType = peek().type;
        int precedence = getOperatorPrecedence(opType);
        
        // If this operator has lower precedence than minimum, stop
        if (precedence < minPrecedence) {
            break;
        }
        
        // Consume the operator
        Token op = peek();
        current++;
        
        // Parse right side with higher precedence (for left-associativity)
        auto right = parseBinaryExpression(precedence + 1);
        
        // Create binary expression node
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

    if(match(Tokentype::FLOAT_LITERAL)) {
        double value = std::stod(previous().lexeme);
        return std::make_unique<FloatExprAST>(value);
    }

    if(match(Tokentype::STRING_LITERAL)) {
        return std::make_unique<StringExprAST>(previous().lexeme);
    }

    if(match(Tokentype::CHAR_LITERAL)) {
    
        std::string lexeme = previous().lexeme;
        char val = lexeme.length() > 0 ? lexeme[0] : '\0'; 
        return std::make_unique<CharExprAST>(val);
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
                            " but Found - " + peek().lexeme);
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

std::string Parser::parseType() {
    if (match(Tokentype::KW_VOID)) return "void";
    if (match(Tokentype::KW_INT)) return "int";
    if (match(Tokentype::KW_FLOAT)) return "float";
    if (match(Tokentype::KW_DOUBLE)) return "double";
    if (match(Tokentype::KW_CHAR)) return "char";
    if (match(Tokentype::KW_STRING)) return "string";
    
    throw std::runtime_error("Expected type specifier.");
}


