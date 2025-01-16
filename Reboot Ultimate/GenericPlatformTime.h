#pragma once

#include "inc.h"

struct FGenericPlatformTime
{
    static FORCEINLINE uint32 Cycles()
    {
        FILETIME ft;
        ULARGE_INTEGER uli{};
        uint64 seconds, microseconds;

        GetSystemTimeAsFileTime(&ft);  // Get current time
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;

        // Convert to microseconds
        uli.QuadPart /= 10;
        uli.QuadPart -= 11644473600000000ULL;

        seconds = uli.QuadPart / 1000000;
        microseconds = uli.QuadPart % 1000000;

        return (uint32)((seconds * 1000000ULL) + microseconds);
    }
};