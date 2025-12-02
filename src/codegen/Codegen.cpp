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

llvm::Type* Codegen::getLLVMType(const std::string& typeName) {
    if (typeName == "int") return llvm::Type::getInt64Ty(*context);
    if (typeName == "float") return llvm::Type::getDoubleTy(*context); 
    if (typeName == "double") return llvm::Type::getDoubleTy(*context);
    if (typeName == "char") return llvm::Type::getInt8Ty(*context);
    if (typeName == "string") return llvm::PointerType::getUnqual(*context);
    if (typeName == "void") return llvm::Type::getVoidTy(*context);
    return llvm::Type::getInt64Ty(*context); // Default
}

long Codegen::visit(ProgramAST& node) {
    for (auto& func : node.functions) {
        func->accept(*this);
    }
    return 0;
}

long Codegen::visit(FunctionAST& node) {
    //  Define function signature
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node.parameters) {
        paramTypes.push_back(getLLVMType(param.first));
    }
    
    llvm::Type* retType = getLLVMType(node.returnType);
    llvm::FunctionType* funcType = llvm::FunctionType::get(retType, paramTypes, false);
    
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
        llvm::AllocaInst* alloca = builder->CreateAlloca(getLLVMType(node.parameters[idx].first), nullptr, arg.getName());
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
    llvm::AllocaInst* alloca = tmpBuilder.CreateAlloca(getLLVMType(node.type), nullptr, node.name);
    
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

long Codegen::visit(PrintStmtAST& node) {
    node.expression->accept(*this);
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
    // We need to know the type to load. 
    // For now, we can get the type from the alloca instruction itself.
    llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(v);
    if (alloca) {
        lastValue = builder->CreateLoad(alloca->getAllocatedType(), v, node.name.c_str());
    } else {
         // Should not happen if we only store allocas
         lastValue = v;
    }
    return 0;
}

long Codegen::visit(CallExprAST& node) {
    llvm::Function* callee = module->getFunction(node.callee);
    
    // Auto declare printf
    if (!callee && node.callee == "printf") {
        std::vector<llvm::Type*> args;
        args.push_back(llvm::PointerType::getUnqual(*context)); // format string
        llvm::FunctionType* printfType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), args, true);
        callee = llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf", module.get());
    }

    if (!callee) {
        logError("Unknown function referenced");
        lastValue = nullptr;
        return 0;
    }
    
    bool isVarArg = callee->isVarArg();
    if (isVarArg) {
        if (node.args.size() < callee->arg_size()) {
            logError("Incorrect # arguments passed to vararg function");
            lastValue = nullptr;
            return 0;
        }
    } else {
        if (callee->arg_size() != node.args.size()) {
            logError("Incorrect # arguments passed");
            lastValue = nullptr;
            return 0;
        }
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
    
    bool isFloat = L->getType()->isDoubleTy() || R->getType()->isDoubleTy();
    
    if (isFloat) {
        // Cast 
        if (!L->getType()->isDoubleTy()) L = builder->CreateSIToFP(L, llvm::Type::getDoubleTy(*context), "casttmp");
        if (!R->getType()->isDoubleTy()) R = builder->CreateSIToFP(R, llvm::Type::getDoubleTy(*context), "casttmp");
        
        switch (node.op) {
            case Tokentype::PLUS:
                lastValue = builder->CreateFAdd(L, R, "addtmp");
                break;
            default:
                logError("invalid binary operator");
                lastValue = nullptr;
                break;
        }
    } else {
        switch (node.op) {
            case Tokentype::PLUS:
                lastValue = builder->CreateAdd(L, R, "addtmp");
                break;
            default:
                logError("invalid binary operator");
                lastValue = nullptr;
                break;
        }
    }
    return 0;
}

long Codegen::visit(FloatExprAST& node) {
    lastValue = llvm::ConstantFP::get(*context, llvm::APFloat(node.value));
    return 0;
}

long Codegen::visit(StringExprAST& node) {
    lastValue = builder->CreateGlobalString(node.value);
    return 0;
}

long Codegen::visit(CharExprAST& node) {
    lastValue = llvm::ConstantInt::get(*context, llvm::APInt(8, node.value));
    return 0;
}
