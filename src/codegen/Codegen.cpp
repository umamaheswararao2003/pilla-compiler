#include "codegen/Codegen.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include <iostream>

Codegen::Codegen() {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("pilla-module", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);

    // Register analysis managers
    pb.registerModuleAnalyses(mam);
    pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam);
    pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);

    // Add passes
    mpm.addPass(createModuleToFunctionPassAdaptor(llvm::UnusedArgElimPass()));
    mpm.addPass(createModuleToFunctionPassAdaptor(llvm::AddCounterPass()));
    // Promote allocas to registers
    fpm.addPass(llvm::PromotePass());
    // Peephole optimizations
    fpm.addPass(llvm::InstCombinePass());
    // Reassociate expressions
    fpm.addPass(llvm::ReassociatePass());
    // CSE
    fpm.addPass(llvm::GVNPass());
    // Simplify control flow
    fpm.addPass(llvm::SimplifyCFGPass());

    // pilla passes
    

}

void Codegen::generate(ProgramAST& program) {
    program.accept(*this);
    module->print(llvm::errs(), nullptr);
}

llvm::Value* Codegen::logError(const char* str) {
    std::cerr << "Codegen Error: " << str << std::endl;
    return nullptr;
}

void Codegen::initializeTargets() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();
}

void Codegen::emitObjectCode(const std::string& filename) {
    // Get target triple
    llvm::Triple targetTriple(llvm::sys::getDefaultTargetTriple());
    module->setTargetTriple(targetTriple);
    
    // Get target
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple.str(), error);
    
    if (!target) {
        llvm::errs() << "Error: " << error << "\n";
        return;
    }
    
    // Create target machine
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    auto RM = llvm::Reloc::Model::PIC_;
    auto targetMachine = target->createTargetMachine(
        targetTriple, CPU, features, opt, RM);
    
    // Configure module
    module->setDataLayout(targetMachine->createDataLayout());
    
    // Emit object file
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message() << "\n";
        return;
    }
    
    llvm::legacy::PassManager pass;
    auto fileType = llvm::CodeGenFileType::ObjectFile;
    
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        llvm::errs() << "TargetMachine can't emit a file of this type\n";
        return;
    }
    
    pass.run(*module);
    dest.flush();
    
    std::cout << "Object file written to: " << filename << "\n";
}

void Codegen::emitAssembly(const std::string& filename) {
    // Get target triple
    llvm::Triple targetTriple(llvm::sys::getDefaultTargetTriple());
    module->setTargetTriple(targetTriple);
    
    // Get target
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple.str(), error);
    
    if (!target) {
        llvm::errs() << "Error: " << error << "\n";
        return;
    }
    
    // Create target machine
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    auto RM = llvm::Reloc::Model::PIC_;
    auto targetMachine = target->createTargetMachine(
        targetTriple, CPU, features, opt, RM);
    
    // Configure module
    module->setDataLayout(targetMachine->createDataLayout());
    
    // Emit assembly file
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message() << "\n";
        return;
    }
    
    llvm::legacy::PassManager pass;
    auto fileType = llvm::CodeGenFileType::AssemblyFile;
    
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        llvm::errs() << "TargetMachine can't emit assembly\n";
        return;
    }
    
    pass.run(*module);
    dest.flush();
    
    std::cout << "Assembly file written to: " << filename << "\n";
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

    mpm.run(*module, mam);
    
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

    if (retType->isVoidTy() && !builder->GetInsertBlock()->getTerminator()) {
        builder->CreateRetVoid(); 
    }
    
    // 5. Verify function
    llvm::verifyFunction(*function);

    // 6. Optimize function
    fpm.run(*function, fam);
    
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

long Codegen::visit(IfStmtAST& node) {
    // Evaluate condition
    node.condition->accept(*this);
    llvm::Value* condValue = lastValue;
    
    // Convert to boolean (compare with 0)
    llvm::Value* condBool;
    if (condValue->getType()->isDoubleTy()) {
        // For float: compare != 0.0
        condBool = builder->CreateFCmpONE(
            condValue,
            llvm::ConstantFP::get(*context, llvm::APFloat(0.0)),
            "ifcond"
        );
    } else {
        // For int: compare != 0
        condBool = builder->CreateICmpNE(
            condValue,
            llvm::ConstantInt::get(*context, llvm::APInt(64, 0)),
            "ifcond"
        );
    }
    
    llvm::Function* function = builder->GetInsertBlock()->getParent();
    
    // Create basic blocks
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*context, "then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*context, "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context, "ifcont");
    
    // Branch based on condition
    builder->CreateCondBr(condBool, thenBB, elseBB);
    
    // Emit then block
    builder->SetInsertPoint(thenBB);
    for (auto& stmt : node.thenBranch) {
        stmt->accept(*this);
    }
    // Only add branch if block doesn't already have terminator
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(mergeBB);
    }
    
    // Emit else block
    function->insert(function->end(), elseBB);
    builder->SetInsertPoint(elseBB);
    for (auto& stmt : node.elseBranch) {
        stmt->accept(*this);
    }
    // Only add branch if block doesn't already have terminator
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateBr(mergeBB);
    }
    
    // Emit merge block
    function->insert(function->end(), mergeBB);
    builder->SetInsertPoint(mergeBB);
    
    return 0;
}

long Codegen::visit(WhileStmtAST& node) {
    llvm::Function* function = builder->GetInsertBlock()->getParent();
    
    // Create basic blocks
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(*context, "while.cond", function);
    llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(*context, "while.body", function);
    llvm::BasicBlock* endBB = llvm::BasicBlock::Create(*context, "while.end", function);
    
    builder->CreateBr(condBB);

    // Emit condition block
    builder->SetInsertPoint(condBB);
    node.condition->accept(*this);
    llvm::Value* condValue = lastValue;
    llvm::Value* condBool = nullptr;

    if (condValue->getType()->isFloatingPointTy()) {
        condBool = builder->CreateFCmpONE(
            condValue,
            llvm::ConstantFP::get(*context, llvm::APFloat(0.0)),
            "whilecond"
        );
    } else {
        condBool = builder->CreateICmpNE(
            condValue,
            llvm::ConstantInt::get(condValue->getType(), 0),
            "whilecond"
        );
    }

    builder->CreateCondBr(condBool, bodyBB, endBB);
     
    // Emit body block
    builder->SetInsertPoint(bodyBB);
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    
    // after body again to condition 
    builder->CreateBr(condBB);

    //emit end block
    builder->SetInsertPoint(endBB);
    
    return 0;
}    

long Codegen::visit(ForStmtAST& node) {

    llvm::Function* function = builder->GetInsertBlock()->getParent();

    if(node.initializer) {
        node.initializer->accept(*this);
    }

    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(*context, "for.cond", function);
    llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(*context, "for.body", function);
    llvm::BasicBlock* incBB = llvm::BasicBlock::Create(*context, "for.inc", function);
    llvm::BasicBlock* endBB = llvm::BasicBlock::Create(*context, "for.end", function);

    builder->CreateBr(condBB);

    // Emit condition block
    builder->SetInsertPoint(condBB);
    
    llvm::Value* condBool = nullptr;
    if(node.condition) {
        node.condition->accept(*this);
        llvm::Value* condValue = lastValue;

        if (condValue->getType()->isFloatingPointTy()) {
            condBool = builder->CreateFCmpONE(
                condValue,
                llvm::ConstantFP::get(*context, llvm::APFloat(0.0)),
                "forcond"
            );
        } else {
            condBool = builder->CreateICmpNE(
                condValue,
                llvm::ConstantInt::get(condValue->getType(), 0),
                "forcond"
            );
        }
    } else {
        condBool = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), 1);
    }

    // conditional branch 
    builder->CreateCondBr(condBool, bodyBB, endBB);

    // Emit body block
    builder->SetInsertPoint(bodyBB);
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }

    // Emit increment block
    builder->CreateBr(incBB);
    builder->SetInsertPoint(incBB);

    if(node.increment) {
        node.increment->accept(*this);
    }

    // after body again to condition
    builder->CreateBr(condBB);

    //emit end block
    builder->SetInsertPoint(endBB);

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
    
    // Special handling for printf
    if (node.callee == "printf") {
        if (!callee) {
            // Auto declare printf
            std::vector<llvm::Type*> args;
            args.push_back(llvm::PointerType::getUnqual(*context)); // format string
            llvm::FunctionType* printfType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), args, true);
            callee = llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf", module.get());
        }
        
        // Generate format string and arguments
        std::vector<llvm::Value*> argsV;
        std::string formatStr = "";
        
        for (unsigned i = 0, e = node.args.size(); i != e; ++i) {
            node.args[i]->accept(*this);
            if (!lastValue) return 0;
            
            // Determine format specifier based on type
            llvm::Type* argType = lastValue->getType();
            if (argType->isIntegerTy(64)) {
                formatStr += "%ld";
            } else if (argType->isIntegerTy(32)) {
                formatStr += "%d";
            } else if (argType->isIntegerTy(8)) {
                formatStr += "%c";
            } else if (argType->isDoubleTy()) {
                formatStr += "%f";
            } else if (argType->isPointerTy()) {
                formatStr += "%s";
            } else {
                formatStr += "%d";  // Default
            }
            
            argsV.push_back(lastValue);
            
            // Add space between values (except for last one)
            if (i < e - 1) {
                formatStr += " ";
            }
        }
        
        // Add newline at the end
        formatStr += "\\n";
        
        // Create global string constant for format
        llvm::Value* formatStrVal = builder->CreateGlobalStringPtr(formatStr);
        
        // Insert format string as first argument
        argsV.insert(argsV.begin(), formatStrVal);
        
        lastValue = builder->CreateCall(callee, argsV, "calltmp");
        return 0;
    }

    // Regular function call handling
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
    // Handle assignment separately
    if (node.op == Tokentype::ASSIGN) {
        // For assignment, left must be a variable
        VariableExprAST* varExpr = dynamic_cast<VariableExprAST*>(node.left.get());
        if (!varExpr) {
            logError("Left side of assignment must be a variable");
            lastValue = nullptr;
            return 0;
        }
        
        // Evaluate right side
        node.right->accept(*this);
        llvm::Value* val = lastValue;
        
        // Get the variable's alloca
        llvm::Value* variable = namedValues[varExpr->name];
        if (!variable) {
            logError("Unknown variable name in assignment");
            lastValue = nullptr;
            return 0;
        }
        
        // Store the value
        builder->CreateStore(val, variable);
        
        // Assignment expression returns the assigned value
        lastValue = val;
        return 0;
    }
    
    // For other operators, evaluate both sides
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
        // Cast to double if needed
        if (!L->getType()->isDoubleTy()) L = builder->CreateSIToFP(L, llvm::Type::getDoubleTy(*context), "casttmp");
        if (!R->getType()->isDoubleTy()) R = builder->CreateSIToFP(R, llvm::Type::getDoubleTy(*context), "casttmp");
        
        switch (node.op) {
            case Tokentype::PLUS:
                lastValue = builder->CreateFAdd(L, R, "addtmp");
                break;
            case Tokentype::MINUS:
                lastValue = builder->CreateFSub(L, R, "subtmp");
                break;
            case Tokentype::MULTIPLY:
                lastValue = builder->CreateFMul(L, R, "multmp");
                break;
            case Tokentype::DIVIDE:
                lastValue = builder->CreateFDiv(L, R, "divtmp");
                break;
            case Tokentype::MODULO:
                lastValue = builder->CreateFRem(L, R, "modtmp");
                break;
            case Tokentype::LESS_THAN:
                lastValue = builder->CreateFCmpULT(L, R, "cmptmp");
                // Convert bool to int (0 or 1)
                lastValue = builder->CreateUIToFP(lastValue, llvm::Type::getDoubleTy(*context), "booltmp");
                break;
            case Tokentype::GRE_THAN:
                lastValue = builder->CreateFCmpUGT(L, R, "cmptmp");
                lastValue = builder->CreateUIToFP(lastValue, llvm::Type::getDoubleTy(*context), "booltmp");
                break;
            case Tokentype::LESS_EQUAL:
                lastValue = builder->CreateFCmpULE(L, R, "cmptmp");
                lastValue = builder->CreateUIToFP(lastValue, llvm::Type::getDoubleTy(*context), "booltmp");
                break;
            case Tokentype::GREATER_EQUAL:
                lastValue = builder->CreateFCmpUGE(L, R, "cmptmp");
                lastValue = builder->CreateUIToFP(lastValue, llvm::Type::getDoubleTy(*context), "booltmp");
                break;
            case Tokentype::EQUAL_EQUAL:
                lastValue = builder->CreateFCmpUEQ(L, R, "cmptmp");
                lastValue = builder->CreateUIToFP(lastValue, llvm::Type::getDoubleTy(*context), "booltmp");
                break;
            case Tokentype::NOT_EQUAL:
                lastValue = builder->CreateFCmpUNE(L, R, "cmptmp");
                lastValue = builder->CreateUIToFP(lastValue, llvm::Type::getDoubleTy(*context), "booltmp");
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
            case Tokentype::MINUS:
                lastValue = builder->CreateSub(L, R, "subtmp");
                break;
            case Tokentype::MULTIPLY:
                lastValue = builder->CreateMul(L, R, "multmp");
                break;
            case Tokentype::DIVIDE:
                lastValue = builder->CreateSDiv(L, R, "divtmp");
                break;
            case Tokentype::MODULO:
                lastValue = builder->CreateSRem(L, R, "modtmp");
                break;
            case Tokentype::LESS_THAN:
                lastValue = builder->CreateICmpSLT(L, R, "cmptmp");
                // Convert bool (i1) to int (i64)
                lastValue = builder->CreateZExt(lastValue, llvm::Type::getInt64Ty(*context), "booltmp");
                break;
            case Tokentype::GRE_THAN:
                lastValue = builder->CreateICmpSGT(L, R, "cmptmp");
                lastValue = builder->CreateZExt(lastValue, llvm::Type::getInt64Ty(*context), "booltmp");
                break;
            case Tokentype::LESS_EQUAL:
                lastValue = builder->CreateICmpSLE(L, R, "cmptmp");
                lastValue = builder->CreateZExt(lastValue, llvm::Type::getInt64Ty(*context), "booltmp");
                break;
            case Tokentype::GREATER_EQUAL:
                lastValue = builder->CreateICmpSGE(L, R, "cmptmp");
                lastValue = builder->CreateZExt(lastValue, llvm::Type::getInt64Ty(*context), "booltmp");
                break;
            case Tokentype::EQUAL_EQUAL:
                lastValue = builder->CreateICmpEQ(L, R, "cmptmp");
                lastValue = builder->CreateZExt(lastValue, llvm::Type::getInt64Ty(*context), "booltmp");
                break;
            case Tokentype::NOT_EQUAL:
                lastValue = builder->CreateICmpNE(L, R, "cmptmp");
                lastValue = builder->CreateZExt(lastValue, llvm::Type::getInt64Ty(*context), "booltmp");
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
