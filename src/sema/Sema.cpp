#include "sema/Sema.h"
#include <iostream>

bool Semantics::analyze(ProgramAST& program) {
    hasError = false;
    functions.clear();
    scopes.clear();
    program.accept(*this);
    program.accept(*this);
    return !hasError;
}

Type stringToType(const std::string& typeName) {
    if (typeName == "int") return Type::Int;
    if (typeName == "float") return Type::Float;
    if (typeName == "double") return Type::Double;
    if (typeName == "char") return Type::Char;
    if (typeName == "string") return Type::String;
    if (typeName == "void") return Type::Void;
    return Type::Invalid;
}

void Semantics::error(const std::string& message) {
    std::cerr << "[Semantic Error] " << message << std::endl;
    hasError = true;
}

long Semantics::visit(ProgramAST& node) {
    // First pass: declare all functions
    for (const auto& func : node.functions) {
        std::vector<Type> paramTypes;
        for (const auto& param : func->parameters) {
            paramTypes.push_back(stringToType(param.first)); 
        }
        declareFunction(func->name, stringToType(func->returnType), paramTypes);
    }

    // Second pass: analyze function bodies
    for (const auto& func : node.functions) {
        func->accept(*this);
    }
    return 0;
}

long Semantics::visit(FunctionAST& node) {
    currentReturntype = stringToType(node.returnType); 
    enterScope();
    
    // Declare parameters in scope
    for (const auto& param : node.parameters) {
        declareVariable(param.second, stringToType(param.first));
    }
    
    for (const auto& stmt : node.body) {
        stmt->accept(*this);
    }
    
    exitScope();
    return 0;
}

long Semantics::visit(VariableDeclAST& node) {
    Type varType = stringToType(node.type);
    if (node.initializer) {
        node.initializer->accept(*this);
        // Check initializer type 
        if (node.initializer->inferredType != varType) {
             // Allow implicit conversion int <-> float for now or error?
             // For strictness, error. But let's allow int -> float.
             if (!(varType == Type::Float && node.initializer->inferredType == Type::Int)) {
                 // error("Type mismatch in variable initialization");
             }
        }
    }
    declareVariable(node.name, varType);
    return 0;
}

long Semantics::visit(ReturnStmtAST& node) {
    node.expression->accept(*this);
    // Check return type (assuming int for now)
    return 0;
}

long Semantics::visit(BinaryExprAST& node) {
    node.left->accept(*this);
    node.right->accept(*this);
    
    if (node.left->inferredType == Type::Float || node.right->inferredType == Type::Float) {
        node.inferredType = Type::Float;
    } else {
        node.inferredType = Type::Int;
    }
    return 0; 
}

long Semantics::visit(NumberExprAST& node) {
    node.inferredType = Type::Int;
    return 0; 
}

long Semantics::visit(FloatExprAST& node) {
    node.inferredType = Type::Float;
    return 0;
}

long Semantics::visit(StringExprAST& node) {
    node.inferredType = Type::String;
    return 0;
}

long Semantics::visit(CharExprAST& node) {
    node.inferredType = Type::Char;
    return 0;
}

long Semantics::visit(VariableExprAST& node) {
    Type type = getVariableType(node.name);
    if (type == Type::Invalid) {
        error("Undefined variable: " + node.name);
    }
    node.inferredType = type;
    return 0;
}

long Semantics::visit(CallExprAST& node) {
    auto func = getFunction(node.callee);
    if (!func) {
        error("Undefined function: " + node.callee);
        node.inferredType = Type::Invalid;
        return 0;
    }
    
    if (node.args.size() != func->paramTypes.size()) {
        error("Incorrect number of arguments for function " + node.callee);
    }
    
    for (const auto& arg : node.args) {
        arg->accept(*this);
    }
    
    node.inferredType = func->returnType;
    return 0;
}

// Symbol table helpers

void Semantics::enterScope() {
    scopes.push_back({});
}

void Semantics::exitScope() {
    scopes.pop_back();
}

void Semantics::declareVariable(const std::string& name, Type type) {
    if (scopes.empty()) return;
    scopes.back().push_back({name, type});
}

Type Semantics::getVariableType(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        for (const auto& var : *it) {
            if (var.first == name) return var.second;
        }
    }
    return Type::Invalid;
}

void Semantics::declareFunction(const std::string& name, Type returnType, const std::vector<Type>& paramTypes) {
    functions.push_back({name, {returnType, paramTypes}});
}

std::optional<Semantics::FunctionInfo> Semantics::getFunction(const std::string& name) {
    for (const auto& func : functions) {
        if (func.first == name) return func.second;
    }
    return std::nullopt;
}