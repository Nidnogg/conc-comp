 # -*- coding: utf-8 -*-
from pathlib import Path
import os
import sys # Requer python versão maior que 3.4!
# Trabalho 2 de Implementação - Computação Concorrente 2019.2
# Grupo: Henrique Vermelho de Toledo, João Pedro Lopes Murtinho
# O terceiro problema de Leitor e Escritor (sem starvation)

# READ ME(!) - este código foi testado em máquinas Windows  (powershell, Windows 10), Linux (bash, elementary linux), Macintosh (bash, el capitan)
# e windows 10 com WSL (ubuntu embutido). De todas os environments, WSL é o único que apresentou problemas com os caminhos dos arquivos de log. EVITE WSL!!!
# Além disso, REQUER PYTHON VERSÃO > 3.4 PARA O MÓDULO SYS!!!!!!!!

# Variáveis de entrada e saída
NTHREADS_READ = 0
NTHREADS_WRITE = 0
nReads = 0
nWrites = 0 

# Variáveis do problema leitor escritor base, todas descritas no código em C.
reading = 0
writing = 0
waitingToWrite = 0
waitingToRead = 0
writeTurn = 0
sharedVar = -1

# As duas variáveis a seguir representam uma implementação rudimentar de um semáforo, 
# no qual elas são incrementadas de NTHREADS_READ (por conta do broadcast) e 1, respectivamente,
# toda vez que uma chamada tReaderSignalled ou tWriterSignalledBroadcasted é feita.

# Cada thread "gasta" esse signal decrementando de 1 toda vez que uma chamada tReaderUnblocked ou tWriterUnblocked é feita,
# para indicar que foi desbloqueada corretamente. Ambas inicializam em 0, e a primeira thread que aparecer recebe um signal = 1 para poder seguir.
readerSignal = 0     
writerSignal = 0

# A primeira thread é importante para determinar quem recebe o primeiro signal
isFirstThread = 1

def commandLineParametersRead(nReaderThreads, nWriterThreads, logNReads, logNWrites):
	global NTHREADS_READ
	global NTHREADS_WRITE
	global nReads
	global nWrites

	NTHREADS_READ = nReaderThreads
	NTHREADS_WRITE = nWriterThreads
	nReads = logNReads
	nWrites = logNWrites

	return 1

def tRead(tid, readValue):
	"""Thread tid leu readValue"""
	global reading
	global writing
	global writeTurn
	global sharedVar

	# se é inconsistente com o que acontece.
	if((readValue != sharedVar) or writing > 0): 
		return 0
	else:
		reading -= 1
		return 1

def tReaderStartRead(tid):
	"""Thread tid vai começar a ler, gasta readerSignal pra ver se espera ou não."""
	global readerSignal
	global reading
	global writeTurn
	global isFirstThread

	if(isFirstThread):
		readerSignal = 1
		isFirstThread = 0 #tava no tUnblocked antes

	reading += 1
	writeTurn = -1

	return 1
	
def tReaderBlocked(tid, logWriting, logWaitingToWrite, logWriteTurn):
	"""Leitor foi bloqueado, se writing > 0 || (waitingToWrite > 0 && writeTurn > 0)"""
	global writing
	global waitingToRead
	global waitingToWrite
	global writeTurn

	waitingToRead += 1

	# Checagem das variáveis de contexto internas do programa em C - se elas não fazem sentido então há algo de errado nos valores durante a execução
	if not(logWriting > 0 or (logWaitingToWrite > 0 and logWriteTurn) > 0):
		return 0

	# As demais checagens são feitas reconstruindo uma lógica interna de leitor escritor em Python.
	if(writing > 0 or (waitingToWrite > 0 and writeTurn > 0)):
		writeTurn = -1 #RISK
		return 1
	else: 
		return 0

def tReaderUnblocked(tid, logWriting, logWaitingToWrite, logWriteTurn):
	"""Leitor foi desbloqueado, pois recebeu signal ou broadcast"""
	global waitingToWrite
	global waitingToRead
	global readerSignal

	if(readerSignal - 1 < 0): 
		print(readerSignal)
		return 0

	else:
		readerSignal -= 1
		waitingToRead -= 1
		return 1

def tReaderSignalled(tid, logReading):
	"""Leitor enviou signal para Escritores, pois reading == 0"""
	global writeTurn
	global writerSignal

	writerSignal += 1
	writeTurn = 1

	return 1

def tWrote(tid, writtenValue):
	"""Escritor escreveu writtenValue"""
	global NTHREADS_READ
	global NTHREADS_WRITE
	global reading
	global writing
	global sharedVar

	if(writtenValue != tid or writing > 1 or reading > 0):
		return 0
	sharedVar = writtenValue
	return 1

def tWriterStartWrite(tid):
	"""Thread tid leu readValue"""
	global writing
	global writerSignal
	global isFirstThread

	if(isFirstThread):
		writerSignal = 1
		isFirstThread = 0

	writing += 1
	return 1


def tWriterBlocked(tid, logReading, logWriting, logWaitingToRead, logWriteTurn):
	"""Escritor foi bloqueado porque reading > 0 || writing > 0 || (waitingToRead > 0 && writeTurn < 0)"""
	global reading
	global writing
	global waitingToWrite
	global waitingToRead
	global writeTurn

	waitingToWrite += 1

	if not(logReading > 0 or logWriting > 0 or (logWaitingToRead > 0 and logWriteTurn < 0)):
		return 0

	if(reading > 0 or writing > 0 or (waitingToRead > 0 or writeTurn < 0)): #risk
		return 1
	else: 
		return 0

def tWriterUnblocked(tid, logReading, logWriting, logWaitingToRead, logWriteTurn):
	"""Escritor foi desbloqueado pois recebeu signal""" 
	global waitingToWrite
	global writerSignal

	if(writerSignal - 1 < 0): 
		return 0

	else:
		writerSignal -= 1
		waitingToWrite -= 1
		return 1

def tWriterSignalledBroadcasted(tid):
	"""Escritor sinalizou para escritores e broadcasteou para leitores"""
	global NTHREADS_READ
	global writing
	global readerSignal
	global writerSignal

	writerSignal += 1
	readerSignal = NTHREADS_READ
	writing -= 1

	return 1

def timeSpent(time):
	print("Tempo de execução: " + str(time) + "s")
	return 1

# Reseta as variáveis globais entre cada arquivo testado. ! Muito importante
def resetGlobals():
	
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

	NTHREADS_READ = 0
	NTHREADS_WRITE = 0
	nReads = 0
	nWrites = 0 
	reading = 0
	writing = 0
	waitingToWrite = 0
	waitingToRead = 0
	writeTurn = 0
	sharedVar = -1
	readerSignal = 0     
	writerSignal = 0
	isFirstThread = 1

def main():
	# Lista com arquivos de logs a serem testados
	logFilePaths = []

	# Coloca na lista logFilePaths todos os arquivos log na pasta logs
	for file in os.listdir("logs"):
		if file.endswith(".txt"):
			logFilePaths.append(Path("logs/" + file))

	# Para cada .txt principal no logs, abre-se o arquivo em modo read e executa cada comando via eval()
	for currentTestPath in logFilePaths:
		if not currentTestPath.exists():
			print('Error: File does not exist!')
		try:
			# Rotina principal, por arquivo de teste. Checa se todos os comandos foram executados corretamente
			lineCounter = 0   # contador de linhas para diagnóstico
			failedLineCounter = 0  # conta Linhas problemáticas
			print('Testando ' + str(currentTestPath)) 
			logFile = open(Path(currentTestPath), 'r')
			for lineNum, command in enumerate(logFile, start=1):
				if(eval(command)):
					lineCounter += 1
				else:
					print(command.strip("\n") + " falhou na linha " + str(lineNum))
					failedLineCounter += 1
			# Prints para diagnóstico
			print('Arquivo testado com sucesso')
			print('Linhas testadas: ' + str(lineCounter + failedLineCounter))
			print('Linhas errôneas detectadas:' + str(failedLineCounter) + '\n')
			resetGlobals()
			logFile.close()

		# Exceções de abertura, valor, e inesperadas capturadas no bloco try catch a seguir
		except OSError as err:
				print("OS error: {0}".format(err))
		except ValueError as err:
				print("ValueError: {0}".format(err))
		except:
				print("Unexpected error:", sys.exc_info()[0])
				raise

if __name__ == '__main__':
	main()