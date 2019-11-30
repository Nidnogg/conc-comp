
from pathlib import Path
import sys # Requires Python ver >=3.4!



#General global variables, 
reading = 0, writing = 0, waitingToWrite = 0, waitingToRead = 0, writeTurn = 0;
sharedVar = -1;  

# Reader specific variables

# Writer specific

def tRead(tid, readValue):
	"""Thread tid leu readValue"""
	reading = 1
	if(readValue != sharedVar) return -1
	else return 1
	
	return None
def tReaderBlocked(tid, logWriting, logWaitingToWrite, logWriteTurn):
	"""Leitor foi bloqueado, se writing > 0 || (waitingToWrite > 0 && writeTurn > 0)"""
	if(writing < 0 && (waitingToWrite < 0 || writeTurn < 0)) return -1 # fica ligado nisso hein viado
	return 1

def tReaderUnblocked(tid, logWriting, logWaitingToWrite, logWriteTurn):
	"""Leitor foi desbloqueado, pois ( writing > 0 || (waitingToWrite > 0 && writeTurn > 0) ) == 0"""
	return 1

def ReaderSignalled(tid, logReading):
	"""Leitor enviou signal para Escritores, pois reading == 0"""
	return 1

def tWrote(tid, writtenValue):
	"""Escritor escreveu writtenValue"""
	return 1

def tWriterBlocked(tid, logReading, logWriting, logWaitingToRead, logWriteTurn):
	"""Escritor foi bloqueado porque reading > 0 || writing > 0 || (waitingToRead > 0 && writeTurn < 0)"""
	return 1

def tWriterUnblocked(tid, logReading, logWriting, logWaitingToRead, logWriteTurn):
	"""Escritor foi desbloqueado pois  ( reading > 0 || writing > 0 || (waitingToRead > 0 && writeTurn < 0) == 0 )""" 
	return 1

def WriterSignalledBroadcasted(tid):
	"""Escritor sinalizou para escritores e broadcasteou para leitores"""
	return 1

def main():
	# Input variables
	logFilePath = Path("logs/main.txt/")
		
	if not logFilePath.exists():
		print('Error: File does not exist!')

	try:
		# Main routine
			for command in open(logFilePath, 'r'):
				if(command):
					print(str(command))
				else:
					print("what")

	except OSError as err:
			print("OS error: {0}".format(err))
	except ValueError:
			print("Could not convert data to an integer.")
	except:
			print("Unexpected error:", sys.exc_info()[0])
			raise

if __name__ = '__main__':
	main()