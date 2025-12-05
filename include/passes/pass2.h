#ifndef LLVM_TRANSFORMS_UNUSEDARGELIMPASS_H
#define LLVM_TRANSFORMS_UNUSEDARGELIMPASS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
namespace llvm {

    struct UnusedArgElimPass : public PassInfoMixin<UnusedArgElimPass> {
        
        // This run method modifies the IR, so it must be careful with PreservedAnalyses
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

        static bool isRequired() { return false; } // Not a required pass
    };

} // namespace llvm

#endif // LLVM_TRANSFORMS_UNUSEDARGELIMPASS_H
