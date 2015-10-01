#pragma once

#include <core/core.h>

#include <cstdint>
#include <vector>

class CORE_API GraphImage
{
public:
	GraphImage() = default;
	GraphImage(int x, int y);

	using Color = uint32_t;
	using ColorsList = std::vector<Color>;

	static GraphImage createDiskImage(Color color); // With a standard size (16x16)
	static GraphImage createSquaresImage(const ColorsList& colors); // Adapt the width to the number of colors

	void disk(Color color);
	void squares(const ColorsList& colors, int squareSize = 10);

	void fill(int x0, int x1, int y0, int y1, Color color);
	void setPixel(int x, int y, Color color); // Warning: no bounds check!

	int width() const;
	int height() const;
	const unsigned char* data() const;

protected:
	int m_width = 0, m_height = 0;
	std::vector<unsigned char> m_data; // ARGB32
};

namespace ImageHelper
{

uint32_t rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void fromRgba(uint32_t c, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a);
uint32_t interpolateColor(uint32_t c1, uint32_t c2, float amt);
float smoothstep(float val, float x0, float x1);

}

//****************************************************************************//

inline void GraphImage::setPixel(int x, int y, uint32_t color)
{ reinterpret_cast<uint32_t*>(&m_data[0])[y * m_width + x] = color; }

inline int GraphImage::width() const
{ return m_width; }

inline int GraphImage::height() const
{ return m_height; }

inline const unsigned char* GraphImage::data() const
{
	if (m_data.empty()) return nullptr;
	return &m_data[0];
}

namespace ImageHelper
{

inline uint32_t rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{ return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

inline void fromRgba(uint32_t c, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a)
{ a = c >> 24; r = c >> 16; g = c >> 8; b = c; }

}