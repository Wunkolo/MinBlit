#include <cstddef>
#include <cstdint>
#include <functional>

#include <MinBlit.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char* argv[])
{
	MinBlit::SurfaceRGBA8888 Screen(512,512);
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
