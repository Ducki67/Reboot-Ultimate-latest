#include "HardwareUtils.h"

#ifdef _WIN32
#include <Windows.h>
#include <iostream>
#include <sstream>

std::string HardwareUtils::GetHardwareID()
{
    DWORD volumeSerialNumber;
    if (GetVolumeInformationA("C:\\", NULL, 0, &volumeSerialNumber, NULL, NULL, NULL, 0))
    {
        std::stringstream ss;
        ss << std::hex << volumeSerialNumber;
        return ss.str();
    }
    return "Unknown_HWID";
}
#elif __linux__
#include <fstream>

std::string HardwareUtils::GetHardwareID()
{
    std::ifstream file("/etc/machine-id");
    std::string hwid;
    if (file.is_open())
    {
        std::getline(file, hwid);
        file.close();
    }
    return hwid.empty() ? "Unknown_HWID" : hwid;
}
#elif __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOPlatformExpert.h>

std::string HardwareUtils::GetHardwareID()
{
    io_service_t platformExpert = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching("IOPlatformExpertDevice"));
    if (!platformExpert) return "Unknown_HWID";

    CFStringRef serialNumber = (CFStringRef)IORegistryEntryCreateCFProperty(platformExpert, CFSTR("IOPlatformSerialNumber"), kCFAllocatorDefault, 0);
    IOObjectRelease(platformExpert);

    if (!serialNumber) return "Unknown_HWID";

    char buffer[256];
    CFStringGetCString(serialNumber, buffer, sizeof(buffer), kCFStringEncodingUTF8);
    CFRelease(serialNumber);

    return std::string(buffer);
}
#endif