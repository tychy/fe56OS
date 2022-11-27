#include <cstdint>
#include <cstdio>
#include <stddef.h>
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "console.hpp"
#include "pci.hpp"
#include "usb/xhci/xhci.hpp"

const PixelColor kDesktopBGColor{45, 118, 237};
const PixelColor kDesktopFGColor{255, 255, 255};

const int kMouseCursorWidth = 15;
const int kMouseCursorHeight = 24;
const char mouse_cursor_shape[kMouseCursorHeight][kMouseCursorWidth + 1] = {
    "@              ",
    "@@             ",
    "@.@            ",
    "@..@           ",
    "@...@          ",
    "@....@         ",
    "@.....@        ",
    "@......@       ",
    "@.......@      ",
    "@........@     ",
    "@.........@    ",
    "@..........@   ",
    "@...........@  ",
    "@............@ ",
    "@......@@@@@@@@",
    "@......@       ",
    "@....@@.@      ",
    "@...@ @.@      ",
    "@..@   @.@     ",
    "@.@    @.@     ",
    "@@      @.@    ",
    "@       @.@    ",
    "         @.@   ",
    "         @@@   ",
};

void operator delete(void *obj) noexcept
{
}

char console_buf[sizeof(Console)];
Console *console;

int printk(const char *format, ...)
{
  va_list ap;
  int result;
  char s[1024];
  va_start(ap, format);
  result = vsprintf(s, format, ap);
  va_end(ap);
  console->PutString(s);
  return result;
}

void SwitchEhci2Xhc(const pci::Device &xhc_dev)
{
  bool intel_ehc_exist = false;
  for (int i = 0; i < pci::num_device; i++)
  {
    if (pci::devices[i].class_code.Match(0x0cu, 0x03u, 0x20u) && // EHCI
        0x8086 == pci::ReadVendorId(pci::devices[i]))
    {
      intel_ehc_exist = true;
      break;
    }
  }
  if (!intel_ehc_exist)
  {
    return;
  }
  uint32_t supersped_ports = pci::ReadConfReg(xhc_dev, 0xdc); // USB3PRM
  pci::WriteConfReg(xhc_dev, 0xd8, supersped_ports);          // USB3_PSSEN
  uint32_t ehci2xhci_ports = pci::ReadConfReg(xhc_dev, 0xd4); // XUSB2PRM
  pci::WriteConfReg(xhc_dev, 0xd0, ehci2xhci_ports);          // XUSB2PR
  printk("SwitchEhci2Xhci: SS = %02x, xHCI = %02x\n", supersped_ports, ehci2xhci_ports);
}

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter *pixel_writer;

extern "C" void
KernelMain(const struct FrameBufferConfig &frame_buffer_config)
{
  switch (frame_buffer_config.pixel_format)
  {
  case kPixelRGBResv8BitPerColor:
    pixel_writer = new (pixel_writer_buf)
        RGBResv8BitPerColorPixelWriter{frame_buffer_config};
    break;
  case kPixelBGRResv8BitPerColor:
    pixel_writer = new (pixel_writer_buf)
        BGRResv8BitPerColorPixelWriter{frame_buffer_config};
    break;
  }

  for (int x = 0; x < frame_buffer_config.horizontal_resolution; x++)
  {
    for (int y = 0; y < frame_buffer_config.vertical_resolution; y++)
    {
      pixel_writer->Write(x, y, {255, 255, 255});
    }
  }
  const int kFrameWidth = frame_buffer_config.horizontal_resolution;
  const int kFrameHeight = frame_buffer_config.vertical_resolution;

  FillRectangle(*pixel_writer,
                {0, 0},
                {kFrameWidth, kFrameHeight - 50},
                kDesktopBGColor);
  FillRectangle(*pixel_writer,
                {0, kFrameHeight - 50},
                {kFrameWidth, 50},
                {1, 8, 17});
  FillRectangle(*pixel_writer,
                {0, kFrameHeight - 50},
                {kFrameWidth / 5, 50},
                {80, 80, 80});
  DrawRectangle(*pixel_writer,
                {10, kFrameHeight - 40},
                {30, 30},
                {160, 160, 160});
  console = new (console_buf) Console{
      *pixel_writer, kDesktopFGColor, kDesktopBGColor};
  printk("Welcome to MikanOS!\n");
  for (int y = 0; y < kMouseCursorHeight; y++)
  {
    for (int x = 0; x < kMouseCursorWidth; x++)
    {
      if (mouse_cursor_shape[y][x] == '@')
      {
        pixel_writer->Write(x + 200, y + 100, {0, 0, 0});
      }
      else if (mouse_cursor_shape[y][x] == '.')
      {
        pixel_writer->Write(x + 200, y + 100, {255, 255, 255});
      }
    }
  }

  auto err = pci::ScanAllBus();
  printk("ScanAllBus: %s\n", err.Name());

  for (int i = 0; i < pci::num_device; i++)
  {
    const auto &dev = pci::devices[i];
    auto vendor_id = pci::ReadVendorId(dev.bus, dev.device, dev.function);
    auto class_code = pci::ReadClassCode(dev.bus, dev.device, dev.function);
    printk("%d.%d.%d: vend %04x, class %08x, head %02x\n",
           dev.bus, dev.device, dev.function, vendor_id, class_code, dev.header_type);
  }
  pci::Device *xhc_dev = nullptr;
  for (int i = 0; i < pci::num_device; i++)
  {
    if (pci::devices[i].class_code.Match(0x0cu, 0x03u, 0x30u))
    {
      xhc_dev = &pci::devices[i];
      if (0x8086 == pci::ReadVendorId(*xhc_dev))
      {
        break;
      }
    }
  }
  if (xhc_dev)
  {
    printk("xHC has been found: %d.%d.%d", xhc_dev->bus, xhc_dev->device, xhc_dev->function);
  }

  const WithError<uint64_t> xhc_bar = pci::ReadBar(*xhc_dev, 0);
  printk("ReadBar: %s\n", xhc_bar.error.Name());
  const uint64_t xhc_mmio_base = xhc_bar.value & ~static_cast<uint64_t>(0xf);
  printk("xHC mmio_base = %08lx\n", xhc_mmio_base);

  usb::xhci::Controller xhc{xhc_mmio_base};
  if (0x8086 == pci::ReadVendorId(*xhc_dev))
  {
    SwitchEhci2Xhc(*xhc_dev);
  }
  {
    auto err = xhc.Initialize();
    printk("xhc.Initialize: %s\n", err.Name());
  }

  printk("xHC starting\n");
  xhc.Run();

  while (1)
    __asm__("hlt");
}

extern "C" void __cxa_pure_virtual()
{
  while (1)
    __asm__("hlt");
}