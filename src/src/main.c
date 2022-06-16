//
//
//

#include "file/file.h"
#include "file/buffer.h"
#include "error/handler.h"
#include "error/style.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "llvm-ir/codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

bool create_file(File *file, char *filename) {

    u64 len = strlen(filename);
    char *ext = ".ch";
    for (u64 i = len - 3; i < len; ++i, ++ext) {
        if (filename[i] != *ext) {
            fprintf(stderr, "chc: %s%serror%s: file extension must be 'ch'\n", BOLD, RED, WHITE);
            return false;
        }
    }

    int res = init_file(file, filename);
    if (res != 0) {
        fprintf(stderr, "chc: %s%serror%s: no such file '%s'\n", BOLD, RED, WHITE, filename);
        return false;
    }

    return true;
}

bool create_ll_file(ASTFile *f, char *filename) {

    CodeGenerator cg;
    init_generator(&cg, filename);
    bool valid = generate_ir(&cg, f);

    if (valid) {
        u64 len = strlen(filename);
        filename[len - 2] = filename[len - 1] = 'l';

        FILE *out = fopen(filename, "wb");
        if (out == NULL) {
            fprintf(stderr, "chc: %s%serror%s: unable to create file '%s'\n", BOLD, RED, WHITE, filename);
            valid = false;
        }

        print_ir(&cg, out);
        fclose(out);
    } else {
        fprintf(stderr, "CODE-ERROR (code-generation): error in intermediate code\n");
        valid = false;
    }

    free_generator(&cg);
    return valid;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "chc: %s%serror%s: no input file\n", BOLD, RED, WHITE);
        exit(EXIT_FAILURE);
    }

    File file;
    bool valid = create_file(&file, argv[1]);
    if (!valid) {
        free_file(&file);
    } else {
        FileBuffer buffer;
        init_buffer(&buffer, &file);

        ErrorHandler eh;
        init_handler(&eh, &file);

        Lexer lexer;
        init_lexer(&lexer, &buffer, &eh);

        SymbolTable symbol_table;
        init_symbol_table(&symbol_table);

        Parser parser;
        init_parser(&parser, &lexer, &symbol_table, &eh);

        ASTFile *f = generate_ast(&parser);

        if (!any_error(&eh)) {
            valid = create_ll_file(f, argv[1]);
        }

        print_messages(&eh);

        free_ast_file(f);
        free(f);
        free_symbol_table(&symbol_table);
        free_lexer(&lexer);
        free_handler(&eh);
        free_file(&file);
    }

    return (valid ? EXIT_SUCCESS : EXIT_FAILURE);
}
