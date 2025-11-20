#include "parser/ASTPrinter.h"
#include "lexer/Token.h"
#include <iostream>

ASTPrinter::ASTPrinter() : indentLevel(0), isLast(false), prefix("") {}

void ASTPrinter::print(ProgramAST& program) {
    std::cout << "\n=== AST Visualization ===" << std::endl;
    program.accept(*this);
    std::cout << "=========================" << std::endl;
}

void ASTPrinter::printIndent(bool last) {
    for (int i = 0; i < indentLevel; i++) {
        std::cout << "  ";
    }
}

void ASTPrinter::printNode(const std::string& nodeName, const std::string& value) {
    printIndent(false);
    if (value.empty()) {
        std::cout << nodeName << std::endl;
    } else {
        std::cout << nodeName << ": " << value << std::endl;
    }
}

void ASTPrinter::increaseIndent(bool last) {
    indentLevel++;
}

void ASTPrinter::decreaseIndent() {
    indentLevel--;
}

long ASTPrinter::visit(ProgramAST& node) {
    printNode("Program");
    
    increaseIndent(true);
    node.function->accept(*this);
    decreaseIndent();
    
    return 0;
}

long ASTPrinter::visit(FunctionAST& node) {
    printNode("Function", node.name + "()");
    
    increaseIndent(true);
    node.body->accept(*this);
    decreaseIndent();
    
    return 0;
}

long ASTPrinter::visit(ReturnStmtAST& node) {
    printNode("Return");
    
    increaseIndent(true);
    node.expression->accept(*this);
    decreaseIndent();
    
    return 0;
}

long ASTPrinter::visit(NumberExprAST& node) {
    printNode("Number", std::to_string(node.value));
    return node.value;
}

long ASTPrinter::visit(BinaryExprAST& node) {
    printNode("BinaryOp", tokenTypeToString(node.op));
    
    increaseIndent(false);
    printIndent(false);
    std::cout << "├─ Left:" << std::endl;
    increaseIndent(false);
    node.left->accept(*this);
    decreaseIndent();
    
    printIndent(false);
    std::cout << "└─ Right:" << std::endl;
    increaseIndent(true);
    node.right->accept(*this);
    decreaseIndent();
    decreaseIndent();
    
    return 0;
}