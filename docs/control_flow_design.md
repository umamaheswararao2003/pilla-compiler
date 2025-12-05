# Control Flow Implementation Design

## Current Architecture Analysis

Your compiler currently has a clean, modular structure:
```
include/
├── lexer/        (Token.h, Lexer.h)
├── parser/       (AST.h, Parser.h, ASTPrinter.h)
├── sema/         (Sema.h)
├── codegen/      (Codegen.h)
└── passes/       (pass1.h, pass2.h)
```

**Current AST structure:**
- All AST nodes are in a **single file** (`AST.h`)
- Uses **visitor pattern** for traversal
- Clean separation: Expressions (`ExprAST`) vs Statements (`StmtAST`)

---

## Recommended Approach: Keep Everything in AST.h

### ✅ **OPTIMAL: Add control flow nodes to existing AST.h**

**Reasons:**
1. **Simplicity** - Your current AST is small (~230 lines), adding control flow won't make it unwieldy
2. **Cohesion** - All AST nodes belong together conceptually
3. **Visitor pattern** - Already set up perfectly for this
4. **No circular dependencies** - Everything stays in one place
5. **Industry standard** - Most compilers (Clang, GCC frontend) keep AST nodes together

**When to split:**
- Only when AST.h exceeds ~1000-1500 lines
- Or when you have 20+ different node types

---

## Alternative Approaches (Not Recommended for Your Case)

### ❌ Option 2: Separate ControlFlowAST.h

```
include/parser/
├── AST.h              (base classes, expressions, basic statements)
├── ControlFlowAST.h   (if, for, while nodes)
└── Parser.h
```

**Pros:**
- Logical separation
- Easier to find control flow nodes

**Cons:**
- ❌ Adds complexity for small benefit
- ❌ Need to include both files everywhere
- ❌ Visitor pattern split across files
- ❌ More maintenance overhead

### ❌ Option 3: Separate CFG (Control Flow Graph)

**CFG is different from AST!** CFG is an intermediate representation used for:
- Optimization passes
- Data flow analysis
- Not for parsing

**Don't confuse:**
- **AST** = Tree representation of source code structure
- **CFG** = Graph of basic blocks for analysis/optimization

You want AST nodes for if/for, not a CFG.

---

## Implementation Plan

### Step 1: Add Keywords to Lexer

**File:** `include/lexer/Token.h`

Add to `Tokentype` enum:
```cpp
enum class Tokentype {
    // ... existing tokens ...
    
    // Control flow keywords
    KW_IF, KW_ELSE, KW_FOR, KW_WHILE, KW_BREAK, KW_CONTINUE,
    
    // ... rest ...
};
```

**File:** `src/lexer/Lexer.cpp`

Add to `identifier()` method:
```cpp
if (idLexeme == "if") return makeToken(Tokentype::KW_IF, idLexeme);
else if (idLexeme == "else") return makeToken(Tokentype::KW_ELSE, idLexeme);
else if (idLexeme == "for") return makeToken(Tokentype::KW_FOR, idLexeme);
else if (idLexeme == "while") return makeToken(Tokentype::KW_WHILE, idLexeme);
// etc.
```

---

### Step 2: Add AST Nodes to AST.h

**File:** `include/parser/AST.h`

Add after existing statement nodes:

```cpp
// ===== Control Flow Statements =====

// if-else statement
class IfStmtAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> condition;
    std::vector<std::unique_ptr<StmtAST>> thenBranch;
    std::vector<std::unique_ptr<StmtAST>> elseBranch;  // optional
    
    IfStmtAST(std::unique_ptr<ExprAST> cond, 
              std::vector<std::unique_ptr<StmtAST>> thenB,
              std::vector<std::unique_ptr<StmtAST>> elseB = {})
        : condition(std::move(cond)), 
          thenBranch(std::move(thenB)), 
          elseBranch(std::move(elseB)) {}
    
    long accept(ASTVisitor& visitor) override;
};

// while loop
class WhileStmtAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> condition;
    std::vector<std::unique_ptr<StmtAST>> body;
    
    WhileStmtAST(std::unique_ptr<ExprAST> cond,
                 std::vector<std::unique_ptr<StmtAST>> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    
    long accept(ASTVisitor& visitor) override;
};

// for loop
class ForStmtAST : public StmtAST {
public:
    std::unique_ptr<StmtAST> init;        // e.g., int i = 0
    std::unique_ptr<ExprAST> condition;   // e.g., i < 10
    std::unique_ptr<ExprAST> increment;   // e.g., i + 1 (or assignment)
    std::vector<std::unique_ptr<StmtAST>> body;
    
    ForStmtAST(std::unique_ptr<StmtAST> i,
               std::unique_ptr<ExprAST> cond,
               std::unique_ptr<ExprAST> inc,
               std::vector<std::unique_ptr<StmtAST>> b)
        : init(std::move(i)), 
          condition(std::move(cond)), 
          increment(std::move(inc)), 
          body(std::move(b)) {}
    
    long accept(ASTVisitor& visitor) override;
};

// break statement
class BreakStmtAST : public StmtAST {
public:
    BreakStmtAST() = default;
    long accept(ASTVisitor& visitor) override;
};

// continue statement
class ContinueStmtAST : public StmtAST {
public:
    ContinueStmtAST() = default;
    long accept(ASTVisitor& visitor) override;
};
```

Update `ASTVisitor`:
```cpp
class ASTVisitor {
public:
    // ... existing visit methods ...
    
    // Control flow
    virtual long visit(IfStmtAST& node) = 0;
    virtual long visit(WhileStmtAST& node) = 0;
    virtual long visit(ForStmtAST& node) = 0;
    virtual long visit(BreakStmtAST& node) = 0;
    virtual long visit(ContinueStmtAST& node) = 0;
};
```

Add inline implementations:
```cpp
inline long IfStmtAST::accept(ASTVisitor& visitor) {
    return visitor.visit(*this);
}
// ... etc for other nodes
```

---

### Step 3: Add Comparison Operators

You'll need comparison operators for conditions:

**Token.h:**
```cpp
// Add to Tokentype enum
EQUAL_EQUAL,      // ==
NOT_EQUAL,        // !=
LESS_EQUAL,       // <=
GREATER_EQUAL,    // >=
// LESS_THAN and GRE_THAN already exist
```

**Lexer.cpp:**
Handle multi-character operators:
```cpp
case '=':
    if (peek() == '=') {
        advance();
        return makeToken(Tokentype::EQUAL_EQUAL, "==");
    }
    return makeToken(Tokentype::ASSIGN, "=");

case '!':
    if (peek() == '=') {
        advance();
        return makeToken(Tokentype::NOT_EQUAL, "!=");
    }
    // Handle ! operator if needed
```

---

### Step 4: Update Parser

**File:** `src/parser/Parser.cpp`

Add parsing methods:

```cpp
std::unique_ptr<StmtAST> Parser::parseStatement() {
    // ... existing checks ...
    
    if (peek().type == Tokentype::KW_IF) {
        return parseIfStatement();
    }
    if (peek().type == Tokentype::KW_WHILE) {
        return parseWhileStatement();
    }
    if (peek().type == Tokentype::KW_FOR) {
        return parseForStatement();
    }
    if (peek().type == Tokentype::KW_BREAK) {
        return parseBreakStatement();
    }
    if (peek().type == Tokentype::KW_CONTINUE) {
        return parseContinueStatement();
    }
    
    // ... existing fallback ...
}

std::unique_ptr<IfStmtAST> Parser::parseIfStatement() {
    consume(Tokentype::KW_IF, "Expected 'if'");
    consume(Tokentype::LPAR, "Expected '(' after 'if'");
    auto condition = parseExpression();
    consume(Tokentype::RPAR, "Expected ')' after condition");
    
    consume(Tokentype::LBRACE, "Expected '{' after if condition");
    std::vector<std::unique_ptr<StmtAST>> thenBranch;
    while (!match(Tokentype::RBRACE) && !isAtEnd()) {
        thenBranch.push_back(parseStatement());
    }
    
    std::vector<std::unique_ptr<StmtAST>> elseBranch;
    if (match(Tokentype::KW_ELSE)) {
        consume(Tokentype::LBRACE, "Expected '{' after 'else'");
        while (!match(Tokentype::RBRACE) && !isAtEnd()) {
            elseBranch.push_back(parseStatement());
        }
    }
    
    return std::make_unique<IfStmtAST>(
        std::move(condition), 
        std::move(thenBranch), 
        std::move(elseBranch)
    );
}
```

---

### Step 5: Update Semantic Analyzer

**File:** `src/sema/Sema.cpp`

Add visit methods:

```cpp
long Semantics::visit(IfStmtAST& node) {
    node.condition->accept(*this);
    
    // Check condition is boolean-compatible (int for now)
    if (node.condition->inferredType != Type::Int) {
        error("If condition must be an integer");
    }
    
    // Analyze then branch
    for (auto& stmt : node.thenBranch) {
        stmt->accept(*this);
    }
    
    // Analyze else branch if present
    for (auto& stmt : node.elseBranch) {
        stmt->accept(*this);
    }
    
    return 0;
}
```

---

### Step 6: Update Code Generator

**File:** `src/codegen/Codegen.cpp`

This is the most complex part. You'll use LLVM's `BasicBlock` for control flow:

```cpp
long Codegen::visit(IfStmtAST& node) {
    // Evaluate condition
    node.condition->accept(*this);
    llvm::Value* condValue = lastValue;
    
    // Convert to boolean (compare with 0)
    llvm::Value* condBool = builder->CreateICmpNE(
        condValue, 
        llvm::ConstantInt::get(*context, llvm::APInt(64, 0)),
        "ifcond"
    );
    
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
    builder->CreateBr(mergeBB);  // Jump to merge
    
    // Emit else block
    function->insert(function->end(), elseBB);
    builder->SetInsertPoint(elseBB);
    for (auto& stmt : node.elseBranch) {
        stmt->accept(*this);
    }
    builder->CreateBr(mergeBB);  // Jump to merge
    
    // Emit merge block
    function->insert(function->end(), mergeBB);
    builder->SetInsertPoint(mergeBB);
    
    return 0;
}
```

---

## File Organization Summary

**Keep it simple - modify existing files:**

```
✅ Modified files:
├── include/lexer/Token.h          (add keywords, comparison ops)
├── src/lexer/Token.cpp            (add token strings)
├── src/lexer/Lexer.cpp            (recognize keywords)
├── include/parser/AST.h           (add control flow nodes)
├── include/parser/Parser.h        (add parsing method declarations)
├── src/parser/Parser.cpp          (implement parsing)
├── src/sema/Sema.cpp              (add semantic checks)
└── src/codegen/Codegen.cpp        (generate LLVM IR)

❌ Don't create:
├── ControlFlowAST.h               (unnecessary complexity)
├── CFG.h                          (different concept)
└── Separate control flow module   (overkill for this stage)
```

---

## Recommended Implementation Order

1. **Start with `if-else`** (simplest control flow)
2. **Then `while`** (introduces loops)
3. **Then `for`** (syntactic sugar over while)
4. **Finally `break/continue`** (requires loop context tracking)

---

## When to Consider Splitting AST.h

Split only when:
- ✅ AST.h exceeds 1000-1500 lines
- ✅ You have 20+ node types
- ✅ Multiple developers working on different AST sections

**For now:** Keep everything in AST.h - it's the cleanest approach.

---

## Example Test Case

Once implemented, you can write:

```cpp
void main() {
    int x = 10;
    
    if (x > 5) {
        printf("%d", x);
    } else {
        printf("%d", 0);
    }
    
    int i = 0;
    while (i < 5) {
        printf("%d", i);
        i = i + 1;
    }
}
```

---

## Summary

**OPTIMAL APPROACH:** Add control flow nodes directly to `AST.h`

**Why:**
- ✅ Simple and maintainable
- ✅ Follows your current architecture
- ✅ Industry standard for compilers of this size
- ✅ No unnecessary complexity
- ✅ Visitor pattern already set up perfectly

**Next steps:**
1. Add keywords to lexer
2. Add AST nodes to AST.h
3. Implement parser methods
4. Add semantic checks
5. Generate LLVM IR with BasicBlocks

Would you like me to start implementing if-else statements as a proof of concept?
