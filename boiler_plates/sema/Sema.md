# Semantic Analyzer Component

## Purpose
The Semantic Analyzer (Sema) performs semantic analysis on the AST. It checks type correctness, variable/function declarations, scope rules, and annotates the AST with type information.

## Files
- `include/sema/Sema.h` - Semantic analyzer class declaration
- `src/sema/Sema.cpp` - Semantic analyzer implementation

## Boilerplate Code

### Sema.h
```cpp
#ifndef PILLA_SEMA_H
#define PILLA_SEMA_H

#include "parser/AST.h"
#include <string>
#include <vector>
#include <map>
#include <optional>

class Semantics : public ASTVisitor {
public:
    // Main entry point
    bool analyze(ProgramAST& program);
    
    // Visitor methods (implement ASTVisitor interface)
    long visit(ProgramAST& node) override;
    long visit(FunctionAST& node) override;
    long visit(VariableDeclAST& node) override;
    long visit(ReturnStmtAST& node) override;
    long visit(PrintStmtAST& node) override;
    long visit(NumberExprAST& node) override;
    long visit(FloatExprAST& node) override;
    long visit(StringExprAST& node) override;
    long visit(CharExprAST& node) override;
    long visit(VariableExprAST& node) override;
    long visit(CallExprAST& node) override;
    long visit(BinaryExprAST& node) override;

private:
    bool hasError = false;
    Type currentReturntype = Type::Invalid;
    
    // Symbol table: scopes stack
    // Each scope is a list of (name, type) pairs
    std::vector<std::vector<std::pair<std::string, Type>>> scopes;
    
    // Function table
    struct FunctionInfo {
        Type returnType;
        std::vector<Type> paramTypes;
    };
    std::vector<std::pair<std::string, FunctionInfo>> functions;
    
    // Scope management
    void enterScope();
    void exitScope();
    void declareVariable(const std::string& name, Type type);
    Type getVariableType(const std::string& name);
    
    // Function management
    void declareFunction(const std::string& name, Type returnType, 
                        const std::vector<Type>& paramTypes);
    std::optional<FunctionInfo> getFunction(const std::string& name);
    
    // Error reporting
    void error(const std::string& message);
};

// Helper function
Type stringToType(const std::string& typeName);

#endif // PILLA_SEMA_H
```

### Sema.cpp (Key Methods)
```cpp
#include "sema/Sema.h"
#include <iostream>

// Main analysis entry point
bool Semantics::analyze(ProgramAST& program) {
    hasError = false;
    functions.clear();
    scopes.clear();
    program.accept(*this);
    return !hasError;
}

// Convert string type to Type enum
Type stringToType(const std::string& typeName) {
    if (typeName == "int") return Type::Int;
    if (typeName == "float") return Type::Float;
    if (typeName == "double") return Type::Double;
    if (typeName == "char") return Type::Char;
    if (typeName == "string") return Type::String;
    if (typeName == "void") return Type::Void;
    return Type::Invalid;
}

// Error reporting
void Semantics::error(const std::string& message) {
    std::cerr << "[Semantic Error] " << message << std::endl;
    hasError = true;
}

// Visit program node
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

// Visit function node
long Semantics::visit(FunctionAST& node) {
    currentReturntype = stringToType(node.returnType);
    enterScope();
    
    // Declare parameters in scope
    for (const auto& param : node.parameters) {
        declareVariable(param.second, stringToType(param.first));
    }
    
    // Analyze function body
    for (const auto& stmt : node.body) {
        stmt->accept(*this);
    }
    
    exitScope();
    return 0;
}

// Visit variable declaration
long Semantics::visit(VariableDeclAST& node) {
    Type varType = stringToType(node.type);
    if (node.initializer) {
        node.initializer->accept(*this);
        // Check type compatibility
        if (node.initializer->inferredType != varType) {
            // Allow implicit int -> float conversion
            if (!(varType == Type::Float && node.initializer->inferredType == Type::Int)) {
                error("Type mismatch in variable initialization");
            }
        }
    }
    declareVariable(node.name, varType);
    return 0;
}

// Visit return statement
long Semantics::visit(ReturnStmtAST& node) {
    node.expression->accept(*this);
    // Could check if return type matches function return type
    return 0;
}

// Visit print statement
long Semantics::visit(PrintStmtAST& node) {
    node.expression->accept(*this);
    return 0;
}

// Visit binary expression
long Semantics::visit(BinaryExprAST& node) {
    node.left->accept(*this);
    node.right->accept(*this);
    
    // Type inference: if either operand is float, result is float
    if (node.left->inferredType == Type::Float || node.right->inferredType == Type::Float) {
        node.inferredType = Type::Float;
    } else {
        node.inferredType = Type::Int;
    }
    return 0;
}

// Visit number literal
long Semantics::visit(NumberExprAST& node) {
    node.inferredType = Type::Int;
    return 0;
}

// Visit float literal
long Semantics::visit(FloatExprAST& node) {
    node.inferredType = Type::Float;
    return 0;
}

// Visit string literal
long Semantics::visit(StringExprAST& node) {
    node.inferredType = Type::String;
    return 0;
}

// Visit char literal
long Semantics::visit(CharExprAST& node) {
    node.inferredType = Type::Char;
    return 0;
}

// Visit variable reference
long Semantics::visit(VariableExprAST& node) {
    Type type = getVariableType(node.name);
    if (type == Type::Invalid) {
        error("Undefined variable: " + node.name);
    }
    node.inferredType = type;
    return 0;
}

// Visit function call
long Semantics::visit(CallExprAST& node) {
    // Special case: allow external functions like printf
    if (node.callee == "printf") {
        for (const auto& arg : node.args) {
            arg->accept(*this);
        }
        node.inferredType = Type::Int;
        return 0;
    }
    
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

// Scope management
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
    // Search from innermost to outermost scope
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        for (const auto& var : *it) {
            if (var.first == name) return var.second;
        }
    }
    return Type::Invalid;
}

// Function management
void Semantics::declareFunction(const std::string& name, Type returnType, 
                               const std::vector<Type>& paramTypes) {
    functions.push_back({name, {returnType, paramTypes}});
}

std::optional<Semantics::FunctionInfo> Semantics::getFunction(const std::string& name) {
    for (const auto& func : functions) {
        if (func.first == name) return func.second;
    }
    return std::nullopt;
}
```

## How It Connects to Other Components

### Input
- **AST**: Abstract Syntax Tree from Parser

### Output
- **Annotated AST**: AST with type information in `inferredType` fields
- **Error Reports**: Semantic errors printed to stderr

### Data Flow
```
Parser → AST → Sema.analyze() → Annotated AST → Code Generator
```

## Key Concepts

1. **Visitor Pattern**: Traverses AST by implementing ASTVisitor interface
2. **Symbol Table**: Tracks variables and their types in nested scopes
3. **Type Inference**: Determines expression types and stores in AST nodes
4. **Scope Management**: Handles nested scopes (function parameters, local variables)
5. **Error Reporting**: Collects and reports semantic errors without crashing
6. **Two-Pass Analysis**: First pass declares functions, second pass checks bodies
