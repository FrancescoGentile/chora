cmake_minimum_required(VERSION 3.18)
project(chc C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(chc
        src/main.c
        src/file/file.h src/file/file.c
        src/file/buffer.h src/file/buffer.c
        src/lexer/token.h src/lexer/token.c
        src/std/types.h
        src/file/coord.h
        src/std/vector/template.h
        src/std/vector/u64.h
        src/lexer/lexer.h src/lexer/lexer.c
        src/error/handler.h src/error/handler.c
        src/std/vector/coord.h
        src/error/style.h
        src/std/string.h src/std/string.c
        src/ast/ast.h src/ast/ast.c
        src/parser/parser.c src/parser/parser.h
        src/std/map/map.h src/std/map/multimap.h
        src/ast/table.h src/ast/table.c
        src/llvm-ir/codegen.c src/llvm-ir/codegen.h
        src/ast/element.c src/ast/element.h
        src/parser/stmt_expr.c src/parser/stmt_expr.h
        src/parser/util.c src/parser/util.h
        src/parser/item.c src/parser/item.h
        src/llvm-ir/stmt_expr.c src/llvm-ir/stmt_expr.h
        src/llvm-ir/operator.c src/llvm-ir/operator.h
        src/llvm-ir/item.c src/llvm-ir/item.h
        src/std/vector.h)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs core analysis native)

# Link against LLVM libraries
target_link_libraries(chc ${llvm_libs})

target_link_libraries(chc -lstdc++)

