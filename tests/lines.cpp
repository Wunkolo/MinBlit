#include <cstddef>
#include <cstdint>
#include <functional>

#include <MinBlit.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char* argv[])
{
	MinBlit::SurfaceRGBA8888 Screen(512, 512);
	Screen.Fill(0xFF'30'30'30);

	constexpr std::size_t Spacing = 64;

	for( std::size_t Y = 0; Y < Screen.GetHeight() / Spacing; ++Y )
	{
		for( std::size_t X = 0; X < Screen.GetWidth() / Spacing; ++X )
		{
			Screen.Line(
				MinBlit::PointSize(0, Y * Spacing),
				MinBlit::PointSize(X * Spacing, 0),
				std::hash<std::size_t>{}(X + Y) | 0xFF'00'00'00
			);
		}
	}

	const MinBlit::SizeT Patterns[] = {
		0b01'01'01'01'01'01'01'01'01'01'01'01'01'01'01'01,
		0b1100'1100'1100'1100'1100'1100'1100'1100,
		0b111000'111000'111000'111000'111000'111000,
		0b11110000'11110000'11110000'11110000,
		0b1111111100000000'1111111100000000
	};

	for( std::size_t i = 0; i < sizeof(Patterns) / sizeof(MinBlit::SizeT); i++ )
	{
		Screen.LineStipple(
			MinBlit::PointSize(Screen.GetWidth() / 2, Screen.GetHeight() / 2 + i * Spacing),
			MinBlit::PointSize(Screen.GetWidth(), Screen.GetHeight() / 2 + i * Spacing),
			std::hash<std::size_t>{}(Patterns[i]) | 0xFF'00'00'00,
			Patterns[i]
		);
	}

	stbi_write_png(
		"Lines.png",
		Screen.GetWidth(),
		Screen.GetHeight(),
		4,
		Screen.GetPixels(),
		0
	);

	return EXIT_SUCCESS;
}
