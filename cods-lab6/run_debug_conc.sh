#!/bin/sh

echo "Useful debug commands: b #linenumber, n for next to avoid stepping into stdlib source, s for step, r for run"
echo "Type in series size:"
read N
gdb --args pi_conc $N
