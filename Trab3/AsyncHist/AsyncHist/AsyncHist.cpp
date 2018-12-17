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

HANDLE asyncEvent;

UINT32 * fHistogram;

LPDWORD numberOfBytesRead;	//Utilizado no readfile para saber quantos bytes foram lidos


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
			printf("PROCESSAMENTO\n");
		}

		//FALTA O CHAMAMENTO DO CALLBACK QUANDO RETORNAMOS ERRO NO PROCESSAMENTO DO READFILE

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
	BOOL bRead;
	OVERLAPPED over; //POR FICHERO/OPERACAO //NAO PODE SER LOCAL

	//O readfile associa uma operação ao IO completion Port
	bRead = ReadFile((HANDLE)file, buffer, sizeof(buffer), numberOfBytesRead, &over) != EOF;

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

		//COMO CRIAR O LPVOID?? O UserCtx é o OVERLAPPED??
		LPVOID nothing = NULL;

		BOOL hOk = HistogramFileAsync(file,cb, nothing);
	}



	//


}









