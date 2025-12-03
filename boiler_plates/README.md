# Pilla Compiler - Component Overview

This directory contains boilerplate documentation for each component of the pilla-compiler.

## Component Structure

### 1. Lexer (`lexer/`)
- **Token.md**: Token structure and type definitions
- **Lexer.md**: Lexical analyzer that converts source code to tokens

### 2. Parser (`parser/`)
- **AST.md**: Abstract Syntax Tree node definitions
- **Parser.md**: Syntactic analyzer that builds AST from tokens

### 3. Semantic Analyzer (`sema/`)
- **Sema.md**: Type checking and semantic validation

### 4. Code Generator (`codegen/`)
- **Codegen.md**: LLVM IR generation and optimization

## Compilation Pipeline

```
Source Code
    ↓
Lexer → [Tokens]
    ↓
Parser → AST
    ↓
Semantic Analyzer → Annotated AST
    ↓
Code Generator → LLVM IR
    ↓
LLVM Optimization Passes
    ↓
Output
```

## How Components Connect

1. **main.cpp** reads source file and creates Lexer
2. **Lexer** scans source and produces Token stream
3. **Parser** consumes Tokens and builds AST
4. **Semantic Analyzer** validates AST and adds type information
5. **Code Generator** traverses AST and emits LLVM IR
6. **LLVM** optimizes and outputs final code

Each component uses the **Visitor Pattern** for AST traversal, allowing separation of concerns.
