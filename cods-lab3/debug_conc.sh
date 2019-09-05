#!/bin/sh

echo "Useful debug commands: b #linenumber, n for next to avoid stepping into stdlib source, s for step, r for run"
echo "Type in matrix dimensions"
read M_SIZE
gdb --args mult_matriz_conc data/A$M_SIZE\x$M_SIZE.txt data/A$M_SIZE\x$M_SIZE.txt data/B$M_SIZE.txt


