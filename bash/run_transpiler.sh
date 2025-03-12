#!/bin/bash

clear
cd ..

mkdir -p generated

# INPUT_FILE="test_all.la"
# INPUT_FILE="test.la"
# INPUT_FILE="correct1.la"
INPUT_FILE="correct2.la"

SKIP_EXECUTION="$1"

echo "Running Bison..."
bison -d phase_2/tuc_transpiler.y -o generated/tuc_transpiler.tab.c
if [ $? -ne 0 ]; then
    echo "Bison encountered an error. Stopping script."
    exit 1
fi

echo "Running Flex..."
flex -o generated/lex.yy.c phase_2/lexer.l
if [ $? -ne 0 ]; then
    echo "Flex encountered an error. Stopping script."
    exit 1
fi

echo "Compiling..."
gcc -o generated/mycompiler generated/lex.yy.c generated/tuc_transpiler.tab.c generated/tuc_transpiler.tab.h phase_2/cgen.c -Iphase_2 -lfl
if [ $? -ne 0 ]; then
    echo "Compilation failed. Stopping script."
    exit 1
fi

cd phase_2
if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: $INPUT_FILE not found!"
    exit 1
fi

echo "Running the program with $INPUT_FILE..."
../generated/mycompiler < "$INPUT_FILE" $INPUT_FILE
if [ $? -ne 0 ]; then
    echo "Execution of mycompiler failed. Stopping script."
    exit 1
fi
cd ..

# ============================================================================
# Compiling outputted program

if [ "$SKIP_EXECUTION" != "1" ]; then
    echo ""
    echo "Compiling output.c..."
    gcc -o generated/output phase_2/output.c
    if [ $? -ne 0 ]; then
        echo "Compilation failed. Please check your code for errors."
        exit 1
    fi

    echo "Executing output..."
    ./generated/output
fi
