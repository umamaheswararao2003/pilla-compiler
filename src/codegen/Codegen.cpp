#include "codegen/Codegen.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Pass.h"
#include <iostream>

Codegen::Codegen() {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("pilla-module", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);

    fpm = std::make_unique<llvm::legacy::FunctionPassManager>(module.get());

    // fron alloca to registers
    fpm->add(llvm::createPromoteMemoryToRegisterPass());
    // peep hole optimisations 
    fpm->add(llvm::createInstructionCombiningPass());
    // Reassociate expressions.
    fpm->add(llvm::createReassociatePass());
    // CSE
    fpm->add(llvm::createGVNPass());
    // used to simplyy the  control flow
    fpm->add(llvm::createCFGSimplificationPass());

    fpm->doInitialization();
}

void Codegen::generate(ProgramAST& program) {
    program.accept(*this);
    module->print(llvm::errs(), nullptr);
}

llvm::Value* Codegen::logError(const char* str) {
    std::cerr << "Codegen Error: " << str << std::endl;
    return nullptr;
}

long Codegen::visit(ProgramAST& node) {
    for (auto& func : node.functions) {
        func->accept(*this);
    }
    return 0;
}

long Codegen::visit(FunctionAST& node) {
    //  Define function signature
    std::vector<llvm::Type*> paramTypes(node.parameters.size(), llvm::Type::getInt64Ty(*context));
    llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getInt64Ty(*context), paramTypes, false);
    
    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node.name, module.get());
    
    //Create entry block
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(entry);
    
    // Record arguments in namedValues
    namedValues.clear();
    unsigned idx = 0;
    for (auto& arg : function->args()) {
        arg.setName(node.parameters[idx].second);
        
        // Create alloca for argument 
        llvm::AllocaInst* alloca = builder->CreateAlloca(llvm::Type::getInt64Ty(*context), nullptr, arg.getName());
        builder->CreateStore(&arg, alloca);
        
        namedValues[std::string(arg.getName())] = alloca;
        idx++;
    }
    
    // 4. Generate body
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    
    // 5. Verify function
    llvm::verifyFunction(*function);

    // 6. Optimize function
    fpm->run(*function);
    
    return 0;
}

long Codegen::visit(VariableDeclAST& node) {
    llvm::Function* function = builder->GetInsertBlock()->getParent();
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    llvm::AllocaInst* alloca = tmpBuilder.CreateAlloca(llvm::Type::getInt64Ty(*context), nullptr, node.name);
    
    if (node.initializer) {
        node.initializer->accept(*this);
        if (lastValue) {
            builder->CreateStore(lastValue, alloca);
        }
    }
    
    namedValues[node.name] = alloca;
    return 0;
}

long Codegen::visit(ReturnStmtAST& node) {
    if (node.expression) {
        node.expression->accept(*this);
        if (lastValue) {
            builder->CreateRet(lastValue);
        } else {
            // Error handling?
             builder->CreateRet(llvm::ConstantInt::get(*context, llvm::APInt(64, 0)));
        }
    } else {
        // Or return 0 if int function
        builder->CreateRetVoid(); 
    }
    return 0;
}

long Codegen::visit(NumberExprAST& node) {
    lastValue = llvm::ConstantInt::get(*context, llvm::APInt(64, node.value));
    return 0;
}

long Codegen::visit(VariableExprAST& node) {
    llvm::Value* v = namedValues[node.name];
    if (!v) {
        logError("Unknown variable name");
        lastValue = nullptr;
        return 0;
    }
    // Load the value from the alloca
    lastValue = builder->CreateLoad(llvm::Type::getInt64Ty(*context), v, node.name.c_str());
    return 0;
}

long Codegen::visit(CallExprAST& node) {
    llvm::Function* callee = module->getFunction(node.callee);
    if (!callee) {
        logError("Unknown function referenced");
        lastValue = nullptr;
        return 0;
    }
    
    if (callee->arg_size() != node.args.size()) {
        logError("Incorrect # arguments passed");
        lastValue = nullptr;
        return 0;
    }
    
    std::vector<llvm::Value*> argsV;
    for (unsigned i = 0, e = node.args.size(); i != e; ++i) {
        node.args[i]->accept(*this);
        if (!lastValue) return 0;
        argsV.push_back(lastValue);
    }
    
    lastValue = builder->CreateCall(callee, argsV, "calltmp");
    return 0;
}

long Codegen::visit(BinaryExprAST& node) {
    node.left->accept(*this);
    llvm::Value* L = lastValue;
    node.right->accept(*this);
    llvm::Value* R = lastValue;
    
    if (!L || !R) {
        lastValue = nullptr;
        return 0;
    }
    
    switch (node.op) {
        case Tokentype::PLUS:
            lastValue = builder->CreateAdd(L, R, "addtmp");
            break;
        default:
            logError("invalid binary operator");
            lastValue = nullptr;
            break;
    }
    return 0;
}
