#include "../../include/injection.h"
#include "../../include/utils.h"
#include <tlhelp32.h>
#include <psapi.h>

namespace RustTool {

DWORD ProcessUtils::GetProcessIdByName(const std::wstring& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (processName == pe32.szExeFile) {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return 0;
}

HANDLE ProcessUtils::OpenProcessWithAccess(DWORD processId, DWORD access) {
    return OpenProcess(access, FALSE, processId);
}

bool ProcessUtils::IsProcessRunning(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (hProcess == NULL) {
        return false;
    }
    
    DWORD exitCode;
    bool isRunning = GetExitCodeProcess(hProcess, &exitCode) && exitCode == STILL_ACTIVE;
    
    CloseHandle(hProcess);
    return isRunning;
}

std::wstring ProcessUtils::GetProcessPath(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) {
        return L"";
    }
    
    wchar_t path[MAX_PATH];
    DWORD pathSize = MAX_PATH;
    
    if (QueryFullProcessImageNameW(hProcess, 0, path, &pathSize)) {
        CloseHandle(hProcess);
        return std::wstring(path);
    }
    
    CloseHandle(hProcess);
    return L"";
}

bool ProcessUtils::InjectDLL(DWORD processId, const std::wstring& dllPath) {
    // Open target process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        Logger::Error("Failed to open target process");
        return false;
    }
    
    bool result = Injection::LoadLibraryInject(hProcess, dllPath);
    CloseHandle(hProcess);
    
    return result;
}

std::vector<DWORD> ProcessUtils::GetAllProcessIds(const std::wstring& processName) {
    std::vector<DWORD> processIds;
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return processIds;
    }
    
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (processName == pe32.szExeFile) {
                processIds.push_back(pe32.th32ProcessID);
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return processIds;
}

bool Injection::LoadLibraryInject(HANDLE hProcess, const std::wstring& dllPath) {
    // Allocate memory in target process
    SIZE_T pathSize = (dllPath.length() + 1) * sizeof(wchar_t);
    LPVOID pRemotePath = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
    if (pRemotePath == NULL) {
        Logger::Error("Failed to allocate memory in target process");
        return false;
    }
    
    // Write DLL path to target process
    SIZE_T bytesWritten;
    if (!WriteProcessMemory(hProcess, pRemotePath, dllPath.c_str(), pathSize, &bytesWritten)) {
        Logger::Error("Failed to write DLL path to target process");
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        return false;
    }
    
    // Get LoadLibraryW address
    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (hKernel32 == NULL) {
        Logger::Error("Failed to get kernel32.dll handle");
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        return false;
    }
    
    LPTHREAD_START_ROUTINE pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");
    if (pLoadLibrary == NULL) {
        Logger::Error("Failed to get LoadLibraryW address");
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        return false;
    }
    
    // Create remote thread
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibrary, pRemotePath, 0, NULL);
    if (hThread == NULL) {
        Logger::Error("Failed to create remote thread");
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        return false;
    }
    
    // Wait for thread to finish
    WaitForSingleObject(hThread, INFINITE);
    
    // Get thread exit code (module handle)
    DWORD exitCode;
    GetExitCodeThread(hThread, &exitCode);
    
    // Cleanup
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
    
    if (exitCode == 0) {
        Logger::Error("LoadLibraryW failed in target process");
        return false;
    }
    
    Logger::Info("DLL injected successfully, module handle: 0x" + std::to_string(exitCode));
    return true;
}

void* Injection::GetModuleBaseAddress(HANDLE hProcess, const std::wstring& moduleName) {
    HMODULE hModules[1024];
    DWORD cbNeeded;
    
    if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            wchar_t szModName[MAX_PATH];
            
            if (GetModuleBaseNameW(hProcess, hModules[i], szModName, sizeof(szModName) / sizeof(wchar_t))) {
                if (moduleName == szModName) {
                    return hModules[i];
                }
            }
        }
    }
    
    return nullptr;
}

bool Injection::ManualMapDLL(HANDLE hProcess, const std::string& dllData) {
    // This is a simplified version - full manual mapping is complex
    // For now, we'll use LoadLibrary injection
    Logger::Warning("Manual mapping not implemented, falling back to LoadLibrary");
    return false;
}

bool Injection::SetWindowsHookInject(const std::wstring& dllPath) {
    // Alternative injection method using SetWindowsHookEx
    HMODULE hMod = LoadLibraryW(dllPath.c_str());
    if (hMod == NULL) {
        Logger::Error("Failed to load DLL for hook injection");
        return false;
    }
    
    // This would require a hook procedure in the DLL
    // For now, we'll use LoadLibrary injection
    FreeLibrary(hMod);
    Logger::Warning("Hook injection not implemented, use LoadLibrary injection");
    return false;
}

} // namespace RustTool