#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

DWORD getProcessIdByName(const wchar_t* processName)
{
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32W processEntry = { 0 };
        processEntry.dwSize = sizeof(processEntry);

        if (Process32FirstW(snapshot, &processEntry))
        {
            do
            {
                if (wcscmp(processEntry.szExeFile, processName) == 0)
                {
                    pid = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &processEntry));
        }

        CloseHandle(snapshot);
    }

    return pid;
}

int main() {
    DWORD processId = getProcessIdByName(L"Kanjozoku Game.exe"); 

    if (!processId) {
        std::cerr << "Did you even launch the game?" << std::endl;
        Sleep(3000);
        return 0;
    }

    std::cout << "PID: " << processId << std::endl;

    int targetValue = 101010101;
    std::cout << "Enter your cars sell price:" << std::endl;
    std::cin >> targetValue;

    std::cout << "Process will close itself when completed. You can sell the car after for the money." << std::endl;

    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

    if (processHandle != NULL) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);

        MEMORY_BASIC_INFORMATION memInfo;
        LPVOID memoryAddress = 0;

        while (memoryAddress < sysInfo.lpMaximumApplicationAddress) {
            if (VirtualQueryEx(processHandle, memoryAddress, &memInfo, sizeof(memInfo)) == sizeof(memInfo)) {
                if (memInfo.State == MEM_COMMIT && (memInfo.Type == MEM_MAPPED || memInfo.Type == MEM_PRIVATE)) {
                    BYTE* buffer = new BYTE[memInfo.RegionSize];
                    SIZE_T bytesRead;

                    if (ReadProcessMemory(processHandle, memoryAddress, buffer, memInfo.RegionSize, &bytesRead)) {
                        for (size_t i = 0; i < bytesRead - sizeof(int); ++i) {
                            int value;
                            memcpy(&value, &buffer[i], sizeof(int));
                            if (value == targetValue) {
                                LPVOID writeAddress = (LPBYTE)memoryAddress + i;
                                int newValue = 100000000;
                                if (WriteProcessMemory(processHandle, writeAddress, &newValue, sizeof(newValue), NULL)) {
                                    std::cout << "Value replaced at address: " << writeAddress << std::endl;
                                }
                                else {
                                    std::cout << "Failed to write to memory." << std::endl;
                                }
                            }
                        }
                    }
                    delete[] buffer;
                }
            }
            memoryAddress = (LPBYTE)memInfo.BaseAddress + memInfo.RegionSize;
        }

        CloseHandle(processHandle);
    }
    else {
        std::cout << "Failed to open the process." << std::endl;
    }

    return 0;
}
