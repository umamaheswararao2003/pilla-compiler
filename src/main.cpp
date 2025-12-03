#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "parser/ASTPrinter.h"
#include "sema/Sema.h"
#include "codegen/Codegen.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char *argv[])
{
    // Parse command-line arguments
    std::string inputFile;
    std::string outputFile;
    bool emitAssembly = false;
    bool emitLLVMOnly = false;
    
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source-file> [options]\n";
        std::cerr << "Options:\n";
        std::cerr << "  -o <file>     Output file (default: output.o or output.s)\n";
        std::cerr << "  -S            Emit assembly instead of object file\n";
        std::cerr << "  -emit-llvm    Only emit LLVM IR (no object/assembly)\n";
        return 1;
    }
    
    inputFile = argv[1];
    
    // Parse options
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-S") {
            emitAssembly = true;
        } else if (arg == "-emit-llvm") {
            emitLLVMOnly = true;
        } else if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        }
    }
    
    // Set default output file if not specified
    if (outputFile.empty() && !emitLLVMOnly) {
        outputFile = emitAssembly ? "output.s" : "output.o";
    }
    
    std::ifstream file(inputFile);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file '" << inputFile << "'\n";
        return 1;
    }

    // Read the entire file into a string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();

    std::cout << "--- Lexing Source Code ---" << std::endl;
    std::cout << code << std::endl;
    std::cout << "--- Generated Tokens ---" << std::endl;

    // Create the lexer and scan the code
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.scanTokens();

    // Print all the tokens
    for (const auto &token : tokens)
    {
        token.print();
    }

    Parser parser(tokens);
    std::unique_ptr<ProgramAST> ast = parser.parse();

    if (!ast)
    {
        std::cerr << "✗ Parsing failed!" << std::endl;
        return 1;
    }

    std::cout << "\n ✓ AST constructed successfully!" << std::endl;

    // ===== AST VISUALIZATION =====
    ASTPrinter printer;
    printer.print(*ast);


    Semantics sema;
    if (!sema.analyze(*ast))
    {                                                
        std::cerr << "✗ Semantic analysis failed!\n"; 
        return 1;                                     
    }
    std::cout << "✓ Semantic analysis passed!\n";

    // ===== CODE GENERATION =====
    std::cout << "\n--- Generating LLVM IR ---\n";
    Codegen codegen;
    
    // Initialize LLVM targets
    codegen.initializeTargets();
    
    // Generate LLVM IR
    codegen.generate(*ast);
    
    // Emit object code or assembly based on flags
    if (!emitLLVMOnly) {
        std::cout << "\n--- Generating Machine Code ---\n";
        if (emitAssembly) {
            codegen.emitAssembly(outputFile);
        } else {
            codegen.emitObjectCode(outputFile);
        }
    }

    return 0;
}