#include "Mem.h"
#include "Windows.h"
#include <TlHelp32.h>


Mem::Mem()
{
	hProcess = nullptr;
}

Mem::~Mem()
{
	CloseHandle(hProcess);
}

uintptr_t Mem::getProcess(const wchar_t* procName)
{
	uintptr_t proc = 0;
	HANDLE hProcId = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(procEntry);

	if (Process32First(hProcId, &procEntry))
	{
		do
		{
			if (!_wcsicmp(procEntry.szExeFile, procName))
			{
				proc = procEntry.th32ProcessID;
				CloseHandle(hProcId);
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, proc);
			}
		} while (Process32Next(hProcId, &procEntry));
	}
	return proc;
}

uintptr_t Mem::getModule(uintptr_t proc, const wchar_t* modName)
{
	HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, proc);
	if (hModule != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hModule, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					CloseHandle(hModule);
					return (uintptr_t)modEntry.modBaseAddr;
				}
			} while (Module32Next(hModule, &modEntry));
		}
	}
	return 0;
}

uintptr_t Mem::getAddress(uintptr_t ptr, std::vector<uintptr_t> offsets)
{
	uintptr_t addr = ptr;
	for (size_t i = 0; i < offsets.size(); ++i)
	{
		addr = readMem<uintptr_t>(addr += offsets[i]);
	}
	return addr;
}

