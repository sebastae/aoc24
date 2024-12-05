#!/bin/bash
if [ ! -f $1 ]; then 
  echo "Invalid input file"
  exit 1;
fi
grep -Po 'mul\(\d{1,3},\d{1,3}\)' $1 | sed 's/mul//' | tr "()," "   " | awk '{print $1*$2}' | paste -sd+ | bc
