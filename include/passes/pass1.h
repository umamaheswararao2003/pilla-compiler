#ifndef LLVM_ADDPASS_ADDCOUNTER_H
#define LLVM_ADDPASS_ADDCOUNTER_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
// #include "llvm/Passes/PassPlugin.h"


namespace llvm {

    // Define the Pass Class
    class AddCounterPass : public PassInfoMixin<AddCounterPass> {
       public : 
        // The run method declaration
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

        // Required static method for the new PM
        static bool isRequired() { return true; }
        
    };

} // namespace llvm

#endif // LLVM_ADDPASS_ADDCOUNTER_H
