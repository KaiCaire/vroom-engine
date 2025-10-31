#include "Log.h"
#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <mutex>

static std::vector<std::string> g_LogBuffer;
static std::mutex g_LogMutex;

void Log(const char file[], int line, const char* format, ...)
{
    static char tmpString1[4096];
    static va_list ap;

    // Construct the string from variable arguments
    va_start(ap, format);
    vsnprintf(tmpString1, 4096, format, ap);
    va_end(ap);

    // Construct the final log message
    std::string logMessage = std::string("\n") + file + "(" + std::to_string(line) + ") : " + tmpString1;

    // Print the formatted string to the standard error stream
    std::cerr << logMessage << std::endl;

    //store logs in memory for imGui console
    //thread safe block - only one thread at a time
    {
        //threads wait until mutex is unlocked to add logs
        std::lock_guard<std::mutex> lock(g_LogMutex);
        g_LogBuffer.emplace_back(logMessage);
    }
}

std::vector<std::string> GetLogBuffer() {
    //getting logs for imGui console
    std::lock_guard<std::mutex> lock(g_LogMutex);
    return g_LogBuffer;
}

void ClearLogs() {
    std::lock_guard<std::mutex> lock(g_LogMutex);
    g_LogBuffer.clear();
}