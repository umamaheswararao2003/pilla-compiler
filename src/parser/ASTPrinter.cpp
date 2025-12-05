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

long ASTPrinter::visit(PrintStmtAST& node) {
    printNode("PrintStmt");
    increaseIndent(true);
    node.expression->accept(*this);
    decreaseIndent();
    return 0;
}

long ASTPrinter::visit(IfStmtAST& node) {
    printNode("IfStmt");
    
    // Print condition
    increaseIndent(false);
    printNode("Condition");
    increaseIndent(true);
    node.condition->accept(*this);
    decreaseIndent();
    decreaseIndent();
    
    // Print then branch
    increaseIndent(false);
    printNode("Then");
    for (size_t i = 0; i < node.thenBranch.size(); ++i) {
        increaseIndent(i == node.thenBranch.size() - 1);
        node.thenBranch[i]->accept(*this);
        decreaseIndent();
    }
    decreaseIndent();
    
    // Print else branch if present
    if (!node.elseBranch.empty()) {
        increaseIndent(true);
        printNode("Else");
        for (size_t i = 0; i < node.elseBranch.size(); ++i) {
            increaseIndent(i == node.elseBranch.size() - 1);
            node.elseBranch[i]->accept(*this);
            decreaseIndent();
        }
        decreaseIndent();
    }
    
    return 0;
}

long ASTPrinter::visit(BinaryExprAST& node) {
    std::string opStr;
    switch (node.op) {
        case Tokentype::PLUS: opStr = "PLUS"; break;
        case Tokentype::MINUS: opStr = "MINUS"; break;
        case Tokentype::MULTIPLY: opStr = "MUL"; break;
        case Tokentype::DIVIDE: opStr = "DIV"; break;
        case Tokentype::MODULO: opStr = "MOD"; break;
        case Tokentype::ASSIGN: opStr = "ASSIGN"; break;
        case Tokentype::NOT_EQUAL: opStr = "NEQ"; break;
        case Tokentype::LESS_THAN: opStr = "LT"; break;
        case Tokentype::GRE_THAN: opStr = "GT"; break;
        case Tokentype::LESS_EQUAL: opStr = "LTE"; break;
        case Tokentype::GREATER_EQUAL: opStr = "GTE"; break;
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

long ASTPrinter::visit(FloatExprAST& node) {
    printNode("Float", std::to_string(node.value));
    return 0;
}

long ASTPrinter::visit(StringExprAST& node) {
    printNode("String", node.value);
    return 0;
}

long ASTPrinter::visit(CharExprAST& node) {
    printNode("Char", std::string(1, node.value));
    return 0;
}