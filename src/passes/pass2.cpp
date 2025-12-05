#include "passes/pass2.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

PreservedAnalyses UnusedArgElimPass::run(Function &F, FunctionAnalysisManager &AM) {
    bool modified = false;
    
    // We need to keep track of which arguments are actually used
    std::vector<unsigned> unusedArgIndices;
    std::vector<Type*> newParamTypes;

    // Iterate through all arguments of the function F
    for (unsigned i = 0; i < F.arg_size(); ++i) {
        Argument* Arg = F.getArg(i);
        
        // If an argument has NO users in the entire module, it's unused.
        if (Arg->use_empty()) {
            unusedArgIndices.push_back(i);
            modified = true;
            errs() << "Removing unused argument: " << Arg->getName() << " from function " << F.getName() << "\n";
        } else {
            // If it is used, we must keep its type in the new function signature
            newParamTypes.push_back(Arg->getType());
        }
    }

    // If we didn't find any unused arguments, we return early and preserve everything
    if (!modified) {
        return PreservedAnalyses::all();
    }

    // --- IR Modification Steps (The tricky part) ---
    
    // 1. Create a new FunctionType with the reduced parameters
    FunctionType* oldFT = F.getFunctionType();
    FunctionType* newFT = FunctionType::get(oldFT->getReturnType(), newParamTypes, oldFT->isVarArg());

    // 2. Create the new function with the new signature
    Function* newF = Function::Create(newFT, F.getLinkage(), F.getAddressSpace(), F.getName(), F.getParent());
    
     // While the blocks are still attached to F, move them to the end of newF
    while (!F.empty()) {
        BasicBlock *BB = &F.front();
        BB->removeFromParent();
        BB->insertInto(newF); // This is the public method name in many versions of LLVM
    }

    // 4. Update all callers of the old function to point to the new function
    F.replaceAllUsesWith(newF);
    
    // 5. Remove the old function declaration from the module
    F.eraseFromParent();
    
    return PreservedAnalyses::none(); 
}