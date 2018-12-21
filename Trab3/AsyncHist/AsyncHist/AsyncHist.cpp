// AsyncHist.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <Windows.h>

#define MAX_THREADS 8



BYTE buffer[100]; //Buffer to receive data from ReadFile

LONG g_nThreadsMax = MAX_THREADS, g_nThreadsCurrent = 0; //Variables for number of threads control
HANDLE IOCP; //IOCompletion Port
DWORD dwThreadIdArray[MAX_THREADS]; //Thread identifier

HANDLE asyncEvent;	//Evento para sinalizar a thread principal
LPDWORD numberOfBytesRead;	//Utilizado no readfile para saber quantos bytes foram lidos

UINT32 * fHistogram; //Histograma


OVERLAPPED over;// overlapped para cada operação


typedef VOID(*AsyncCallback)(
	LPVOID userCtx,
	DWORD status,
	UINT32 * histogram);


static DWORD WINAPI PutThreadInPool(void*Param)
{
	//Variáveis locais por thread
	DWORD numberOfBytes;
	ULONG_PTR CompletionKey;

	//Tem que estar dento de um ciclo
	InterlockedIncrement(&g_nThreadsCurrent);
	for(BOOL bStayInPool = TRUE; bStayInPool;) {

		OVERLAPPED * ol;
		//FALTA VERIFICAÇÃO DO Nº DE THREADS MÁXIMO
		BOOL bOk = GetQueuedCompletionStatus(IOCP, &numberOfBytes, &CompletionKey, &ol, 10000);
		DWORD dwIOError = GetLastError();
		
		if (bOk == FALSE) {
			printf("ERROR:%d\n", dwIOError);
			return 0;
		}


		if (bOk || (ol != NULL)) {
			//PROCESSAMENTO DOS DADOS
			if (ol == NULL) ol = &over;
			for (int i = 0; i < sizeof(buffer); i++) {

				int charToRead = buffer[i];

				if (charToRead > 64 && charToRead < 91)
					charToRead = charToRead + 32;

				if (charToRead > 96 && charToRead < 123)
				{
					charToRead = charToRead - 97;
					++fHistogram[charToRead];
				}

				//COMO OBTER O FILE PARA CONTINUAR??
				BOOL bRead = ReadFile((HANDLE)file, buffer, sizeof(buffer), numberOfBytesRead, ol) != EOF;


				//FALTA O CHAMAMENTO DO CALLBACK CHAMADO QUANDO RETORNAMOS ERRO NO PROCESSAMENTO DO READFILE
			}

			printf("PROCESSAMENTO\n");
		}

		

	}

	return 1;
}





BOOL AsyncInit()
{
	//Declare IO Completion Port
	//IO Completion Port has a pool itself
	IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	//CreateThreads and use GetQueuedCompletionStatus to associate them to IO Completion Port
	HANDLE hThread;
	for (int i = g_nThreadsCurrent; i < g_nThreadsMax; i++)
	{

		hThread = CreateThread(NULL, 0, PutThreadInPool, 0, 0, &dwThreadIdArray[i]);
		if (hThread == NULL) {
			printf("THREAD NOT CREATED");
		}

	}

	asyncEvent = CreateEvent(NULL,TRUE,FALSE, TEXT("Event"));

	return TRUE;
}




BOOL HistogramFileAsync(PCSTR file, AsyncCallback cb, LPVOID userCtx) 
{
	
	//O readfile associa uma operação ao IO completion Port
	BOOL  bRead = ReadFile((HANDLE)file, buffer, sizeof(buffer), numberOfBytesRead, &over) != EOF;

	if (bRead == FALSE) {



	}


	return TRUE;
}



int main()
{

	if (AsyncInit()) {
		printf("Threads Created");
		getchar();

		CHAR * file = new CHAR[50];
		strcpy(file,"Desktop\\so.txt");
	
		AsyncCallback cb = NULL;

		//COMO CRIAR O LPVOID?? O UserCtx é o CONTEXTO??
		LPVOID nothing = NULL;

		BOOL hOk = HistogramFileAsync(file,cb, nothing);


		//Aguardar que o callback e seja sinalizado um evento para ir para o getchar()
	}


	printf("Processamento do ficheiro terminado.");
	getchar();
	   
}









