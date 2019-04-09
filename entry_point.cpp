#include <Windows.h>
#include <TlHelp32.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <vector>

std::vector<int> get_pid(std::string name) {
	std::vector<int> ret;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(snapshot, &entry))
	{
		while (Process32Next(snapshot, &entry))
		{
			if (!strcmp(entry.szExeFile, name.c_str()))
				ret.push_back(entry.th32ProcessID);
		}
	}
	CloseHandle(snapshot);
	return ret;
}

typedef LONG(NTAPI *NtSuspendProcess_t)(IN HANDLE ProcessHandle);
typedef LONG(NTAPI *NtResumeProcess_t) (IN HANDLE ProcessHandle);

void suspend_process(uint32_t pid)
{
	auto process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	auto NtSuspendProcess = (NtSuspendProcess_t)GetProcAddress(GetModuleHandle("ntdll"), "NtSuspendProcess");

	NtSuspendProcess(process_handle);
	CloseHandle(process_handle);
}

void resume_process(uint32_t pid)
{
	auto process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	auto NtResumeProcess = (NtResumeProcess_t)GetProcAddress(GetModuleHandle("ntdll"), "NtResumeProcess");

	NtResumeProcess(process_handle);
	CloseHandle(process_handle);
}

int main() {
	std::vector<std::string> programs = { "client32.exe", "runplugin.exe", "Runplugin64.exe" };
	ShowWindow(GetConsoleWindow(), false);

	if (!get_pid("sys32.exe").empty())
		return 0;

	for (auto program : programs) 
		for(auto pid : get_pid(program))
			suspend_process(pid);
	
	while (!GetAsyncKeyState(VK_END)) 
		Sleep(500);
	
	for (auto program : programs)
		for (auto pid : get_pid(program))
			resume_process(pid);

	return 0;
}