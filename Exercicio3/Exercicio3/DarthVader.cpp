// DarthVader.cpp : Defines the entry point for the console application.
// Creates the father thread
//

#include "stdafx.h"
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>




#define BLOCK_SIZE 1000


int _tmain(int argc, _TCHAR* argv[])
{

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES) };
	DWORD res;

	_tprintf(_T("process_sharing_objects: begin\n"));
	sa.bInheritHandle = TRUE;
	HANDLE mapFile = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, BLOCK_SIZE, NULL);
	_tprintf(_T("process_sharing_objects: mapFile handle = %d\n"), (INT)(UINT_PTR)mapFile);
	assert(mapFile != NULL);
	TCHAR * block = (TCHAR*)MapViewOfFile(mapFile, FILE_MAP_ALL_ACCESS, 0, 0, BLOCK_SIZE);


	TCHAR commandLine[100];

	//Tries to get message from command line
	//TODO
	MSG msg;
	BOOL bRet;

	while ((bRet = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//
	//


	PTCHAR message = _T("TEST");
	_tcscpy(block, message);



	TCHAR childName;
	//TCHAR childName = _T("NOME DO OUTRO CPP"):



	return 0;
}

