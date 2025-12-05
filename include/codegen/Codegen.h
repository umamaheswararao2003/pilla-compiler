#ifndef PILLA_CODEGEN_H
#define PILLA_CODEGEN_H

#include "passes/pass1.h"
#include "passes/pass2.h"
#include "parser/AST.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Host.h"
#include "llvm/IR/LegacyPassManager.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

class Codegen : public ASTVisitor {
public:
    Codegen();
    void generate(ProgramAST& program);
    llvm::Module* getModule() { return module.get(); }
    
    // Machine code generation methods
    void initializeTargets();
    void emitObjectCode(const std::string& filename);
    void emitAssembly(const std::string& filename);

    long visit(ProgramAST& node) override;
    long visit(FunctionAST& node) override;
    long visit(VariableDeclAST& node) override;
    long visit(ReturnStmtAST& node) override;
    long visit(PrintStmtAST& node) override;
    long visit(IfStmtAST& node) override;
    long visit(WhileStmtAST& node) override;
    long visit(ForStmtAST& node) override;
    long visit(NumberExprAST& node) override;
    long visit(VariableExprAST& node) override;
    long visit(CallExprAST& node) override;
    long visit(BinaryExprAST& node) override;
    long visit(FloatExprAST& node) override;
    long visit(StringExprAST& node) override;
    long visit(CharExprAST& node) override;

private:
    llvm::Type* getLLVMType(const std::string& typeName);
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    // New Pass Manager members
    llvm::LoopAnalysisManager lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::ModuleAnalysisManager mam;
    llvm::PassBuilder pb;

    llvm::FunctionPassManager fpm;
    llvm::ModulePassManager mpm;

    std::map<std::string, llvm::Value*> namedValues;
    llvm::Value* lastValue = nullptr;

    llvm::Value* logError(const char* str);
};

#endif //PILLA_CODEGEN_H