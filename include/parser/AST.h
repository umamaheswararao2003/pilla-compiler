#ifndef PILLA_AST_H
#define PILLA_AST_H


#include "lexer/Token.h" // For TokenType 
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <utility>

enum class Type { Int, Float, Double, Char, String, Void, Invalid};

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

class FloatExprAST : public ExprAST {
    public:
    double value;
    FloatExprAST(double val) : value(val) {}
    long accept(ASTVisitor& visitor) override;
};

class StringExprAST : public ExprAST {
    public:
    std::string value;
    StringExprAST(const std::string& val) : value(val) {}
    long accept(ASTVisitor& visitor) override;
};

class CharExprAST : public ExprAST {
    public:
    char value;
    CharExprAST(char val) : value(val) {}
    long accept(ASTVisitor& visitor) override;
};

// variable usage
class VariableExprAST : public ExprAST {
    public:
    std::string name;
    VariableExprAST(const std::string& name) : name(name) {}
    long accept(ASTVisitor& visitor) override;
};

// function call
class CallExprAST : public ExprAST {
    public:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;
    CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args)
        : callee(callee), args(std::move(args)) {}
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

// variable declaration
class VariableDeclAST : public StmtAST {
    public:
    std::string type;
    std::string name;
    std::unique_ptr<ExprAST> initializer;
    VariableDeclAST(const std::string& type, const std::string& name, std::unique_ptr<ExprAST> init)
        : type(type), name(name), initializer(std::move(init)) {}
    long accept(ASTVisitor& visitor) override;
};

//node for return 
class ReturnStmtAST : public StmtAST {
    public:
    std::unique_ptr<ExprAST> expression;

    ReturnStmtAST(std::unique_ptr<ExprAST> expr) : expression(std::move(expr)) {}

    long accept(ASTVisitor& visitor) override;
};

// expression statement
class PrintStmtAST : public StmtAST {
    public:
    std::unique_ptr<ExprAST> expression;
    PrintStmtAST(std::unique_ptr<ExprAST> expr) : expression(std::move(expr)) {}
    long accept(ASTVisitor& visitor) override;
};

// if-else statement
class IfStmtAST : public StmtAST {
    public:
    std::unique_ptr<ExprAST> condition;
    std::vector<std::unique_ptr<StmtAST>> thenBranch;
    std::vector<std::unique_ptr<StmtAST>> elseBranch;
    
    IfStmtAST(std::unique_ptr<ExprAST> cond, 
              std::vector<std::unique_ptr<StmtAST>> thenB,
              std::vector<std::unique_ptr<StmtAST>> elseB = {})
        : condition(std::move(cond)), 
          thenBranch(std::move(thenB)), 
          elseBranch(std::move(elseB)) {}
    
    long accept(ASTVisitor& visitor) override;
};

class WhileStmtAST : public StmtAST {
    public:
    std::unique_ptr<ExprAST> condition;
    std::vector<std::unique_ptr<StmtAST>> body;
    WhileStmtAST(std::unique_ptr<ExprAST> cond, 
                  std::vector<std::unique_ptr<StmtAST>> bodyStmts)
            : condition (std::move(cond)), body(std::move(bodyStmts)) {}

    long accept(ASTVisitor& visitor) override;
};

class ForStmtAST : public StmtAST {
    public:
    std::unique_ptr<StmtAST> initializer;
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<ExprAST> increment;
    std::vector<std::unique_ptr<StmtAST>> body;

    ForStmtAST(std::unique_ptr<StmtAST> init, std::unique_ptr<ExprAST> cond,
             std::unique_ptr<ExprAST> incr, std::vector<std::unique_ptr<StmtAST>> bodyStmts)
        : initializer(std::move(init)), condition(std::move(cond)), 
            increment(std::move(incr)), body(std::move(bodyStmts)) {}

    long accept(ASTVisitor& visitor);
};


class FunctionAST {
    public:
    std::string returnType;
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters; 
    std::vector<std::unique_ptr<StmtAST>> body;

    FunctionAST(const std::string& returnType, const std::string& name, 
                std::vector<std::pair<std::string, std::string>> params,
                std::vector<std::unique_ptr<StmtAST>> body)
        : returnType(returnType), name(name), parameters(std::move(params)), body(std::move(body)) {}
        
    long accept(ASTVisitor& visitor);
};

// node for full program 
class ProgramAST {
    public:
    std::vector<std::unique_ptr<FunctionAST>> functions;

    ProgramAST(std::vector<std::unique_ptr<FunctionAST>> funcs)
        : functions(std::move(funcs)) {}

    long accept(ASTVisitor& visitor);
};

// visitor baseclass
class ASTVisitor {
    public:
    virtual ~ASTVisitor() = default;
    virtual long visit(ProgramAST& node) = 0;
    virtual long visit(FunctionAST& node) = 0;
    virtual long visit(VariableDeclAST& node) = 0;
    virtual long visit(ReturnStmtAST& node) = 0;
    virtual long visit(PrintStmtAST& node) = 0;
    virtual long visit(IfStmtAST& node) = 0;
    virtual long visit(WhileStmtAST& node) = 0;
    virtual long visit(ForStmtAST& node) = 0;
    virtual long visit(NumberExprAST& node) = 0;
    virtual long visit(VariableExprAST& node) = 0;
    virtual long visit(CallExprAST& node) = 0;
    virtual long visit(BinaryExprAST& node) = 0;
    virtual long visit(FloatExprAST& node) = 0;
    virtual long visit(StringExprAST& node) = 0;
    virtual long visit(CharExprAST& node) = 0;
};

// -- simple imlementations of accept()

inline long ProgramAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long FunctionAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long VariableDeclAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long ReturnStmtAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long PrintStmtAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long IfStmtAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long WhileStmtAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long ForStmtAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long NumberExprAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long VariableExprAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long CallExprAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long BinaryExprAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long FloatExprAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long StringExprAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

inline long CharExprAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}

#endif //PILLA_AST_H
