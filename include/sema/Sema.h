#ifndef PILLA_SEMANTICS_H
#define PILLA_SEMANTICS_H

#include "parser/AST.h"
#include <string>
#include <vector>

class Semantics : public ASTVisitor {
    public:
    // return true if it is valid 
    bool analyze(ProgramAST& program);

    // visitor methods
    long visit(ProgramAST& node) override;
    long visit(FunctionAST& node) override;
    long visit(ReturnStmtAST& node) override;
    long visit(BinaryExprAST& node) override;
    long visit(NumberExprAST& node) override;

    private:
    // helper
    void error(const std::string& message);
    bool hasError = false;

    Type currentReturntype = Type::Invalid;
};

#endif //PILLA_SEMANTICS_H