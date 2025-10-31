#pragma once
#include <windows.h>
#include <psapi.h>
#include <thread>
#include <string>

float GetMemoryUsageMB() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.PrivateUsage / 1024.0f / 1024.0f;
    }
    return 0.0f;
}

static unsigned int GetCPUCoreCount()
{
    return std::thread::hardware_concurrency();
}