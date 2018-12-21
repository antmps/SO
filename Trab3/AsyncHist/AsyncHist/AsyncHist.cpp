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


OVERLAPPED over;// overlapped para cada operação

//Interface do AsyncCallback
typedef VOID(*AsyncCallback)(
	LPVOID userCtx,
	DWORD status,
	UINT32 * histogram);


//contexto passado por cada histograma
typedef struct {
	HANDLE hFile;
	AsyncCallback * cb;
	LPVOID userCTX;
	UINT32 * fHistogram;
} THREAD_CTX, *PTHREAD_CTX;


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
		BOOL bOk = GetQueuedCompletionStatus(IOCP, &numberOfBytes, &CompletionKey, &ol, INFINITE);
		DWORD dwIOError = GetLastError();

		PTHREAD_CTX  tCTX = (PTHREAD_CTX)CompletionKey;

		if (bOk == FALSE) {
			if (dwIOError == ERROR_HANDLE_EOF) {
				UINT32 * hist = tCTX->fHistogram;
				LPVOID uCTX = tCTX->userCTX;
				AsyncCallback *cb = tCTX->cb;
				(*cb)(uCTX, 0, hist);
				return 1;
			}
			else {
				printf("ERROR DW: %d\n", dwIOError);
				return 0;
			}
		}


		if (bOk || (ol != NULL)) {
			//PROCESSAMENTO DOS DADOS
			PTHREAD_CTX  tCTX = (PTHREAD_CTX)CompletionKey;
			for (int i = 0; i < sizeof(buffer); i++) {

				int charToRead = buffer[i];

				if (charToRead >= 'A' && charToRead <= 'Z')
					charToRead = charToRead  + ('a'-'A');

				if (charToRead >= 'a' && charToRead <= 'z')
				{
					charToRead = charToRead;
					printf("CARACTER: %c\n", charToRead);
					++(tCTX->fHistogram[charToRead]);
				}
			}
			ol->Offset += numberOfBytes;

			BOOL bRead = ReadFile(tCTX->hFile, buffer, sizeof(buffer), NULL, ol) ;
			if (!bRead && GetLastError() == ERROR_HANDLE_EOF) {
				//Chamamento da função callback passada no contexto
				(*tCTX->cb)(tCTX->userCTX, 0, tCTX->fHistogram);

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

BOOL AssociateDeviceWithCompletionPort(HANDLE hComplPort, HANDLE hDevice, DWORD CompletionKey) {
	HANDLE h = CreateIoCompletionPort(hDevice, hComplPort, CompletionKey, 0);
	return h == hComplPort;
}


BOOL HistogramFileAsync(PCSTR file, AsyncCallback cb, LPVOID userCtx) 
{
	//Createfile para obter o handle
	HANDLE hFile = CreateFileA(file,GENERIC_READ,0,NULL,OPEN_EXISTING, FILE_FLAG_OVERLAPPED,NULL);
	
	//Histograma para inserir os valores
	UINT32 * fHistogram = new UINT32['z'-'a'];

	//contexto que será utilizado nesta operação
	//ADICIONAR OVERLAPPED
	PTHREAD_CTX tCTX = new THREAD_CTX;
	tCTX->cb = &cb;
	tCTX->hFile = hFile;
	tCTX->fHistogram = fHistogram;
	tCTX->userCTX = userCtx;
	

	//Associar ao IOCP
	if (!AssociateDeviceWithCompletionPort(IOCP, hFile, (DWORD)tCTX)) {
		(*cb)(userCtx,GetLastError(),NULL);
		return FALSE;
	}
	
	//O readfile associa uma operação ao IO completion Port
	BOOL  bRead = ReadFile(hFile, buffer, sizeof(buffer), NULL, &over);

	//Caso não seja possível efectuar a leitura do ReadFile
	if (bRead == FALSE && GetLastError() != ERROR_IO_PENDING) {
		(*cb)(userCtx, GetLastError(), NULL);
		return FALSE;	
	}


	return TRUE;
} 



int main()
{

	if (AsyncInit()) {
		printf("Threads Created\n");

		CHAR * file = new CHAR[50];
		strcpy(file,"C:\\Users\\user\\Desktop\\so.txt");
	
		AsyncCallback cb = [](LPVOID userCtx, DWORD status, UINT32 * histogram) {
			if (status != 0) { printf("ERRO: %d\n", status); return; }
			for (int i = 0; i < 26; ++i) {
				printf("%c, %d", ('a' + i), histogram[i]);
			}
			getchar();
		};
		BOOL hOk = HistogramFileAsync(file, cb, NULL);

		
		//Aguardar que o callback e seja sinalizado um evento para ir para o getchar()
	}
	

	printf("Processamento do ficheiro terminado.\n");
	getchar();
	   
}









