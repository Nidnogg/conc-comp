from pathlib import Path
import sys # Requires Python ver >=3.4!

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

readerSignal = 1 #tem que ficar ligado
writerSignal = 1

isFirstThread = 1

globalSetterCode = ["global NTHREADS_READ", \
					"global NTHREADS_WRITE", \
					"global nReads", \
					"global nWrites", \
					"global reading", \
					"global writing", \
					"global waitingToWrite", \
					"global waitingToRead", \
					"global writeTurn", \
					"global sharedVar", \
					"global readerSignal", \
					"global writerSignal", \
					"global isFirstThread" ] 

def commandLineParametersRead(nReaderThreads, nWriterThreads, logNReads, logNWrites):
	global NTHREADS_READ
	global NTHREADS_WRITE
	global nReads
	global nWrites
	global reading
	global writing
	global waitingToWrite
	global waitingToRead
	global writeTurn
	global sharedVar
	global readerSignal
	global writerSignal
	global isFirstThread
	global evilDebug

	NTHREADS_READ = nReaderThreads
	NTHREADS_WRITE = nWriterThreads
	nReads = logNReads
	nWrites = logNWrites

	return 1

def tRead(tid, readValue):
	"""Thread tid leu readValue"""
	global NTHREADS_READ
	global NTHREADS_WRITE
	global nReads
	global nWrites
	global reading
	global writing
	global waitingToWrite
	global waitingToRead
	global writeTurn
	global sharedVar
	global readerSignal
	global writerSignal
	global isFirstThread

	if((readValue != sharedVar) or writing > 0): 
		return 0
	else:
		reading -= 1
		return 1

def tReaderStartRead(tid):
	"""Thread tid leu readValue"""
	global NTHREADS_READ
	global NTHREADS_WRITE
	global nReads
	global nWrites
	global reading
	global writing
	global waitingToWrite
	global waitingToRead
	global writeTurn
	global sharedVar
	global readerSignal
	global writerSignal
	global isFirstThread

	reading += 1
	writeTurn = 1

	return 1
	
def tReaderBlocked(tid, logWriting, logWaitingToWrite, logWriteTurn):
	"""Leitor foi bloqueado, se writing > 0 || (waitingToWrite > 0 && writeTurn > 0)"""
	global NTHREADS_READ
	global NTHREADS_WRITE
	global nReads
	global nWrites
	global reading
	global writing
	global waitingToWrite
	global waitingToRead
	global writeTurn
	global sharedVar
	global readerSignal
	global writerSignal
	global isFirstThread

	waitingToRead += 1
	print("waitingToRead INTERNAL = " + str(waitingToRead))
	print("writeTurn internal = " + str(writeTurn))
	print("writing = " + str(writing))

	if(writing > 0 or (waitingToWrite > 0 and writeTurn > 0)):
		return 1
	else: 
		return 0

def tReaderUnblocked(tid, logWriting, logWaitingToWrite, logWriteTurn):
	"""Leitor foi desbloqueado, pois ( writing > 0 || (waitingToWrite > 0 && writeTurn > 0) ) == 0"""
	global NTHREADS_READ
	global NTHREADS_WRITE
	global nReads
	global nWrites
	global reading
	global writing
	global waitingToWrite
	global waitingToRead
	global writeTurn
	global sharedVar
	global readerSignal
	global writerSignal
	global isFirstThread

	if(isFirstThread):
		readerSignal = 1
		isFirstThread = 0

	if(readerSignal - 1 < 0): 
		return 0
	else: readerSignal -= 1
	
	
	if (not(writing > 0 or (waitingToWrite > 0 and writeTurn > 0))): 
		waitingToRead -= 1
		reading += 1
		return 1
		
	else: 
		return 0

def tReaderSignalled(tid, logReading):
	"""Leitor enviou signal para Escritores, pois reading == 0"""
	global NTHREADS_READ
	global NTHREADS_WRITE
	global nReads
	global nWrites
	global reading
	global writing
	global waitingToWrite
	global waitingToRead
	global writeTurn
	global sharedVar
	global readerSignal
	global writerSignal
	global isFirstThread

	#if(reading > 0): return 0
	writerSignal += 1
	return 1

def tWrote(tid, writtenValue):
	"""Escritor escreveu writtenValue"""
	global NTHREADS_READ
	global NTHREADS_WRITE
	global nReads
	global nWrites
	global reading
	global writing
	global waitingToWrite
	global waitingToRead
	global writeTurn
	global sharedVar
	global readerSignal
	global writerSignal
	global isFirstThread

	#print("writtenValue = "  + str(writtenValue) + " tid = " + str(tid) + " writing = " + str(writing) + " reading = " + str(reading) ) 
	if(writtenValue != tid or writing > 1 or reading > 0):
		return 0
	sharedVar = writtenValue
	writing -= 1
	return 1

def tWriterStartWrite(tid):
	"""Thread tid leu readValue"""
	global NTHREADS_READ
	global NTHREADS_WRITE
	global nReads
	global nWrites
	global reading
	global writing
	global waitingToWrite
	global waitingToRead
	global writeTurn
	global sharedVar
	global readerSignal
	global writerSignal
	global isFirstThread

	writing += 1
	return 1


def tWriterBlocked(tid, logReading, logWriting, logWaitingToRead, logWriteTurn):
	"""Escritor foi bloqueado porque reading > 0 || writing > 0 || (waitingToRead > 0 && writeTurn < 0)"""
	global NTHREADS_READ
	global NTHREADS_WRITE
	global nReads
	global nWrites
	global reading
	global writing
	global waitingToWrite
	global waitingToRead
	global writeTurn
	global sharedVar
	global readerSignal
	global writerSignal
	global isFirstThread

	waitingToWrite += 1
	print("wwaitingToWrite internal = " + str(waitingToWrite))

	print("writeTurn internal = " + str(writeTurn))
	print("reading int = " + str(reading))
	print("writing internal = " + str(writing))


	if(reading > 0 or writing > 0 or (waitingToRead > 0 and writeTurn < 0)):
		return 1
	else: 
		return 0

def tWriterUnblocked(tid, logReading, logWriting, logWaitingToRead, logWriteTurn):
	"""Escritor foi desbloqueado pois  ( reading > 0 || writing > 0 || (waitingToRead > 0 && writeTurn < 0) == 0 )""" 
	global NTHREADS_READ
	global NTHREADS_WRITE
	global nReads
	global nWrites
	global reading
	global writing
	global waitingToWrite
	global waitingToRead
	global writeTurn
	global sharedVar
	global readerSignal
	global writerSignal
	global isFirstThread

	if(isFirstThread):
		writerSignal = 1
		isFirstThread = 0

	if(writerSignal - 1 < 0): return 0
	else: writerSignal -= 1

	if not((reading > 0 or writing > 0 or (waitingToRead > 0 and writeTurn < 0))):
		waitingToWrite -= 1
		writing += 1
		writeTurn = -1
		return 1

	else: 
		return 0

def tWriterSignalledBroadcasted(tid):
	"""Escritor sinalizou para escritores e broadcasteou para leitores"""
	global NTHREADS_READ
	global NTHREADS_WRITE
	global nReads
	global nWrites
	global reading
	global writing
	global waitingToWrite
	global waitingToRead
	global writeTurn
	global sharedVar
	global readerSignal
	global writerSignal
	global isFirstThread

	#devodarmenos1aqui? (tem que ver)
	writerSignal += 1
	readerSignal = NTHREADS_READ
	return 1

def main():
	# Input variables
	logFilePath = Path("logs/mainTest.txt/")
		
	if not logFilePath.exists():
		print('Error: File does not exist!')

	try:
		# Main routine
		for command in open(logFilePath, 'r'):
			if(eval(command)):
				pass
				#print("waitingToRead = " + str(waitingToRead))
				#print("waitingToWrite = " + str(waitingToWrite))
				print("reading = " + str(reading))
				print(command.strip("\n") + " is correct")
			else:
				print(command.strip("\n") + " has failed!")
		open(logFilePath, 'r').close()

	except OSError as err:
			print("OS error: {0}".format(err))
	except ValueError:
			print("Could not convert data to an integer.")
	except:
			print("Unexpected error:", sys.exc_info()[0])
			raise


if __name__ == '__main__':
	main()