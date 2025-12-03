# Parser Component

## Purpose
The Parser performs syntactic analysis, converting a stream of tokens into an Abstract Syntax Tree (AST). It checks that the token sequence follows the language grammar rules.

## Files
- `include/parser/Parser.h` - Parser class declaration
- `src/parser/Parser.cpp` - Parser implementation

## Boilerplate Code

### Parser.h
```cpp
#ifndef PILLA_PARSER_H
#define PILLA_PARSER_H

#include "lexer/Token.h"
#include "parser/AST.h"
#include <vector>
#include <memory>

class Parser {
public:
    // Constructor: takes token stream from lexer
    Parser(const std::vector<Token>& tokens);
    
    // Main entry point: parse entire program
    std::unique_ptr<ProgramAST> parse();

private:
    std::vector<Token> tokens;  // Token stream
    size_t current = 0;         // Current token index
    
    // Grammar parsing methods
    std::unique_ptr<FunctionAST> parseFunction();
    std::unique_ptr<StmtAST> parseStatement();
    std::unique_ptr<VariableDeclAST> parseVariableDecl();
    std::unique_ptr<ReturnStmtAST> parseReturnStatement();
    std::unique_ptr<StmtAST> parsePrintStatement();
    
    std::unique_ptr<ExprAST> parseExpression();
    std::unique_ptr<ExprAST> parseAdditiveExpression();
    std::unique_ptr<ExprAST> parsePrimary();
    
    std::string parseType();
    
    // Helper methods
    bool match(Tokentype type);              // Check and consume token
    Token consume(Tokentype type, const std::string& message);  // Expect token
    Token peek();                            // Look at current token
    Token previous();                        // Get previous token
    bool isAtEnd();                          // Check if at EOF
};

#endif // PILLA_PARSER_H
```

### Parser.cpp (Key Methods)
```cpp
#include "parser/Parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

// Main parsing entry point
std::unique_ptr<ProgramAST> Parser::parse() {
    std::vector<std::unique_ptr<FunctionAST>> functions;
    try {
        while (!isAtEnd()) {
            functions.push_back(parseFunction());
        }
        return std::make_unique<ProgramAST>(std::move(functions));
    } catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        return nullptr;
    }
}

// Parse function definition
// Grammar: type IDENTIFIER '(' params ')' '{' statements '}'
std::unique_ptr<FunctionAST> Parser::parseFunction() {
    std::string returnType = parseType();
    Token name = consume(Tokentype::IDENTIFIER, "Expected function name.");
    consume(Tokentype::LPAR, "Expected '('.");

    // Parse parameters
    std::vector<std::pair<std::string, std::string>> parameters;
    if (!match(Tokentype::RPAR)) {
        do {
            std::string paramType = parseType();
            Token paramName = consume(Tokentype::IDENTIFIER, "Expected parameter name.");
            parameters.push_back({paramType, paramName.lexeme});
        } while (match(Tokentype::COMMA));
        consume(Tokentype::RPAR, "Expected ')'.");
    }

    consume(Tokentype::LBRACE, "Expected '{'.");

    // Parse function body
    std::vector<std::unique_ptr<StmtAST>> body;
    while (!match(Tokentype::RBRACE) && !isAtEnd()) {
        body.push_back(parseStatement());
    }

    return std::make_unique<FunctionAST>(returnType, name.lexeme, 
                                         std::move(parameters), std::move(body));
}

// Parse statement
std::unique_ptr<StmtAST> Parser::parseStatement() {
    // Check for variable declaration
    Tokentype type = peek().type;
    if (type == Tokentype::KW_INT || type == Tokentype::KW_FLOAT || 
        type == Tokentype::KW_DOUBLE || type == Tokentype::KW_CHAR || 
        type == Tokentype::KW_STRING) {
        return parseVariableDecl();
    }
    
    // Check for return statement
    if (peek().type == Tokentype::KW_RETURN) {
        return parseReturnStatement();
    }
    
    // Otherwise, expression statement
    return parsePrintStatement();
}

// Parse variable declaration
// Grammar: type IDENTIFIER ['=' expression] ';'
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

// Parse return statement
// Grammar: 'return' expression ';'
std::unique_ptr<ReturnStmtAST> Parser::parseReturnStatement() {
    consume(Tokentype::KW_RETURN, "Expected 'return'");
    auto expression = parseExpression();
    consume(Tokentype::SEMICOLON, "Expected ';' after return value.");
    return std::make_unique<ReturnStmtAST>(std::move(expression));
}

// Parse expression statement
std::unique_ptr<StmtAST> Parser::parsePrintStatement() {
    auto expr = parseExpression();
    consume(Tokentype::SEMICOLON, "Expected ';' after expression.");
    return std::make_unique<PrintStmtAST>(std::move(expr));
}

// Parse expression (currently only addition)
std::unique_ptr<ExprAST> Parser::parseExpression() {
    return parseAdditiveExpression();
}

// Parse additive expression
// Grammar: primary ('+' primary)*
std::unique_ptr<ExprAST> Parser::parseAdditiveExpression() {
    auto left = parsePrimary();

    while (match(Tokentype::PLUS)) {
        Token op = previous();
        auto right = parsePrimary();
        left = std::make_unique<BinaryExprAST>(op.type, std::move(left), std::move(right));
    }

    return left;
}

// Parse primary expression
// Grammar: NUMBER | FLOAT | STRING | CHAR | IDENTIFIER | call
std::unique_ptr<ExprAST> Parser::parsePrimary() {
    if (match(Tokentype::NUMBER)) {
        long value = std::stol(previous().lexeme);
        return std::make_unique<NumberExprAST>(value);
    }

    if (match(Tokentype::FLOAT_LITERAL)) {
        double value = std::stod(previous().lexeme);
        return std::make_unique<FloatExprAST>(value);
    }

    if (match(Tokentype::STRING_LITERAL)) {
        return std::make_unique<StringExprAST>(previous().lexeme);
    }

    if (match(Tokentype::CHAR_LITERAL)) {
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
            // Variable reference
            return std::make_unique<VariableExprAST>(name);
        }
    }

    throw std::runtime_error("Expected expression, found " + peek().lexeme);
}

// Parse type specifier
std::string Parser::parseType() {
    if (match(Tokentype::KW_VOID)) return "void";
    if (match(Tokentype::KW_INT)) return "int";
    if (match(Tokentype::KW_FLOAT)) return "float";
    if (match(Tokentype::KW_DOUBLE)) return "double";
    if (match(Tokentype::KW_CHAR)) return "char";
    if (match(Tokentype::KW_STRING)) return "string";
    
    throw std::runtime_error("Expected type specifier.");
}

// Helper: match and consume token
bool Parser::match(Tokentype type) {
    if (isAtEnd()) return false;
    if (peek().type == type) {
        current++;
        return true;
    }
    return false;
}

// Helper: consume expected token
Token Parser::consume(Tokentype type, const std::string& message) {
    if (isAtEnd()) {
        throw std::runtime_error("Unexpected end of file: " + message);
    }
    if (peek().type == type) {
        current++;
        return previous();
    }
    throw std::runtime_error("Syntax error: " + message + " but found " + peek().lexeme);
}

Token Parser::peek() {
    return tokens[current];
}

Token Parser::previous() {
    return tokens[current - 1];
}

bool Parser::isAtEnd() {
    return peek().type == Tokentype::E_O_F;
}
```

## How It Connects to Other Components

### Input
- **Token Stream**: Vector of tokens from Lexer

### Output
- **AST**: Abstract Syntax Tree representing program structure

### Data Flow
```
Lexer → [Tokens] → Parser.parse() → AST → Semantic Analyzer
```

## Key Concepts

1. **Recursive Descent**: Each grammar rule becomes a parsing method
2. **Lookahead**: Using `peek()` to decide which rule to apply
3. **Error Handling**: Throwing exceptions with descriptive messages
4. **Precedence**: Expression parsing respects operator precedence
5. **Grammar Rules**: Each method corresponds to a production rule
