#ifndef PILLA_ASTPRINTER_H
#define PILLA_ASTPRINTER_H

#include "parser/Parser.h"
#include <iostream>
#include <string>

// inherit visitor 
class ASTPrinter : public ASTVisitor {
    public:
    ASTPrinter();

        // Print the entire AST starting from root
    void print(ProgramAST& program);

    // Visitor methods
    // Visitor methods
    long visit(ProgramAST& node) override;
    long visit(FunctionAST& node) override;
    long visit(VariableDeclAST& node) override;
    long visit(ReturnStmtAST& node) override;
    long visit(PrintStmtAST& node) override;
    long visit(NumberExprAST& node) override;
    long visit(VariableExprAST& node) override;
    long visit(CallExprAST& node) override;
    long visit(BinaryExprAST& node) override;
    long visit(FloatExprAST& node) override;
    long visit(StringExprAST& node) override;
    long visit(CharExprAST& node) override;

private:
    int indentLevel;
    bool isLast;
    std::string prefix;

    // Helper methods
    void printIndent(bool isLast);
    void printNode(const std::string& nodeName, const std::string& value = "");
    void increaseIndent(bool isLast);
    void decreaseIndent();

};

#endif // PILLA_ASTPRINTER_H