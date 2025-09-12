#!/bin/bash

# Compile the Tetris game and capture errors
g++ -o tetris main.cpp -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system 2>&1 > compilererror.txt

echo "Compilation output saved to compilererror.txt"
