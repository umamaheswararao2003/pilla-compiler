#include "parser/ASTPrinter.h"
#include "lexer/Token.h"
#include <iostream>

ASTPrinter::ASTPrinter() : indentLevel(0), isLast(false), prefix("") {}

void ASTPrinter::print(ProgramAST& program) {
    std::cout << "\n=== AST Visualization ===" << std::endl;
    program.accept(*this);
    std::cout << "=========================" << std::endl;
}

void ASTPrinter::printIndent(bool /*last*/) {
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

void ASTPrinter::increaseIndent(bool /*last*/) {
    indentLevel++;
}

void ASTPrinter::decreaseIndent() {
    indentLevel--;
}

long ASTPrinter::visit(ProgramAST& node) {
    printNode("Program");
    
    for (size_t i = 0; i < node.functions.size(); ++i) {
        increaseIndent(i == node.functions.size() - 1);
        node.functions[i]->accept(*this);
        decreaseIndent();
    }
    return 0;
}

long ASTPrinter::visit(FunctionAST& node) {
    std::string params = "";
    for (size_t i = 0; i < node.parameters.size(); ++i) {
        params += node.parameters[i].first + " " + node.parameters[i].second;
        if (i < node.parameters.size() - 1) params += ", ";
    }
    printNode("Function", node.returnType + " " + node.name + "(" + params + ")");
    
    for (size_t i = 0; i < node.body.size(); ++i) {
        increaseIndent(i == node.body.size() - 1);
        node.body[i]->accept(*this);
        decreaseIndent();
    }
    return 0;
}

long ASTPrinter::visit(VariableDeclAST& node) {
    printNode("VarDecl", node.type + " " + node.name);
    if (node.initializer) {
        increaseIndent(true);
        node.initializer->accept(*this);
        decreaseIndent();
    }
    return 0;
}

long ASTPrinter::visit(ReturnStmtAST& node) {
    printNode("Return");
    increaseIndent(true);
    node.expression->accept(*this);
    decreaseIndent();
    return 0;
}

long ASTPrinter::visit(BinaryExprAST& node) {
    std::string opStr;
    switch (node.op) {
        case Tokentype::PLUS: opStr = "PLUS"; break;
        default: opStr = "UNKNOWN"; break;
    }
    printNode("BinaryOp", opStr);
    
    increaseIndent(false);
    printNode("Left");
    increaseIndent(true);
    node.left->accept(*this);
    decreaseIndent();
    decreaseIndent();
    
    increaseIndent(true);
    printNode("Right");
    increaseIndent(true);
    node.right->accept(*this);
    decreaseIndent();
    decreaseIndent();
    return 0;
}

long ASTPrinter::visit(NumberExprAST& node) {
    printNode("Number", std::to_string(node.value));
    return 0;
}

long ASTPrinter::visit(VariableExprAST& node) {
    printNode("Variable", node.name);
    return 0;
}

long ASTPrinter::visit(CallExprAST& node) {
    printNode("Call", node.callee);
    for (size_t i = 0; i < node.args.size(); ++i) {
        increaseIndent(i == node.args.size() - 1);
        node.args[i]->accept(*this);
        decreaseIndent();
    }
    return 0;
}