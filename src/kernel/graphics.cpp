#include "graphics.hpp"

void RGBResv8BitPerColorPixelWriter::Write(int x, int y, const PixelColor &c)
{
    auto p = PixelAt(x, y);
    p[0] = c.r;
    p[1] = c.g;
    p[2] = c.b;
}

void BGRResv8BitPerColorPixelWriter::Write(int x, int y, const PixelColor &c)
{
    auto p = PixelAt(x, y);
    p[0] = c.b;
    p[1] = c.g;
    p[2] = c.r;
}

void FillRectangle(PixelWriter &writer, const Vector2D<int> &pos, const Vector2D<int> size, const PixelColor &c)
{
    for (int y = 0; y < size.y; y++)
    {
        for (int x = 0; x < size.x; x++)
        {
            writer.Write(pos.x + x, pos.y + y, c);
        }
    }
}

void DrawRectangle(PixelWriter &writer, const Vector2D<int> &pos, const Vector2D<int> size, const PixelColor &c)
{
    for (int x = 0; x < size.x; x++)
    {
        writer.Write(pos.x + x, pos.y, c);
        writer.Write(pos.x + x, pos.y + size.y - 1, c);
    }

    for (int y = 1; y < size.y - 1; y++)
    {
        writer.Write(pos.x, pos.y + y, c);
        writer.Write(pos.x + size.x - 1, pos.y + y, c);
    }
}