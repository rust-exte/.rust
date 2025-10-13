#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>
#include "../include/injection.h"
#include "../include/utils.h"

using namespace RustTool;

void PrintBanner() {
    std::cout << R"(
    ____             __     ______            __   ___   ____ ___  ____  
   / __ \__  _______/ /_   /_  __/___  ____  / /  |__ \ / __ \__ \/ ___/ 
  / /_/ / / / / ___/ __/    / / / __ \/ __ \/ /   __/ // / / /_/ /\__ \  
 / _, _/ /_/ (__  ) /_     / / / /_/ / /_/ / /   / __// /_/ / __/___/ /  
/_/ |_|\__,_/____/\__/    /_/  \____/\____/_/   /____/\____/____//____/  
                                                                          
)" << std::endl;
    std::cout << "Rust Tool 2025 - Silent Loader v2.5.1" << std::endl;
    std::cout << "========================================" << std::endl;
}

bool WaitForRustProcess() {
    std::wcout << L"[INFO] Waiting for Rust process..." << std::endl;
    
    while (true) {
        DWORD rustPid = ProcessUtils::GetProcessIdByName(L"RustClient.exe");
        if (rustPid != 0) {
            std::wcout << L"[INFO] Found Rust process (PID: " << rustPid << L")" << std::endl;
            return true;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::wcout << L"." << std::flush;
    }
    
    return false;
}

bool InjectDLL() {
    DWORD rustPid = ProcessUtils::GetProcessIdByName(L"RustClient.exe");
    if (rustPid == 0) {
        std::wcout << L"[ERROR] Rust process not found!" << std::endl;
        return false;
    }
    
    // Get DLL path
    std::wstring executableDir = Utils::GetExecutableDirectory();
    std::wstring dllPath = executableDir + L"\\rust-tool.dll";
    
    if (!Utils::FileExists(dllPath)) {
        std::wcout << L"[ERROR] DLL not found: " << dllPath << std::endl;
        return false;
    }
    
    std::wcout << L"[INFO] Injecting DLL: " << dllPath << std::endl;
    
    // Attempt injection
    if (ProcessUtils::InjectDLL(rustPid, dllPath)) {
        std::wcout << L"[SUCCESS] DLL injected successfully!" << std::endl;
        
        // Play confirmation beep
        Utils::CreateBeep(800, 200);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Utils::CreateBeep(1000, 200);
        
        return true;
    } else {
        std::wcout << L"[ERROR] Failed to inject DLL!" << std::endl;
        return false;
    }
}

void ShowUsageInstructions() {
    std::cout << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << "              USAGE INSTRUCTIONS              " << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << "1. Launch Rust through Steam" << std::endl;
    std::cout << "2. Wait for the game to fully load" << std::endl;
    std::cout << "3. Press INSERT to open the tool menu" << std::endl;
    std::cout << std::endl;
    std::cout << "Default Hotkeys:" << std::endl;
    std::cout << "  INSERT     - Toggle Menu" << std::endl;
    std::cout << "  F1         - Toggle Aimbot" << std::endl;
    std::cout << "  F2         - Toggle ESP" << std::endl;
    std::cout << "  F3         - Toggle No Recoil" << std::endl;
    std::cout << "  F4         - Toggle Fly Hack" << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << std::endl;
}

int main() {
    // Initialize
    PrintBanner();
    Logger::Initialize(L"loader.log", true, true);
    
    // Check admin privileges
    if (!Utils::IsUserAdmin()) {
        std::cout << "[ERROR] This application must be run as administrator!" << std::endl;
        std::cout << "Please right-click and select 'Run as administrator'" << std::endl;
        system("pause");
        return 1;
    }
    
    Logger::Info("Loader started successfully");
    
    try {
        // Wait for Rust process
        if (!WaitForRustProcess()) {
            Logger::Error("Failed to find Rust process");
            std::cout << "[ERROR] Could not find Rust process!" << std::endl;
            system("pause");
            return 1;
        }
        
        // Small delay to ensure process is fully loaded
        std::cout << "[INFO] Waiting for process to stabilize..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        
        // Inject DLL
        if (!InjectDLL()) {
            Logger::Error("Failed to inject DLL");
            std::cout << "[ERROR] Injection failed!" << std::endl;
            std::cout << "Make sure:" << std::endl;
            std::cout << "- You are running as administrator" << std::endl;
            std::cout << "- Antivirus is disabled" << std::endl;
            std::cout << "- rust-tool.dll is in the same folder" << std::endl;
            system("pause");
            return 1;
        }
        
        Logger::Info("DLL injected successfully");
        ShowUsageInstructions();
        
        std::cout << "[SUCCESS] Tool loaded successfully!" << std::endl;
        std::cout << "You can now minimize this window." << std::endl;
        std::cout << std::endl;
        std::cout << "Press ENTER to exit..." << std::endl;
        std::cin.get();
        
    } catch (const std::exception& e) {
        Logger::Error("Exception occurred: " + std::string(e.what()));
        std::cout << "[ERROR] An error occurred: " << e.what() << std::endl;
        system("pause");
        return 1;
    }
    
    Logger::Info("Loader exiting");
    Logger::Shutdown();
    return 0;
}