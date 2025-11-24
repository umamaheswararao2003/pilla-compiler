#ifndef PILLA_CODEGEN_H
#define PILLA_CODEGEN_H

#include "parser/AST.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

class Codegen : public ASTVisitor {
public:
    Codegen();
    void generate(ProgramAST& program);
    llvm::Module* getModule() { return module.get(); }

    long visit(ProgramAST& node) override;
    long visit(FunctionAST& node) override;
    long visit(VariableDeclAST& node) override;
    long visit(ReturnStmtAST& node) override;
    long visit(NumberExprAST& node) override;
    long visit(VariableExprAST& node) override;
    long visit(CallExprAST& node) override;
    long visit(BinaryExprAST& node) override;

private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::map<std::string, llvm::Value*> namedValues;
    llvm::Value* lastValue = nullptr;

    llvm::Value* logError(const char* str);
};

#endif //PILLA_CODEGEN_H