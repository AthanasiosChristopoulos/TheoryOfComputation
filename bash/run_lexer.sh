#!/bin/bash

clear
cd ..

# Ensure the "generated" directory exists
mkdir -p generated

echo "Running Flex..."

flex -o generated/lex.yy.c phase_1/lexer_alone.l

if [ $? -ne 0 ]; then
    echo "Flex encountered an error. Stopping script."
    exit 1
fi

echo "Compiling..."

gcc -o generated/mylexer generated/lex.yy.c -lfl

if [ $? -ne 0 ]; then
    echo "Compilation failed. Stopping script."
    exit 1
fi

echo "Running the program with test_lexer.la..."

./generated/mylexer < phase_1/test_lexer.la
