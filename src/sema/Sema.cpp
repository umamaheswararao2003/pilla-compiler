#include <sema/Sema.h>
#include<iostream>

bool Semantics::analyze(ProgramAST& program) {
    hasError = false;
    program.accept(*this);
    return !hasError;
}

void Semantics::error(const std::string& message) {
    std::cerr << "semantic error" << message << std::endl;
    hasError = true;
}

long Semantics::visit(ProgramAST& node) {
    if (node.function) node.function->accept(*this);

    return 0;
}

long Semantics::visit(FunctionAST& node) {
    currentReturntype = Type::Int;
    if (node.body) node.body->accept(*this);

    return 0;
}

long Semantics::visit(ReturnStmtAST& node) {
    if(!node.expression) {
        error(" return without value in non-void");
    }
    node.expression->accept(*this);

    if (node.expression->inferredType != currentReturntype) {
        error("return type mismatch");
    }

    return 0;
}

long Semantics::visit(BinaryExprAST& node) {
    if(node.left) node.left->accept(*this);
    if(node.right) node.right->accept(*this);

    if(node.op == Tokentype::PLUS) {
        if(node.left->inferredType != Type::Int ||
            node.right->inferredType != Type::Int) {
                error("integer operands required");
                node.inferredType = Type::Invalid;
                node.constantValue.reset();
                return 0;
        }

        node.inferredType = Type::Int;

        if (node.left->constantValue && node.right->constantValue) {
            node.constantValue = *node.left->constantValue + *node.right->constantValue;
        } else {
            node.constantValue.reset();
        }
        return 0;
    }
    error("for now only + supported");
    node.inferredType = Type::Invalid;
    node.constantValue.reset();
    return 0;
}

long Semantics::visit(NumberExprAST& node) {
    node.inferredType = Type::Int;
    node.constantValue = node.value;
    return 0;
}