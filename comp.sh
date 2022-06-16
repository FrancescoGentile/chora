#!/bin/bash

mkdir -p bin
gcc -std=c11 -w `llvm-config --cflags` \
    src/main.c src/lexer/*.c src/parser/*.c src/llvm-ir/*.c src/error/*.c src/file/*.c src/ast/*.c src/std/*.c \
    `llvm-config --ldflags --libs core analysis --system-libs` -lstdc++ -o bin/chc