#pragma once
#include <cstdint>
#include <array>
#include "error.hpp"

namespace pci
{
    const uint16_t kConfigAddress = 0x0cf8;
    const uint16_t kConfigData = 0x0cfc;

    struct ClassCode
    {
        uint8_t base, sub, interface;
        bool Match(uint8_t b)
        {
            return b == base;
        }
        bool Match(uint8_t b, uint8_t s)
        {
            return Match(b) && (s == sub);
        }
        bool Match(uint8_t b, uint8_t s, uint8_t i)
        {
            return Match(b, s) && (i == interface);
        }
    };
    struct Device
    {
        uint8_t bus, device, function, header_type;
        ClassCode class_code;
    };

    void WriteAddress(uint32_t address);
    void WriteData(uint32_t data);
    uint32_t ReadData();
    uint32_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function);
    ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function);
    uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function);
    uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function);
    bool IsSingleFunctionDevice(uint32_t header_type);

    inline uint16_t ReadVendorId(const Device &dev)
    {
        return ReadVendorId(dev.bus, dev.device, dev.function);
    };

    inline std::array<Device, 32> devices;
    inline int num_device;
    Error ScanAllBus();

    constexpr uint8_t CalcBarAddress(unsigned int bar_index)
    {
        return 0x10u + 4 * bar_index;
    }

    uint32_t ReadConfReg(const Device &device, uint32_t addr);
    void WriteConfReg(const Device &dev, uint8_t reg_addr, uint32_t value);

    WithError<uint64_t> ReadBar(Device &device, unsigned int bar_index);
}