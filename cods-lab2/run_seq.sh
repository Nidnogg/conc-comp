#!/bin/sh

echo "Type in matrix dimensions"
read M_SIZE
./mult_matriz_vetor data/A$M_SIZE\x$M_SIZE.txt data/X$M_SIZE.txt data/B$M_SIZE.txt


