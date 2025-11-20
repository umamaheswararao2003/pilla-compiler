#ifndef PILLA_AST_H
#define PILLA_AST_H


#include "lexer/Token.h" // For TokenType 
#include <string>
#include <vector>
#include <optional>
#include <memory>


enum class Type { Int, Invalid};

// this defines the nodes of the abstract syntax tree

// forward declare 
class ASTVisitor;

//  ------ Base class --------

// base forall expression nodes

class ExprAST {
    public:
    virtual ~ExprAST() = default;

    virtual long accept(ASTVisitor& visitor) = 0;

    Type inferredType = Type::Invalid;
    std::optional<long> constantValue = std::nullopt;

};

// base for all expression nodes
class StmtAST {
    public:
    virtual ~StmtAST() = default;

    virtual long accept(ASTVisitor& visitor) = 0;
};

// expression nodes

// numeric
class NumberExprAST : public ExprAST {
    public:
    long value;
    NumberExprAST(long val) : value(val) {}
    long accept(ASTVisitor& visitor) override;
};


// binary 

class BinaryExprAST : public ExprAST {
    public:
    Tokentype op;
    std::unique_ptr<ExprAST> left;
    std::unique_ptr<ExprAST> right;
    BinaryExprAST(Tokentype op, std::unique_ptr<ExprAST> l, std::unique_ptr<ExprAST> r)
        : op(op), left(std::move(l)), right(std::move(r)) {}

    long accept(ASTVisitor& visitor) override;
};

// statement nodes

//node for return 
class ReturnStmtAST : public StmtAST {
    public:
    std::unique_ptr<ExprAST> expression;

    ReturnStmtAST(std::unique_ptr<ExprAST> expr) : expression(std::move(expr)) {}

    long accept(ASTVisitor& visitor) override;
};

class FunctionAST {
    public:
    std::string name;
    std::unique_ptr<StmtAST> body;

    FunctionAST(const std::string& name, std::unique_ptr<StmtAST> body)
        : name(name), body(std::move(body)) {}
        
    long accept(ASTVisitor& visitor);
};

// node for full program 
class ProgramAST {
    public:
    std::unique_ptr<FunctionAST> function;

    ProgramAST(std::unique_ptr<FunctionAST> func)
        : function(std::move(func)) {}

    long accept(ASTVisitor& visitor);
};

// visitor baseclass
class ASTVisitor {
    public:
    virtual ~ASTVisitor() = default;
    virtual long visit(ProgramAST& node) = 0;
    virtual long visit(FunctionAST& node) = 0;
    virtual long visit(ReturnStmtAST& node) = 0;
    virtual long visit(NumberExprAST& node) = 0;
    virtual long visit(BinaryExprAST& node) = 0;
};

// -- simple imlementations of accept()

inline long ProgramAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long FunctionAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long ReturnStmtAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long NumberExprAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long BinaryExprAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}


#endif //PILLA_AST_H
