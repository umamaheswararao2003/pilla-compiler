# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(pilla-compiler C CXX)

# Set the C++ standard (e.g., C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Add the executable target.
# It's built from all the .cpp files in the src directory.
add_executable(pilla-compiler
    src/main.cpp
    src/lexer/Lexer.cpp
    src/lexer/Token.cpp
    src/parser/Parser.cpp
    src/sema/Sema.cpp
    src/parser/ASTPrinter.cpp
    src/codegen/Codegen.cpp
)

# Tell CMake to look for header files in the 'include' directory
target_include_directories(pilla-compiler SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Optional: Enable warnings
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    target_compile_options(pilla-compiler PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Link LLVM libraries
llvm_map_components_to_libnames(llvm_libs 
    core 
    support 
    irreader 
    native 
    passes
    target
    mc
    asmparser
    asmprinter
)
target_link_libraries(pilla-compiler PRIVATE ${llvm_libs})