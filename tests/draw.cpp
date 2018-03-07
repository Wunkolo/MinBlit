#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <thread>

#include <MinBlit.hpp>

#define SG_DEFINE
#define SG_STATIC
#define SG_W32
#include "sg.hpp"

constexpr std::size_t Width = 1280 / 4;
constexpr std::size_t Height = 720 / 4;
constexpr std::size_t FrameRate = 60;
constexpr std::size_t Delay = 1000 / 60;
constexpr auto TestDuration = std::chrono::seconds(10);

int main(int argc, char* argv[])
{
	sg_init(
		"Draw Test",
		Width * 2, Height * 2
	);

	MinBlit::SurfaceRGB888 Screen(Width, Height);

	const std::chrono::high_resolution_clock::time_point Start = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point End;

	std::size_t Tick = 0;
	sg_event CurEvent;
	do
	{
		End = std::chrono::high_resolution_clock::now();

		const std::float_t Phase = ((End - Start) / std::chrono::duration<std::float_t>(TestDuration));
		if( sg_poll(&CurEvent)
			&& CurEvent.type == SG_ev_keydown
			&& CurEvent.key == SG_key_esc )
		{
			break;
		}

		///

		Screen.Circle(
			static_cast<MinBlit::SizeT>(Width * Phase),
			Height/2,
			30,
			MinBlit::PixelRGB888(0xFFFFFFFF)
		);

		Screen.LineStipple(
		{ 0,0 },
		{ static_cast<MinBlit::SizeT>(Width * Phase), Height/2 },
		MinBlit::PixelRGB888(0xFFFF00FF)
		);

		sg_paint(
			Screen.GetPixels(),
			Width, Height
		);
		Screen.Fill(0x00101010);
		///

		++Tick;
		printf(
			"%08zu: %08f\r",
			Tick,
			Tick / static_cast<std::float_t>(FrameRate)
		);
		std::this_thread::sleep_for(std::chrono::milliseconds(Delay));
	}
	while( End - Start < TestDuration );

	sg_exit();
	return EXIT_SUCCESS;
}
