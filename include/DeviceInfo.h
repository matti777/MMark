#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <string>

/** Information about the device/platform. */
struct DeviceInfo
{
    std::string m_manufacturer; // "Apple"
    std::string m_model; // "iPhone", "iPhone 4S"
    std::string m_productName; // "4S" or just empty
    std::string m_osVersion; // "5.0"
    std::string m_deviceType; // "mobilephone", "minitablet", "tablet", "other"
    int m_totalRam; // in kB
    std::string m_cpuType;
    int m_numCpuCores;
    int m_cpuFrequency; // in MHz
};

#endif // DEVICEINFO_H
