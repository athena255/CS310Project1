#pragma once
#include <Windows.h>
#include <vector>

class Mem
{
public:
	Mem();
	~Mem();
	template <class T>
	T readMem(uintptr_t addr)
	{
		T x;
		ReadProcessMemory(hProcess, (BYTE*)addr, &x, sizeof(x), nullptr);
		return x;
	}

	template <class T>
	void writeMem(uintptr_t addr, T x)
	{ 
		WriteProcessMemory(hProcess, (BYTE*)addr, &x, sizeof(x), nullptr);

	}
	uintptr_t getProcess(const wchar_t*);
	uintptr_t getModule(uintptr_t, const wchar_t*);
	uintptr_t getAddress(uintptr_t, std::vector<uintptr_t>);
	HANDLE hProcess;
};

