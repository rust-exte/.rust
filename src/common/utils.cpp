#include "../../include/utils.h"
#include <shlwapi.h>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <random>
#include <ctime>

#pragma comment(lib, "shlwapi.lib")

namespace RustTool {

// Static member definitions
std::wstring Logger::m_logFilePath = L"";
bool Logger::m_consoleOutput = true;
bool Logger::m_fileOutput = true;

// Utils implementation
std::wstring Utils::StringToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], sizeNeeded);
    return wstrTo;
}

std::string Utils::WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
    return strTo;
}

std::string Utils::ToLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string Utils::ToUpperCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool Utils::Contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

bool Utils::FileExists(const std::wstring& path) {
    return PathFileExistsW(path.c_str()) == TRUE;
}

std::string Utils::ReadFileToString(const std::wstring& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();
    
    return contents.str();
}

bool Utils::WriteStringToFile(const std::wstring& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    file.close();
    
    return true;
}

std::wstring Utils::GetExecutablePath() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return std::wstring(path);
}

std::wstring Utils::GetExecutableDirectory() {
    std::wstring path = GetExecutablePath();
    size_t lastSlash = path.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        return path.substr(0, lastSlash);
    }
    return path;
}

float Utils::DegreesToRadians(float degrees) {
    return degrees * (3.14159265359f / 180.0f);
}

float Utils::RadiansToDegrees(float radians) {
    return radians * (180.0f / 3.14159265359f);
}

float Utils::Clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float Utils::Lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float Utils::Distance2D(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

float Utils::Distance3D(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return sqrt(dx * dx + dy * dy + dz * dz);
}

void Utils::ShowConsole() {
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
}

void Utils::HideConsole() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    FreeConsole();
}

void Utils::SetConsoleTitle(const std::wstring& title) {
    SetConsoleTitleW(title.c_str());
}

DWORD Utils::GetCurrentProcessId() {
    return ::GetCurrentProcessId();
}

bool Utils::IsUserAdmin() {
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID administratorsGroup;
    
    BOOL isAdmin = AllocateAndInitializeSid(
        &ntAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &administratorsGroup
    );
    
    if (isAdmin) {
        CheckTokenMembership(NULL, administratorsGroup, &isAdmin);
        FreeSid(administratorsGroup);
    }
    
    return isAdmin == TRUE;
}

void Utils::CreateBeep(DWORD frequency, DWORD duration) {
    Beep(frequency, duration);
}

int Utils::RandomInt(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

float Utils::RandomFloat(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

void Utils::SeedRandom() {
    srand(static_cast<unsigned int>(time(nullptr)));
}

DWORD Utils::GetCurrentTimeMs() {
    return GetTickCount();
}

std::string Utils::GetCurrentTimeString() {
    time_t rawtime;
    struct tm timeinfo;
    char buffer[80];
    
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buffer);
}

void Utils::Sleep(DWORD milliseconds) {
    ::Sleep(milliseconds);
}

// Logger implementation
void Logger::Initialize(const std::wstring& logPath, bool console, bool file) {
    m_consoleOutput = console;
    m_fileOutput = file;
    
    if (file && !logPath.empty()) {
        m_logFilePath = logPath;
    } else if (file) {
        m_logFilePath = Utils::GetExecutableDirectory() + L"\\rust-tool.log";
    }
    
    if (m_fileOutput && !m_logFilePath.empty()) {
        // Clear existing log file
        std::wofstream logFile(m_logFilePath, std::ios::trunc);
        if (logFile.is_open()) {
            logFile << L"Rust Tool 2025 - Log Started at " << Utils::StringToWString(Utils::GetCurrentTimeString()) << std::endl;
            logFile << L"================================================" << std::endl;
            logFile.close();
        }
    }
}

void Logger::Log(LogLevel level, const std::string& message) {
    std::string timestamp = Utils::GetCurrentTimeString();
    std::string levelStr = LogLevelToString(level);
    std::string fullMessage = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    if (m_consoleOutput) {
        WriteToConsole(fullMessage);
    }
    
    if (m_fileOutput) {
        WriteToFile(fullMessage);
    }
}

void Logger::Info(const std::string& message) {
    Log(LOG_INFO, message);
}

void Logger::Warning(const std::string& message) {
    Log(LOG_WARNING, message);
}

void Logger::Error(const std::string& message) {
    Log(LOG_ERROR, message);
}

void Logger::Debug(const std::string& message) {
#ifdef _DEBUG
    Log(LOG_DEBUG, message);
#endif
}

void Logger::Shutdown() {
    if (m_fileOutput && !m_logFilePath.empty()) {
        std::wofstream logFile(m_logFilePath, std::ios::app);
        if (logFile.is_open()) {
            logFile << L"================================================" << std::endl;
            logFile << L"Log Ended at " << Utils::StringToWString(Utils::GetCurrentTimeString()) << std::endl;
            logFile.close();
        }
    }
}

std::string Logger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARN";
        case LOG_ERROR: return "ERROR";
        case LOG_DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

void Logger::WriteToFile(const std::string& message) {
    if (m_logFilePath.empty()) return;
    
    std::wofstream logFile(m_logFilePath, std::ios::app);
    if (logFile.is_open()) {
        logFile << Utils::StringToWString(message) << std::endl;
        logFile.close();
    }
}

void Logger::WriteToConsole(const std::string& message) {
    std::cout << message << std::endl;
}

} // namespace RustTool