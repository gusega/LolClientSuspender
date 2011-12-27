#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h>

const _TCHAR* lolclient = L"LolClient.exe";
const _TCHAR* lolgame = L"League Of Legends.exe";
DWORD millisecondsToSleep = 5000;

BOOL suspendResumeProcThreads(DWORD dwOwnerPID, BOOL suspend) { 
	BOOL result = false;
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
	THREADENTRY32 te32; 
	// Fill in the size of the structure before using it. 
	te32.dwSize = sizeof(THREADENTRY32);
 
	// Take a snapshot of all running threads  
	hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
	if(hThreadSnap != INVALID_HANDLE_VALUE && Thread32First( hThreadSnap, &te32 )) {
		result = true;
		while(Thread32Next(hThreadSnap, &te32 ))  {
			if(te32.th32OwnerProcessID == dwOwnerPID) {
				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME , true, te32.th32ThreadID);			
				if (suspend) {
					SuspendThread(hThread);
				} else {
					ResumeThread(hThread);
				}
				CloseHandle(hThread);
			}
		}
	}
	if (hThreadSnap != INVALID_HANDLE_VALUE) {
		CloseHandle( hThreadSnap );
	}
	return result;
}

DWORD findProcIdByName(const _TCHAR* procName) {
	DWORD result = 0;
	PROCESSENTRY32 entry;
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 

    entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = INVALID_HANDLE_VALUE;
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (snapshot != INVALID_HANDLE_VALUE && Process32First(snapshot, &entry)) {
		while (Process32Next(snapshot, &entry) == TRUE) {
			if (_tcsicmp(entry.szExeFile, procName) == 0)
			{  
				result = entry.th32ProcessID;
			}
		}
	}
	if (snapshot != INVALID_HANDLE_VALUE) {
		CloseHandle(snapshot);
	}
	return result;
}

BOOL doSmth() {
	boolean scanLeagueOfL = true;
	DWORD lolGameId = 0;
	while (scanLeagueOfL) {
		lolGameId = findProcIdByName(lolgame);
		if (lolGameId == -1) {
			return false;
		} else if (lolGameId == 0) {
			Sleep(millisecondsToSleep);
		} else {
			scanLeagueOfL = false;
		}
	}
	//find lol game id;
	DWORD clientProcId = findProcIdByName(lolclient);
	if (clientProcId <= 0) {
		return false;
	}

	if (!suspendResumeProcThreads(clientProcId, true)) {
		return false;
	}
	HANDLE lolGame = OpenProcess(SYNCHRONIZE, true, lolGameId);
	DWORD ret = WaitForSingleObject(lolGame, INFINITE);
	CloseHandle(lolGame);
	if (!suspendResumeProcThreads(clientProcId, false)) {
		return false;
	}
	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	while (doSmth());
	return 0;
}