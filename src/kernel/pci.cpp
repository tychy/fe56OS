#include "pci.hpp"
#include "asmfunc.hpp"
#include "error.hpp"

namespace
{
    using namespace pci;

    /** @brief CONFIG_ADDRESS 用の３２ビット整数を作成する */
    uint32_t MakeAddress(uint8_t bus, uint8_t device,
                         uint8_t function, uint8_t reg_addr)
    {
        auto shl = [](uint32_t x, unsigned int bits)
        {
            return x << bits;
        };
        return shl(1, 31) // enable bit
               | shl(bus, 16) | shl(device, 11) | shl(function, 8) | (reg_addr & 0xfcu);
    }

    Error AddDevice(Device dev)
    {
        if (num_device == devices.size())
        {
            return MAKE_ERROR(Error::kFull);
        }
        devices[num_device] = dev;
        ++num_device;
        return MAKE_ERROR(Error::kSuccess);
    }

    Error ScanBus(uint8_t bus);

    Error ScanFunction(uint8_t bus, uint8_t device, uint8_t function)
    {
        ClassCode class_code = ReadClassCode(bus, device, function);
        auto header_type = ReadHeaderType(bus, device, function);
        Device dev{bus, device, function, header_type, class_code};
        if (auto err = AddDevice(dev))
        {
            return err;
        }

        if (class_code.Match(0x06u, 0x04u))
        {
            // PCI- PCI bridge
            auto bus_numbers = ReadBusNumbers(bus, device, function);
            uint8_t secondary_bus = (bus_numbers >> 8) & 0xffu;
            return ScanBus(secondary_bus);
        }

        return MAKE_ERROR(Error::kSuccess);
    }

    Error ScanDevice(uint32_t bus, uint32_t device)
    {
        if (auto err = ScanFunction(bus, device, 0))
        {
            return err;
        }
        if (IsSingleFunctionDevice(ReadHeaderType(bus, device, 0)))
        {
            return MAKE_ERROR(Error::kSuccess);
        }
        for (uint8_t function = 1; function < 8; function++)
        {
            if (ReadVendorId(bus, device, function) == 0xffffu)
            {
                continue;
            }
            if (auto err = ScanFunction(bus, device, function))
            {
                return err;
            }
        }

        return MAKE_ERROR(Error::kSuccess);
    }
    Error ScanBus(uint8_t bus)
    {
        for (uint8_t device = 0; device < 32; device++)
        {
            if (ReadVendorId(bus, device, 0) == 0xffffu)
            {
                continue;
            }
            if (auto err = ScanDevice(bus, device))
            {
                return err;
            }
        }
        return MAKE_ERROR(Error::kSuccess);
    }
}

namespace pci
{
    void WriteAddress(uint32_t address)
    {
        IoOut32(kConfigAddress, address);
    }

    void WriteData(uint32_t data)
    {
        IoOut32(kConfigData, data);
    }

    uint32_t ReadData()
    {
        return IoIn32(kConfigData);
    }

    uint32_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function)
    {
        WriteAddress(MakeAddress(bus, device, function, 0x00));
        return ReadData() & 0xffffu;
    }

    ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function)
    {
        WriteAddress(MakeAddress(bus, device, function, 0x08));
        auto class_code = ReadData();
        ClassCode cc;
        cc.base = (class_code >> 24) & 0xffu;
        cc.sub = (class_code >> 16) & 0xffu;
        cc.interface = (class_code >> 8) & 0xffu;
        return cc;
    }

    uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function)
    {
        WriteAddress(MakeAddress(bus, device, function, 0x0c));
        return (ReadData() > 16) & 0xffu;
    }

    uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function)
    {
        WriteAddress(MakeAddress(bus, device, function, 0x18));
        return ReadData();
    }
    bool IsSingleFunctionDevice(uint32_t header_type)
    {
        return (header_type & 0x80u) == 0;
    }

    Error ScanAllBus()
    {
        num_device = 0;

        auto header_type = ReadHeaderType(0, 0, 0);
        if (IsSingleFunctionDevice(header_type))
        {
            return ScanBus(0);
        }
        for (uint8_t function = 0; function < 8; function++)
        {
            if (ReadVendorId(0, 0, 0) == 0xffffu)
            {
                continue;
            }
            if (auto err = ScanBus(function))
            {
                return err;
            }
        }
        return MAKE_ERROR(Error::kSuccess);
    }

    uint32_t ReadConfReg(const Device &device, uint32_t addr)
    {
        WriteAddress(MakeAddress(device.bus, device.device, device.function, addr));
        return ReadData();
    }
    void WriteConfReg(const Device &dev, uint8_t reg_addr, uint32_t value)
    {
        WriteAddress(MakeAddress(dev.bus, dev.device, dev.function, reg_addr));
        WriteData(value);
    }

    WithError<uint64_t> ReadBar(Device &device, unsigned int bar_index)
    {
        if (bar_index >= 6)
        {
            return {0, MAKE_ERROR(Error::kIndexOutOfRange)};
        }

        const auto addr = CalcBarAddress(bar_index);
        const auto bar = ReadConfReg(device, addr);

        // 32 bit address
        if ((bar & 4u) == 0)
        {
            return {bar, MAKE_ERROR(Error::kSuccess)};
        }

        // 64 bit address
        if (bar_index >= 5)
        {
            return {0, MAKE_ERROR(Error::kIndexOutOfRange)};
        }

        const auto bar_upper = ReadConfReg(device, addr + 4);
        return {
            bar | (static_cast<uint64_t>(bar_upper) << 32),
            MAKE_ERROR(Error::kSuccess)};
    }
}
