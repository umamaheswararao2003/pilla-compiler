# Walkthrough - Codegen Implementation

I have implemented the code generation phase for the `pilla-compiler` using LLVM.

## Changes

### 1. CMake Configuration
- Updated `CMakeLists.txt` to find and link LLVM libraries.
- Enabled C language support (`project(pilla-compiler C CXX)`) to satisfy LLVM's CMake checks.
- Added `src/codegen/Codegen.cpp` to the executable target.

### 2. Codegen Class
- Defined `Codegen` class in `include/codegen/Codegen.h` inheriting from `ASTVisitor`.
- Implemented `Codegen` in `src/codegen/Codegen.cpp`.
  - Uses `llvm::IRBuilder`, `llvm::Module`, and `llvm::LLVMContext`.
  - Generates LLVM IR for:
    - Functions
    - Variable declarations (using `alloca` in entry block)
    - Return statements
    - Binary expressions (`+` only for now)
    - Function calls
    - Number literals and variable lookups

### 3. Main Integration
- Updated `src/main.cpp` to instantiate `Codegen` and run it after semantic analysis.
- Prints the generated LLVM IR to `stderr` (via `module->print`).

## Verification Results

Ran the compiler on `test/ex.cpp`:

```cpp
int sum(int a, int b) {
  return a + b;
}

int main() {
  int a =10;
  int b =2;
  int c = sum(a,b);
  return 0;
}
```

### Generated Output

```llvm
; ModuleID = 'pilla-module'
source_filename = "pilla-module"

define i64 @sum(i64 %a, i64 %b) {
entry:
  %a1 = alloca i64, align 8
  store i64 %a, i64* %a1, align 4
  %b2 = alloca i64, align 8
  store i64 %b, i64* %b2, align 4
  %a3 = load i64, i64* %a1, align 4
  %b4 = load i64, i64* %b2, align 4
  %addtmp = add i64 %a3, %b4
  ret i64 %addtmp
}

define i64 @main() {
entry:
  %c = alloca i64, align 8
  %b = alloca i64, align 8
  %a = alloca i64, align 8
  store i64 10, i64* %a, align 4
  store i64 2, i64* %b, align 4
  %a1 = load i64, i64* %a, align 4
  %b2 = load i64, i64* %b, align 4
  %calltmp = call i64 @sum(i64 %a1, i64 %b2)
  store i64 %calltmp, i64* %c, align 4
  ret i64 0
}
```

The output confirms that the AST is correctly traversed and valid LLVM IR is generated.
