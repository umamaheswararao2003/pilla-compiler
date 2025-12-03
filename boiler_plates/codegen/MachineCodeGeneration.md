# Target Machine Code Generation

## Overview
After generating LLVM IR (Intermediate Representation), the next step is to convert it to actual machine code that can run on target hardware. This document outlines different approaches to achieve this.

---

## Approach 1: LLVM Backend (Recommended)

### Description
Use LLVM's built-in backend infrastructure to compile IR to native machine code. This is the most common and robust approach.

### Methods

#### 1.1 Generate Object Files (.o)
Compile LLVM IR directly to object files that can be linked with a system linker.

**Advantages:**
- Industry-standard approach
- Full optimization support
- Cross-platform compilation
- Supports all LLVM target architectures

**Implementation Steps:**
1. Initialize LLVM target machine
2. Set target triple (e.g., x86_64-pc-linux-gnu)
3. Create target machine with optimization level
4. Emit object code using `TargetMachine::addPassesToEmitFile()`

**Code Structure:**
```cpp
// In Codegen.h
#include "llvm/Target/TargetMachine.h"
#include "llvm/MC/TargetRegistry.h"

class Codegen {
    // ... existing members ...
    void emitObjectCode(const std::string& filename);
    void initializeTarget();
};

// In Codegen.cpp
void Codegen::emitObjectCode(const std::string& filename) {
    // 1. Initialize targets
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    
    // 2. Get target triple
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(targetTriple);
    
    // 3. Get target
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    
    // 4. Create target machine
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto targetMachine = target->createTargetMachine(
        targetTriple, CPU, features, opt, RM);
    
    // 5. Configure module
    module->setDataLayout(targetMachine->createDataLayout());
    
    // 6. Emit object file
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    
    llvm::legacy::PassManager pass;
    auto fileType = llvm::CGFT_ObjectFile;
    
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        llvm::errs() << "TargetMachine can't emit a file of this type";
        return;
    }
    
    pass.run(*module);
    dest.flush();
}
```

**Usage:**
```bash
# Compile to object file
./pilla-compiler input.pilla -o output.o

# Link with system linker
gcc output.o -o executable
# or
ld output.o -lc -o executable
```

#### 1.2 Generate Assembly (.s)
Emit human-readable assembly code instead of binary object files.

**Advantages:**
- Debuggable and inspectable
- Educational value
- Can be hand-optimized
- Platform-specific assembly

**Implementation:**
```cpp
void Codegen::emitAssembly(const std::string& filename) {
    // Same setup as emitObjectCode...
    
    auto fileType = llvm::CGFT_AssemblyFile; // Changed from CGFT_ObjectFile
    
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        llvm::errs() << "TargetMachine can't emit assembly";
        return;
    }
    
    pass.run(*module);
    dest.flush();
}
```

**Usage:**
```bash
# Compile to assembly
./pilla-compiler input.pilla -S -o output.s

# Assemble and link
as output.s -o output.o
gcc output.o -o executable
```

#### 1.3 JIT Compilation (Just-In-Time)
Execute code immediately without creating files, useful for REPL or scripting.

**Advantages:**
- Instant execution
- No intermediate files
- Great for interactive environments
- Dynamic code generation

**Implementation:**
```cpp
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"

class Codegen {
    std::unique_ptr<llvm::orc::LLJIT> jit;
    
public:
    void initJIT();
    int executeMain();
};

void Codegen::initJIT() {
    auto jitOrErr = llvm::orc::LLJITBuilder().create();
    if (!jitOrErr) {
        llvm::errs() << "Failed to create JIT\n";
        return;
    }
    jit = std::move(*jitOrErr);
}

int Codegen::executeMain() {
    // Add module to JIT
    auto TSM = llvm::orc::ThreadSafeModule(
        std::move(module), std::move(context));
    
    auto err = jit->addIRModule(std::move(TSM));
    if (err) {
        llvm::errs() << "Failed to add module\n";
        return -1;
    }
    
    // Look up main function
    auto mainSymbol = jit->lookup("main");
    if (!mainSymbol) {
        llvm::errs() << "Could not find main function\n";
        return -1;
    }
    
    // Execute
    auto mainFn = (int (*)())mainSymbol->getAddress();
    return mainFn();
}
```

**Usage:**
```bash
# Execute directly
./pilla-compiler input.pilla --jit
```

---

## Approach 2: LLVM Tools Pipeline

### Description
Use external LLVM command-line tools to process IR through multiple stages.

### 2.1 LLC (LLVM Compiler)
Use `llc` to compile LLVM IR to assembly or object code.

**Workflow:**
```bash
# Generate LLVM IR
./pilla-compiler input.pilla -emit-llvm -o output.ll

# Compile IR to assembly
llc output.ll -o output.s

# Assemble to object file
as output.s -o output.o

# Link
gcc output.o -o executable
```

**Advantages:**
- Separation of concerns
- Easy debugging at each stage
- Can apply custom LLC optimizations
- No need to implement backend in compiler

### 2.2 Clang as Backend
Use Clang to compile LLVM IR to executables.

**Workflow:**
```bash
# Generate LLVM IR
./pilla-compiler input.pilla -emit-llvm -o output.ll

# Compile with Clang
clang output.ll -o executable
```

**Advantages:**
- Handles linking automatically
- Platform-specific optimizations
- Standard library linking
- Minimal implementation effort

---

## Approach 3: Custom Backend

### Description
Implement your own code generator from scratch, bypassing LLVM's backend.

### 3.1 Direct Assembly Generation
Generate assembly code directly from your AST or IR.

**Advantages:**
- Full control over code generation
- Educational value
- Can create custom calling conventions
- Minimal dependencies

**Disadvantages:**
- Must implement for each target architecture
- No automatic optimizations
- Complex to maintain
- Requires deep architecture knowledge

**Example Structure:**
```cpp
class X86CodeGen {
    std::ofstream asmFile;
    
public:
    void generateProlog();
    void generateEpilog();
    void generateInstruction(const std::string& instr);
    void allocateRegister();
    void emitFunction(FunctionAST& func);
};

void X86CodeGen::emitFunction(FunctionAST& func) {
    asmFile << ".globl " << func.name << "\n";
    asmFile << func.name << ":\n";
    asmFile << "    pushq %rbp\n";
    asmFile << "    movq %rsp, %rbp\n";
    // ... generate body ...
    asmFile << "    popq %rbp\n";
    asmFile << "    ret\n";
}
```

### 3.2 Bytecode Generation
Generate custom bytecode for a virtual machine.

**Advantages:**
- Platform independent
- Easier than native code generation
- Can add runtime features (GC, reflection)
- Portable across systems

**Disadvantages:**
- Requires VM implementation
- Slower than native code
- Additional runtime overhead

**Example:**
```cpp
enum class Opcode {
    PUSH, POP, ADD, SUB, CALL, RET, LOAD, STORE
};

class BytecodeGen {
    std::vector<uint8_t> bytecode;
    
public:
    void emitOpcode(Opcode op);
    void emitOperand(int32_t value);
    void writeToFile(const std::string& filename);
};
```

---

## Approach 4: Transpilation

### Description
Convert your language to another high-level language (C, C++, JavaScript) and use its compiler.

### 4.1 Transpile to C
Generate C code from your AST, then use GCC/Clang.

**Advantages:**
- Leverage mature C compilers
- Excellent optimization
- Wide platform support
- Easy interop with C libraries

**Example:**
```cpp
class CTranspiler {
    std::ofstream cFile;
    
public:
    void transpileFunction(FunctionAST& func) {
        cFile << func.returnType << " " << func.name << "(";
        // ... parameters ...
        cFile << ") {\n";
        // ... body ...
        cFile << "}\n";
    }
};
```

**Workflow:**
```bash
# Transpile to C
./pilla-compiler input.pilla --transpile-c -o output.c

# Compile C code
gcc output.c -o executable
```

### 4.2 Transpile to JavaScript/WebAssembly
For web deployment.

**Advantages:**
- Runs in browsers
- Cross-platform
- No installation needed
- Modern runtime features

---

## Approach 5: Hybrid Approaches

### 5.1 LLVM IR → C (via llc)
```bash
llc -march=c output.ll -o output.c
gcc output.c -o executable
```

### 5.2 LLVM IR → WebAssembly
```bash
llc -march=wasm32 output.ll -o output.wasm
```

### 5.3 Multi-Target Support
Support multiple backends in your compiler:
```cpp
enum class Backend {
    LLVM_OBJECT,
    LLVM_ASSEMBLY,
    LLVM_JIT,
    TRANSPILE_C,
    CUSTOM_X86
};

class Codegen {
    Backend backend;
    
public:
    void setBackend(Backend b) { backend = b; }
    void emit(const std::string& filename);
};
```

---

## Comparison Matrix

| Approach | Difficulty | Performance | Portability | Optimization | Recommended For |
|----------|-----------|-------------|-------------|--------------|-----------------|
| LLVM Object Files | Low | Excellent | High | Excellent | Production compilers |
| LLVM Assembly | Low | Excellent | High | Excellent | Debugging, learning |
| LLVM JIT | Medium | Excellent | High | Good | REPLs, scripting |
| LLC Pipeline | Very Low | Excellent | High | Excellent | Quick prototypes |
| Clang Backend | Very Low | Excellent | High | Excellent | Minimal effort |
| Custom Assembly | Very High | Good | Low | Manual | Educational projects |
| Bytecode VM | High | Fair | Excellent | Manual | Dynamic languages |
| Transpile to C | Medium | Excellent | High | Excellent | Interop with C |
| WebAssembly | Medium | Good | Excellent | Good | Web applications |

---

## Recommended Implementation Order

### Phase 1: Basic (Current State)
- ✅ LLVM IR generation
- ✅ IR optimization passes

### Phase 2: Object Code Generation
1. Implement `emitObjectCode()` method
2. Add target initialization
3. Test on native platform

### Phase 3: Assembly Output
1. Implement `emitAssembly()` method
2. Add command-line flag `-S`
3. Verify assembly correctness

### Phase 4: Executable Generation
1. Integrate system linker
2. Add standard library linking
3. Create standalone executables

### Phase 5: Advanced Features
1. Cross-compilation support
2. JIT compilation
3. Multiple backend support

---

## Next Steps for Pilla Compiler

Based on your current implementation, here's what to add:

### 1. Add Object Code Emission
```cpp
// In include/codegen/Codegen.h
void emitObjectCode(const std::string& filename);
void emitAssembly(const std::string& filename);
void initializeTargets();

// In src/codegen/Codegen.cpp
// Implement the methods shown in Approach 1.1 and 1.2
```

### 2. Update CMakeLists.txt
```cmake
# Add LLVM components for code generation
llvm_map_components_to_libnames(llvm_libs
    core
    support
    native
    passes
    target          # Add this
    mc              # Add this
    asmparser       # Add this
    asmprinter      # Add this
)
```

### 3. Update Main Driver
```cpp
// In src/main.cpp
int main(int argc, char** argv) {
    // ... existing parsing ...
    
    Codegen codegen;
    codegen.generate(program);
    
    // Add these options:
    if (emitObject) {
        codegen.emitObjectCode("output.o");
    }
    if (emitAssembly) {
        codegen.emitAssembly("output.s");
    }
    
    return 0;
}
```

### 4. Add Command-Line Options
```bash
./pilla-compiler input.pilla -o output.o        # Object file
./pilla-compiler input.pilla -S -o output.s     # Assembly
./pilla-compiler input.pilla -emit-llvm         # LLVM IR only
./pilla-compiler input.pilla -o executable      # Full compilation
```

---

## Resources

- [LLVM Target Machine Documentation](https://llvm.org/docs/WritingAnLLVMBackend.html)
- [LLVM Code Generation](https://llvm.org/docs/CodeGenerator.html)
- [LLVM ORC JIT](https://llvm.org/docs/ORCv2.html)
- [Kaleidoscope Tutorial - Chapter 8](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl08.html)
