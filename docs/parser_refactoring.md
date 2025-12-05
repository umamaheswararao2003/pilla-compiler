# Parser Refactoring: Unified Binary Expression Handling

## Overview

Refactored the parser to use a **single unified method** for handling all binary operations instead of having separate methods for different precedence levels. This makes the code cleaner, more maintainable, and easier to extend with new operators.

## What Changed

### Before (Confusing Approach)

The parser had **two separate methods** for different precedence levels:

```cpp
// In Parser.h
std::unique_ptr<ExprAST> parseAdditiveExpression();      // For + and -
std::unique_ptr<ExprAST> parseMultiplicativeExpression(); // For *, /, %

// In Parser.cpp
std::unique_ptr<ExprAST> Parser::parseExpression() {
    return parseAdditiveExpression();  // Confusing: why start with "additive"?
}

std::unique_ptr<ExprAST> Parser::parseAdditiveExpression() {
    auto left = parseMultiplicativeExpression();  // Calls another method
    while (match(Tokentype::PLUS) || match(Tokentype::MINUS)) {
        // Handle + and -
    }
    return left;
}

std::unique_ptr<ExprAST> Parser::parseMultiplicativeExpression() {
    auto left = parsePrimary();
    while (match(Tokentype::MULTIPLY) || match(Tokentype::DIVIDE) || match(Tokentype::MODULO)) {
        // Handle *, /, %
    }
    return left;
}
```

**Problems:**
- ❌ Confusing method names and call chain
- ❌ Hard to understand which method handles which operators
- ❌ Difficult to add new precedence levels
- ❌ Code duplication in the while loops

---

### After (Clean Approach)

Now we have **one unified method** with a precedence table:

```cpp
// In Parser.h
std::unique_ptr<ExprAST> parseBinaryExpression(int minPrecedence);
int getOperatorPrecedence(Tokentype op);
bool isBinaryOperator(Tokentype type);

// In Parser.cpp
int Parser::getOperatorPrecedence(Tokentype op) {
    switch (op) {
        case Tokentype::MULTIPLY:
        case Tokentype::DIVIDE:
        case Tokentype::MODULO:
            return 2;  // Higher precedence
        case Tokentype::PLUS:
        case Tokentype::MINUS:
            return 1;  // Lower precedence
        default:
            return 0;  // Not a binary operator
    }
}

bool Parser::isBinaryOperator(Tokentype type) {
    return getOperatorPrecedence(type) > 0;
}

std::unique_ptr<ExprAST> Parser::parseExpression() {
    return parseBinaryExpression(0);  // Start with minimum precedence
}

std::unique_ptr<ExprAST> Parser::parseBinaryExpression(int minPrecedence) {
    auto left = parsePrimary();
    
    while (!isAtEnd() && isBinaryOperator(peek().type)) {
        Tokentype opType = peek().type;
        int precedence = getOperatorPrecedence(opType);
        
        if (precedence < minPrecedence) {
            break;  // Stop if precedence too low
        }
        
        Token op = peek();
        current++;
        
        // Recursively parse right side with higher precedence
        auto right = parseBinaryExpression(precedence + 1);
        
        left = std::make_unique<BinaryExprAST>(op.type, std::move(left), std::move(right));
    }
    
    return left;
}
```

**Benefits:**
- ✅ **Single source of truth** for all binary operators
- ✅ **Clear precedence table** - easy to see which operators have higher precedence
- ✅ **Easy to extend** - just add new cases to `getOperatorPrecedence()`
- ✅ **No code duplication** - one loop handles all operators
- ✅ **Self-documenting** - precedence values make it obvious (2 > 1)

---

## How It Works: Precedence Climbing Algorithm

The algorithm uses **recursion** to handle operator precedence:

### Example: `2 + 3 * 4`

1. **Start:** `parseBinaryExpression(0)` (minPrecedence = 0)
   - Parse left: `2`
   - See `+` (precedence = 1)
   - Since 1 >= 0, continue
   - Recursively parse right: `parseBinaryExpression(2)` (precedence + 1)

2. **Recursive call:** `parseBinaryExpression(2)` (minPrecedence = 2)
   - Parse left: `3`
   - See `*` (precedence = 2)
   - Since 2 >= 2, continue
   - Recursively parse right: `parseBinaryExpression(3)`

3. **Deeper recursive call:** `parseBinaryExpression(3)` (minPrecedence = 3)
   - Parse left: `4`
   - No more operators (or precedence too low)
   - Return `4`

4. **Back to step 2:**
   - Create: `3 * 4`
   - Return this expression

5. **Back to step 1:**
   - Create: `2 + (3 * 4)`
   - Return final expression

**Result:** Correctly evaluates as `2 + 12 = 14`, not `5 * 4 = 20`

---

## Adding New Operators

To add a new operator (e.g., exponentiation `**`):

1. Add token to lexer: `POWER`
2. Update `getOperatorPrecedence()`:
   ```cpp
   case Tokentype::POWER:
       return 3;  // Higher than multiply/divide
   ```
3. Add code generation in `Codegen.cpp`

That's it! No need to create new parsing methods.

---

## Verification

All tests pass with identical results:

```bash
./build/pilla-compiler test/test_precedence.pilla -o test.o
gcc test.o -o test
./test
```

**Output:** `"14""7""5""10""60""9""20"` ✓

All operator precedence rules work correctly!

---

## Files Modified

1. [Parser.h](file:///home/blu-bridge021/Desktop/pilla-compiler/include/parser/Parser.h)
   - Removed: `parseAdditiveExpression()`, `parseMultiplicativeExpression()`
   - Added: `parseBinaryExpression(int)`, `getOperatorPrecedence()`, `isBinaryOperator()`

2. [Parser.cpp](file:///home/blu-bridge021/Desktop/pilla-compiler/src/parser/Parser.cpp)
   - Replaced two separate methods with one unified `parseBinaryExpression()`
   - Added helper functions for precedence management

---

## Summary

✅ **Cleaner code** - One method instead of two
✅ **Easier to understand** - Precedence table is explicit
✅ **More maintainable** - Easy to add new operators
✅ **Same functionality** - All tests pass
✅ **Better design** - Uses standard precedence climbing algorithm

The refactored parser is now production-ready and follows best practices for expression parsing!
