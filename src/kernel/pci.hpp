#pragma once
#include <cstdint>
#include <array>
#include "error.hpp"

namespace pci
{
    const uint16_t kConfigAddress = 0x0cf8;
    const uint16_t kConfigData = 0x0cfc;

    void WriteAddress(uint32_t address);
    void WriteData(uint32_t data);
    uint32_t ReadData();
    uint32_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function);
    uint32_t ReadClassCode(uint8_t bus, uint8_t device, uint8_t function);
    uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function);
    uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function);
    bool IsSingleFunctionDevice(uint32_t header_type);

    struct Device
    {
        uint8_t bus, device, function, header_type;
    };

    inline std::array<Device, 32> devices;
    inline int num_device;
    Error ScanAllBus();
}