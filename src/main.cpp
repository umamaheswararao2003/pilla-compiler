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
    // Check if the user provided a filename
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <source-file>\n";
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file '" << filename << "'\n";
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
    codegen.generate(*ast);

    return 0;
}