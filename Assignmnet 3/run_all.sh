#!/bin/bash
# Compile and run every program; outputs go to outputs/
mkdir -p build outputs data
for f in src/q*.c; do
  b=$(basename "$f" .c)
  gcc -O2 -Wall -Wextra "$f" -o "build/$b" -lm || { echo "compile failed: $f"; continue; }
  echo "== $b"; ./build/$b | tee "outputs/$b.txt"
done
