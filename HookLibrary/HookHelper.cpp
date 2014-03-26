#include "HookHelper.h"
#include "ntdll.h"
#include <tlhelp32.h>

const WCHAR * BadProcessnameList[] = {
	L"ollydbg.exe",
	L"idag.exe",
	L"idaw.exe",
	L"scylla.exe",
	L"scylla_x64.exe",
	L"scylla_x86.exe",
	L"ImportREC.exe"
};

bool IsProcessBad(const WCHAR * name, int nameSizeInBytes)
{
	WCHAR nameCopy[300] = { 0 };
	memcpy(nameCopy, name, nameSizeInBytes);


	for (int i = 0; i < _countof(BadProcessnameList); i++)
	{
		if (!lstrcmpiW(nameCopy, BadProcessnameList[i]))
		{
			return true;
		}
	}

	return false;
}

bool IsValidProcessHandle(HANDLE hProcess)
{
	if (hProcess == 0)
	{
		return false;
	}
	else if (hProcess == NtCurrentProcess)
	{
		return true;
	}
	else
	{
		return IsValidHandle(hProcess);
	}
}

bool IsValidThreadHandle(HANDLE hThread)
{
	if (hThread == 0)
	{
		return false;
	}
	else if (hThread == NtCurrentThread)
	{
		return true;
	}
	else
	{
		return IsValidHandle(hThread);
	}
}

bool IsValidHandle(HANDLE hHandle)
{
	//return !!GetHandleInformation(hThread, &flags); //calls NtQueryObject ObjectHandleFlagInformation
	ULONG retLen = 0;
	OBJECT_HANDLE_FLAG_INFORMATION flags;
	flags.ProtectFromClose = 0;
	flags.Inherit = 0;
	return NtQueryObject(hHandle, ObjectHandleFlagInformation, &flags, sizeof(OBJECT_HANDLE_FLAG_INFORMATION), &retLen) >= 0;
}


DWORD GetProcessIdByProcessHandle(HANDLE hProcess)
{
	PROCESS_BASIC_INFORMATION pbi;

	if (NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), 0) >= 0)
	{
		return (DWORD)pbi.UniqueProcessId;
	}
	
	return 0;
}

DWORD GetExplorerProcessId()
{
	return GetProcessIdByName(L"explorer.exe");
}

DWORD GetProcessIdByName(const WCHAR * processName)
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return 0;
	}

	DWORD pid = 0;

	do
	{
		if (!lstrcmpiW(pe32.szExeFile, processName))
		{
			pid = pe32.th32ProcessID;
			break;
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return pid;
}