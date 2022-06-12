#include <cstdint>
#include "frame_buffer_config.hpp"
struct PixelColor
{
  uint8_t r, g, b;
};

/** WritePixel write one pixel
 * @retval 0 succeed
 * @retval non-0 failed
 */
int WritePixel(const struct FrameBufferConfig &config,
               int x, int y, const PixelColor &c)
{
  const int pixel_position = config.pixels_per_scan_line * y + x;
  if (config.pixel_format == kPixelRGBResv8BitPerColor)
  {
    uint8_t *p = &config.frame_buffer[pixel_position * 4];
    p[0] = c.r;
    p[1] = c.g;
    p[2] = c.b;
  }
  else if (config.pixel_format == kPixelBGRResv8BitPerColor)
  {
    uint8_t *p = &config.frame_buffer[pixel_position * 4];
    p[0] = c.b;
    p[1] = c.g;
    p[2] = c.r;
  }
  else
  {
    return -1;
  }
  return 0;
}

extern "C" void KernelMain(const struct FrameBufferConfig &frame_buffer_config)
{
  for (int x = 0; x < frame_buffer_config.horizontal_resolution; x++)
  {
    for (int y = 0; y < frame_buffer_config.vertical_resolution; y++)
    {
      WritePixel(frame_buffer_config, x, y, {255, 255, 255});
    }
  }

  for (int x = 0; x < 100; x++)
  {
    for (int y = 0; y < 200; y++)
    {
      WritePixel(frame_buffer_config, x + 100, y + 100, {255, 0, 0});
    }
  }

  while (1)
    __asm__("hlt");
}
