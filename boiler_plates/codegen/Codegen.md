# Code Generator Component

## Purpose
The Code Generator (Codegen) traverses the AST and generates LLVM Intermediate Representation (IR). It also applies optimization passes using the LLVM Pass Manager and can emit native machine code (object files and assembly).

## Files
- `include/codegen/Codegen.h` - Code generator class declaration
- `src/codegen/Codegen.cpp` - Code generator implementation

## How It Connects to Other Components

### Input
- **Annotated AST**: AST with type information from Semantic Analyzer

### Output
- **LLVM IR**: Intermediate representation printed to stderr
- **LLVM Module**: In-memory representation for further processing
- **Object Files (.o)**: Native machine code in ELF format
- **Assembly Files (.s)**: Human-readable assembly code

### Data Flow
```
Sema → Annotated AST → Codegen.generate() → LLVM IR → Optimization → Machine Code
                                                                    ↓
                                                            Object/Assembly Files
```

## Key Concepts

1. **Visitor Pattern**: Traverses AST by implementing ASTVisitor interface
2. **IRBuilder**: LLVM utility for constructing IR instructions
3. **Value Tracking**: `lastValue` stores result of last expression
4. **Symbol Table**: `namedValues` maps variable names to LLVM values
5. **Optimization**: New Pass Manager applies optimization passes (PromotePass, InstCombinePass, ReassociatePass, GVNPass, SimplifyCFG)
6. **Type Mapping**: Converts language types to LLVM types
7. **Alloca Instructions**: Variables stored in stack-allocated memory
8. **Target Machine**: Generates native code for specific architectures
9. **Native Target**: Supports x86-64 code generation

## Machine Code Generation

### Target Initialization
- `initializeTargets()`: Initializes LLVM native target infrastructure
- Supports native platform (x86-64 on Linux)

### Object Code Emission
- `emitObjectCode(filename)`: Generates ELF object files
- Uses LLVM's legacy PassManager
- Applies target-specific optimizations
- Output can be linked with system linker (gcc/ld)

### Assembly Emission
- `emitAssembly(filename)`: Generates human-readable assembly
- Useful for debugging and learning
- Shows optimized machine instructions

## Command-Line Usage

```bash
# Generate object file (default)
./pilla-compiler input.pilla -o output.o

# Generate assembly
./pilla-compiler input.pilla -S -o output.s

# Only emit LLVM IR
./pilla-compiler input.pilla -emit-llvm

# Link to executable
gcc output.o -o executable
```

## Implementation Details

See the actual source files in `src/codegen/Codegen.cpp` and `include/codegen/Codegen.h` for the complete implementation.

For detailed information on different machine code generation approaches, see [MachineCodeGeneration.md](file:///home/blu-bridge021/Desktop/pilla-compiler/boiler_plates/codegen/MachineCodeGeneration.md).
