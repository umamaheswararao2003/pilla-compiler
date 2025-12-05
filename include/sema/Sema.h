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
    // visitor methods
    long visit(ProgramAST& node) override;
    long visit(FunctionAST& node) override;
    long visit(VariableDeclAST& node) override;
    long visit(ReturnStmtAST& node) override;
    long visit(PrintStmtAST& node) override;
    long visit(IfStmtAST& node) override;
    long visit(WhileStmtAST& node) override;
    long visit(ForStmtAST& node) override;
    long visit(BinaryExprAST& node) override;
    long visit(NumberExprAST& node) override;
    long visit(VariableExprAST& node) override;
    long visit(CallExprAST& node) override;
    long visit(FloatExprAST& node) override;
    long visit(StringExprAST& node) override;
    long visit(CharExprAST& node) override;

    private:
    // helper
    void error(const std::string& message);
    bool hasError = false;

    Type currentReturntype = Type::Invalid;
    
    // Simple symbol table: map variable name to type
    // Using a vector of maps to handle scopes (though currently we only have function scope)
    std::vector<std::vector<std::pair<std::string, Type>>> scopes;
    
    void enterScope();
    void exitScope();
    void declareVariable(const std::string& name, Type type);
    Type getVariableType(const std::string& name);
    
    // Function table
    struct FunctionInfo {
        Type returnType;
        std::vector<Type> paramTypes;
    };
    std::vector<std::pair<std::string, FunctionInfo>> functions;
    void declareFunction(const std::string& name, Type returnType, const std::vector<Type>& paramTypes);
    std::optional<FunctionInfo> getFunction(const std::string& name);
};

#endif //PILLA_SEMANTICS_H