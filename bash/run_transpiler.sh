#!/bin/bash
 
clear
cd ..

mkdir -p generated

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

echo "Running the program with test1.in..."
cd phase_2
../generated/mycompiler < test1.in
cd ..