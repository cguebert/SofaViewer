#include "GraphImage.h"

#include <algorithm>
#include <iostream>

namespace ImageHelper
{

uint32_t interpolateColor(uint32_t c1, uint32_t c2, float amt)
{
	amt = std::max(0.f, std::min(amt, 1.f));
	unsigned char f2 = static_cast<unsigned char>(255 * amt);
	unsigned char f1 = 255 - f2;
	uint32_t t = (c1 & 0xff00ff) * f1 + (c2 & 0xff00ff) * f2;
	t >>= 8;
	t &= 0xff00ff;

	c1 = ((c1 >> 8) & 0xff00ff) * f1 + ((c2 >> 8) & 0xff00ff) * f2;
	c1 &= 0xff00ff00;
	return (c1 | t);
}

float smoothstep(float val, float x0, float x1)
{
	if (val < x0)	return 0.f;
	if (val >= x1)	return 1.f;
	val = (val - x0) / (x1 - x0);
	return (val*val * (3 - 2 * val));
}

} // namespace ImageHelper

using namespace ImageHelper;

GraphImage::GraphImage(int w, int h)
	: m_width(w), m_height(h), m_data(w * h * 4, 0)
{ }

GraphImage GraphImage::createDiskImage(Color color)
{
	auto img = GraphImage(16, 16);
	img.disk(color);
	return img;
}

GraphImage GraphImage::createSquaresImage(const ColorsList& colors)
{
	const int squareSize = 10;
	auto img = GraphImage(squareSize * colors.size() + 2, squareSize + 2);
	img.squares(colors);
	return img;
}

void GraphImage::fill(int x0, int x1, int y0, int y1, Color color)
{
	for(int y = y0; y <= y1; ++y)
		for(int x = x0; x <= x1; ++x)
			setPixel(x, y, color);
}

void GraphImage::disk(Color color)
{
	const float cx = (m_width - 1) / 2.0f, cy = (m_height - 1) / 2.0f;
	const float d2 = std::min(cx, cy);
	const float d1 = d2 - 0.5f;
	const float d0 = std::max(0.f, d1 - 1.2f);
	const auto black = rgba(0, 0, 0, 255);
	const auto transparent = rgba(0, 0, 0, 0);

	for (int y = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x)
		{
			const float dx = x - cx, dy = y - cy;
			const float d = sqrt(dx*dx + dy*dy);

			if(d < d0) // Inside
				setPixel(x, y, color);
			else if(d > d2) // Outside
				setPixel(x, y, transparent);
			else if(d < d1) // From inside to the stroke
				setPixel(x, y,
						 interpolateColor(color, black, smoothstep(d, d0, d1))
						 );
			else if(d > d1) // From the stroke to outside
				setPixel(x, y,
						 interpolateColor(black, transparent, smoothstep(d, d1, d2))
						 );
		}
	}
}

void GraphImage::squares(const ColorsList& colors, int squareSize)
{
	const auto black = rgba(0, 0, 0, 255);
	fill(0, m_width - 1, 0, 0, black);
	fill(0, m_width - 1, m_height - 1, m_height - 1, black);
	fill(0, 0, 0, m_height - 1, black);
	fill(m_width - 1, m_width - 1, 0, m_height - 1, black);

	for(int i = 0, nb = colors.size(); i < nb; ++i)
		fill(1 + i * squareSize, (i + 1) * squareSize, 1, m_height - 2, colors[i]);
}
