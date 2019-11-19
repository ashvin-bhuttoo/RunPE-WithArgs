// Runner.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream> // Standard C++ library for console I/O
#include <fstream>
#include <Windows.h> // WinAPI Header
#include "Stub.h"
using namespace std;

std::string executeCommand(const char* cmd);
int RunPortableExecutable(void* Image, string cmdLine);
HANDLE MapFileToMemory(LPCSTR filename);

int main()
{
	RunPortableExecutable(MapFileToMemory("..\\Debug\\Stub.exe"), "from_file");
	RunPortableExecutable((void*)rawData, "from_memory");

	cout << "Runner Thread." << endl;
	while(true)
		getchar();
}

// use this if you want to read the executable from disk
HANDLE MapFileToMemory(LPCSTR filename)
{
	std::streampos size;
	std::fstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		size = file.tellg();

		char* Memblock = new char[size]();

		file.seekg(0, std::ios::beg);
		file.read(Memblock, size);
		file.close();

		return Memblock;
	}
	return 0;
}

int RunPortableExecutable(void* Image, string cmdLine)
{
	IMAGE_DOS_HEADER* DOSHeader;
	IMAGE_NT_HEADERS* NtHeader;
	IMAGE_SECTION_HEADER* SectionHeader;
	PROCESS_INFORMATION PI;
	STARTUPINFOA SI;
	CONTEXT* CTX;
	DWORD* ImageBase = NULL;
	void* pImageBase = NULL;
	int count;
	char CurrentFilePath[1024];
	DOSHeader = PIMAGE_DOS_HEADER(Image);
	NtHeader = PIMAGE_NT_HEADERS(DWORD(Image) + DOSHeader->e_lfanew);
	GetModuleFileNameA(0, CurrentFilePath, 1024);
	if (NtHeader->Signature == IMAGE_NT_SIGNATURE)
	{
		ZeroMemory(&PI, sizeof(PI));
		ZeroMemory(&SI, sizeof(SI));
		cmdLine = "Stub.exe " + cmdLine;
		bool threadcreated = CreateProcessA(CurrentFilePath, (LPSTR)cmdLine.c_str(), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &SI, &PI);
		if (threadcreated == true)
		{
			CTX = LPCONTEXT(VirtualAlloc(NULL, sizeof(CTX), MEM_COMMIT, PAGE_READWRITE));
			CTX->ContextFlags = CONTEXT_FULL;
			if (GetThreadContext(PI.hThread, LPCONTEXT(CTX)))
			{
				ReadProcessMemory(PI.hProcess, LPCVOID(CTX->Ebx + 8), LPVOID(&ImageBase), 4, 0);
				pImageBase = VirtualAllocEx(PI.hProcess, LPVOID(NtHeader->OptionalHeader.ImageBase),
					NtHeader->OptionalHeader.SizeOfImage, 0x3000, PAGE_EXECUTE_READWRITE);
				if (pImageBase == 00000000) {
					ResumeThread(PI.hThread);
					ExitProcess(NULL);
					return 1;
				}
				if (pImageBase > 0) {
					WriteProcessMemory(PI.hProcess, pImageBase, Image, NtHeader->OptionalHeader.SizeOfHeaders, NULL);
					for (count = 0; count < NtHeader->FileHeader.NumberOfSections; count++)
					{
						SectionHeader = PIMAGE_SECTION_HEADER(DWORD(Image) + DOSHeader->e_lfanew + 248 + (count * 40));
						WriteProcessMemory(PI.hProcess, LPVOID(DWORD(pImageBase) + SectionHeader->VirtualAddress),
							LPVOID(DWORD(Image) + SectionHeader->PointerToRawData), SectionHeader->SizeOfRawData, 0);
					}
					WriteProcessMemory(PI.hProcess, LPVOID(CTX->Ebx + 8),
						LPVOID(&NtHeader->OptionalHeader.ImageBase), 4, 0);
					CTX->Eax = DWORD(pImageBase) + NtHeader->OptionalHeader.AddressOfEntryPoint;
					SetThreadContext(PI.hThread, LPCONTEXT(CTX));
					ResumeThread(PI.hThread);
					return 0;
				}
			}
		}
	}
}


std::string executeCommand(const char* cmd) {
	char buffer[128];
	std::string result = "";
	std::shared_ptr<FILE> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe) throw std::runtime_error("popen() failed!");

	while (!feof(pipe.get())) {
		if (fgets(buffer, 128, pipe.get()) != NULL)
			result += buffer;
	}
	return result;
}