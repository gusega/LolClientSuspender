#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <cstdio>
#include <iostream>

using namespace std;

const char * lolclient = "LolClient.exe";
const char * lolgame = "League Of Legends.exe";
DWORD millisecondsToSleep = 2000;

BOOL suspendResumeProcThreads(DWORD dwOwnerPID, BOOL suspend) 
{ 
  HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
  THREADENTRY32 te32; 
  // Fill in the size of the structure before using it. 
  te32.dwSize = sizeof(THREADENTRY32 ); 
 
  // Take a snapshot of all running threads  
  hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
  if( hThreadSnap == INVALID_HANDLE_VALUE ) {
    return false; 
  } 
 
  // Retrieve information about the first thread,
  // and exit if unsuccessful
  if( !Thread32First( hThreadSnap, &te32 ) ) 
  {
    CloseHandle( hThreadSnap );     // Must clean up the snapshot object!
    return false;
  }

  // Now walk the thread list of the system,
  // and suspend/resume each thread
  // associated with the specified process
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

//  Don't forget to clean up the snapshot object.
  CloseHandle( hThreadSnap );
  return true;
}

DWORD findProcIdByName(const char * procName) {
	PROCESSENTRY32 entry;
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 

    entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = INVALID_HANDLE_VALUE;
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (snapshot == INVALID_HANDLE_VALUE) {
		return -1;
	}
	
	if (!Process32First(snapshot, &entry) == TRUE)
    {
	    CloseHandle(snapshot);
		return -1;
	}

    while (Process32Next(snapshot, &entry) == TRUE)
    {
        if (_stricmp(entry.szExeFile, procName) == 0)
        {  
			CloseHandle(snapshot);
			return entry.th32ProcessID;
		}
    }
	return 0;
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
	HANDLE lolGame = OpenProcess(SYNCHRONIZE/*PROCESS_QUERY_INFORMATION*/, true, lolGameId);
	DWORD ret = WaitForSingleObject(lolGame, INFINITE);
	CloseHandle(lolGame);
	if (!suspendResumeProcThreads(clientProcId, false)) {
		return false;
	}
	return true;
}
int _tmain(int argc, _TCHAR* argv[]) {
	while (doSmth());
	return 0;
}