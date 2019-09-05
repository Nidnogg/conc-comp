#!/bin/sh

echo "Type in dimensions:"
read M_SIZE
./mult_matriz_conc data/A$M_SIZE\x$M_SIZE.txt data/A$M_SIZE\x$M_SIZE.txt data/B$M_SIZE.txt


