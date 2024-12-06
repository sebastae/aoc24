#!/bin/bash
if [ ! -f $1 ]; then 
  echo "Invalid input file"
  exit 1;
fi
grep -Po 'mul\(\d{1,3},\d{1,3}\)' $1 | sed 's/mul//' | tr "()," "   " | awk '{print $1*$2}' | paste -sd+ | bc
grep -Po "mul\(\d{1,3},\d{1,3}\)|do(n't)?\(\)" $1 | sed -E "s/mul|[\(\)]//g;s/don't/@/;s/do/_/;s/,/ /" | awk '/[0-9]/ {print $1*$2}; /^[@_]/ {print}' | paste -sd+ | sed -E 's/@[^_]*_/0/g;s/@.*?$/0/g;s/_/0/g' | bc

