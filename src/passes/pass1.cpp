#include "passes/pass1.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h" // For errs()

using namespace llvm;

// Implement the 'run' method that was declared in the header
PreservedAnalyses AddCounterPass::run(Function &F, FunctionAnalysisManager &AM) {
    
    errs() << "Analyzing function: " << F.getName() << "\n";

    unsigned addCount = 0;
    // Iterate over all basic blocks and instructions
    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            if (I.getOpcode() == Instruction::Add) {
                addCount++;
            }
        }
    }

    errs() << "  Found " << addCount << " ADD instructions.\n \n";

    // Since we only read the IR and didn't modify it, 
    // all analyses remain valid.
    return PreservedAnalyses::all();
}
