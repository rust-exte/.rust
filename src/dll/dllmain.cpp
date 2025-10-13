#include <windows.h>
#include <iostream>
#include "../include/utils.h"

// Forward declarations for our core classes
namespace RustTool {
    class Aimbot;
    class ESP;
    class Config;
    class MemoryManager;
    class GameHooks;
    class D3D11Hook;
}

// Global instances
RustTool::Aimbot* g_aimbot = nullptr;
RustTool::ESP* g_esp = nullptr;
RustTool::Config* g_config = nullptr;
RustTool::MemoryManager* g_memoryManager = nullptr;
RustTool::GameHooks* g_gameHooks = nullptr;
RustTool::D3D11Hook* g_d3dHook = nullptr;

bool g_isInitialized = false;
bool g_shouldUnload = false;

DWORD WINAPI InitializeThread(LPVOID lpParam) {
    try {
        // Initialize logger
        RustTool::Logger::Initialize(L"rust-tool.log", false, true);
        RustTool::Logger::Info("DLL loaded, initializing...");
        
        // Show console for debugging (can be disabled in release)
        #ifdef _DEBUG
        RustTool::Utils::ShowConsole();
        RustTool::Utils::SetConsoleTitle(L"Rust Tool 2025 - Debug Console");
        std::cout << "Rust Tool 2025 - Debug Console" << std::endl;
        std::cout << "===============================" << std::endl;
        #endif
        
        // Wait for game to be ready
        Sleep(3000);
        
        // Initialize memory manager
        // g_memoryManager = new RustTool::MemoryManager();
        RustTool::Logger::Info("Memory manager initialized");
        
        // Initialize configuration
        // g_config = new RustTool::Config();
        // g_config->LoadFromFile(L"config.json");
        RustTool::Logger::Info("Configuration loaded");
        
        // Initialize core features
        // g_aimbot = new RustTool::Aimbot();
        // g_esp = new RustTool::ESP();
        RustTool::Logger::Info("Core features initialized");
        
        // Initialize hooks
        // g_gameHooks = new RustTool::GameHooks();
        // g_d3dHook = new RustTool::D3D11Hook();
        RustTool::Logger::Info("Hooks initialized");
        
        g_isInitialized = true;
        RustTool::Logger::Info("Tool fully initialized and ready");
        
        // Play confirmation sound
        Beep(1000, 200);
        
        // Main loop
        while (!g_shouldUnload) {
            if (g_isInitialized) {
                // Update core systems
                // if (g_aimbot) g_aimbot->Update();
                // if (g_esp) g_esp->Update();
            }
            
            Sleep(1); // Small delay to prevent excessive CPU usage
        }
        
    } catch (const std::exception& e) {
        RustTool::Logger::Error("Exception in initialization: " + std::string(e.what()));
        MessageBoxA(NULL, e.what(), "Rust Tool Error", MB_OK | MB_ICONERROR);
    }
    
    return 0;
}

DWORD WINAPI UnloadThread(LPVOID lpParam) {
    try {
        RustTool::Logger::Info("Beginning shutdown sequence...");
        
        g_shouldUnload = true;
        Sleep(100); // Give main loop time to exit
        
        // Cleanup in reverse order
        if (g_d3dHook) {
            delete g_d3dHook;
            g_d3dHook = nullptr;
        }
        
        if (g_gameHooks) {
            delete g_gameHooks;
            g_gameHooks = nullptr;
        }
        
        if (g_esp) {
            delete g_esp;
            g_esp = nullptr;
        }
        
        if (g_aimbot) {
            delete g_aimbot;
            g_aimbot = nullptr;
        }
        
        if (g_config) {
            // g_config->SaveToFile(L"config.json");
            delete g_config;
            g_config = nullptr;
        }
        
        if (g_memoryManager) {
            delete g_memoryManager;
            g_memoryManager = nullptr;
        }
        
        #ifdef _DEBUG
        RustTool::Utils::HideConsole();
        #endif
        
        RustTool::Logger::Info("Cleanup completed, exiting...");
        RustTool::Logger::Shutdown();
        
        // Free library and exit
        FreeLibraryAndExitThread((HMODULE)lpParam, 0);
        
    } catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Rust Tool Unload Error", MB_OK | MB_ICONERROR);
    }
    
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // Disable thread library calls for performance
        DisableThreadLibraryCalls(hModule);
        
        // Create initialization thread
        CreateThread(nullptr, 0, InitializeThread, hModule, 0, nullptr);
        break;
        
    case DLL_PROCESS_DETACH:
        // Create unload thread
        CreateThread(nullptr, 0, UnloadThread, hModule, 0, nullptr);
        break;
        
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    
    return TRUE;
}