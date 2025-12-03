# AST (Abstract Syntax Tree) Component

## Purpose
The AST represents the syntactic structure of the program as a tree. Each node represents a construct in the source code (function, statement, expression, etc.). Uses the Visitor pattern for traversal.

## Files
- `include/parser/AST.h` - AST node definitions and visitor interface

## Boilerplate Code

### AST.h
```cpp
#ifndef PILLA_AST_H
#define PILLA_AST_H

#include <string>
#include <vector>
#include <memory>
#include "lexer/Token.h"

// Forward declarations
class ProgramAST;
class FunctionAST;
class VariableDeclAST;
class ReturnStmtAST;
class PrintStmtAST;
class NumberExprAST;
class FloatExprAST;
class StringExprAST;
class CharExprAST;
class VariableExprAST;
class CallExprAST;
class BinaryExprAST;

// Type enum for semantic analysis
enum class Type {
    Int,
    Float,
    Double,
    Char,
    String,
    Void,
    Invalid
};

// Visitor interface (for traversal)
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    virtual long visit(ProgramAST& node) = 0;
    virtual long visit(FunctionAST& node) = 0;
    virtual long visit(VariableDeclAST& node) = 0;
    virtual long visit(ReturnStmtAST& node) = 0;
    virtual long visit(PrintStmtAST& node) = 0;
    virtual long visit(NumberExprAST& node) = 0;
    virtual long visit(FloatExprAST& node) = 0;
    virtual long visit(StringExprAST& node) = 0;
    virtual long visit(CharExprAST& node) = 0;
    virtual long visit(VariableExprAST& node) = 0;
    virtual long visit(CallExprAST& node) = 0;
    virtual long visit(BinaryExprAST& node) = 0;
};

// Base AST node
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual long accept(ASTVisitor& visitor) = 0;
};

// Program (root node)
class ProgramAST : public ASTNode {
public:
    std::vector<std::unique_ptr<FunctionAST>> functions;
    
    ProgramAST(std::vector<std::unique_ptr<FunctionAST>> funcs)
        : functions(std::move(funcs)) {}
    
    long accept(ASTVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

// Function definition
class FunctionAST : public ASTNode {
public:
    std::string returnType;
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters; // (type, name)
    std::vector<std::unique_ptr<StmtAST>> body;
    
    FunctionAST(std::string retType, std::string funcName,
                std::vector<std::pair<std::string, std::string>> params,
                std::vector<std::unique_ptr<StmtAST>> stmts)
        : returnType(retType), name(funcName), 
          parameters(std::move(params)), body(std::move(stmts)) {}
    
    long accept(ASTVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

// Base statement node
class StmtAST : public ASTNode {};

// Variable declaration
class VariableDeclAST : public StmtAST {
public:
    std::string type;
    std::string name;
    std::unique_ptr<ExprAST> initializer;
    
    VariableDeclAST(std::string varType, std::string varName,
                    std::unique_ptr<ExprAST> init)
        : type(varType), name(varName), initializer(std::move(init)) {}
    
    long accept(ASTVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

// Return statement
class ReturnStmtAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> expression;
    
    ReturnStmtAST(std::unique_ptr<ExprAST> expr)
        : expression(std::move(expr)) {}
    
    long accept(ASTVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

// Print statement (expression statement)
class PrintStmtAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> expression;
    
    PrintStmtAST(std::unique_ptr<ExprAST> expr)
        : expression(std::move(expr)) {}
    
    long accept(ASTVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

// Base expression node
class ExprAST : public ASTNode {
public:
    Type inferredType = Type::Invalid;  // Set during semantic analysis
};

// Number literal
class NumberExprAST : public ExprAST {
public:
    long value;
    
    NumberExprAST(long val) : value(val) {}
    
    long accept(ASTVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

// Float literal
class FloatExprAST : public ExprAST {
public:
    double value;
    
    FloatExprAST(double val) : value(val) {}
    
    long accept(ASTVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

// String literal
class StringExprAST : public ExprAST {
public:
    std::string value;
    
    StringExprAST(std::string val) : value(val) {}
    
    long accept(ASTVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

// Character literal
class CharExprAST : public ExprAST {
public:
    char value;
    
    CharExprAST(char val) : value(val) {}
    
    long accept(ASTVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

// Variable reference
class VariableExprAST : public ExprAST {
public:
    std::string name;
    
    VariableExprAST(std::string varName) : name(varName) {}
    
    long accept(ASTVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

// Function call
class CallExprAST : public ExprAST {
public:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;
    
    CallExprAST(std::string funcName, std::vector<std::unique_ptr<ExprAST>> arguments)
        : callee(funcName), args(std::move(arguments)) {}
    
    long accept(ASTVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

// Binary expression (e.g., a + b)
class BinaryExprAST : public ExprAST {
public:
    Tokentype op;
    std::unique_ptr<ExprAST> left;
    std::unique_ptr<ExprAST> right;
    
    BinaryExprAST(Tokentype operation, std::unique_ptr<ExprAST> lhs,
                  std::unique_ptr<ExprAST> rhs)
        : op(operation), left(std::move(lhs)), right(std::move(rhs)) {}
    
    long accept(ASTVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

#endif // PILLA_AST_H
```

## How It Connects to Other Components

### Created By
- **Parser**: Builds AST nodes during parsing

### Used By
- **Semantic Analyzer**: Traverses AST to check types and semantics
- **Code Generator**: Traverses AST to generate LLVM IR
- **AST Printer**: Traverses AST to visualize structure

### Data Flow
```
Parser → builds → AST → traversed by → Sema/Codegen (via Visitor pattern)
```

## Key Concepts

1. **Tree Structure**: Hierarchical representation of program structure
2. **Visitor Pattern**: Allows different operations on AST without modifying node classes
3. **Smart Pointers**: Uses `std::unique_ptr` for automatic memory management
4. **Type Inference**: `inferredType` field populated during semantic analysis
5. **Polymorphism**: Virtual `accept()` method enables visitor pattern
