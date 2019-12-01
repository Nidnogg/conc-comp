# Variáveis de entrada e saída
NTHREADS_READ = 0
NTHREADS_WRITE = 0
nReads = 0
nWrites = 0 

#General global variables
reading = 0
writing = 0
waitingToWrite = 0
waitingToRead = 0
writeTurn = 0
sharedVar = -1

readerSignal = 0 
writerSignal = 0

isFirstThread = 1