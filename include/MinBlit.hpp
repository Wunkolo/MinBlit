/*

MinBlit - Header only 2d blitting engine

COMPILING
	In just 1 of .cpp file that includes this library, add:
		#define MINBLIT_IMPLEMENTATION
	before your include. So at least one .cpp file in your project should have:
		#define MINBLIT_IMPLEMENTATION
		#include "MinBlit.hpp"
	Other headers and source files can include MinBlit.hpp as much as they want
	but just one of your source files requires #define MINBLIT_IMPLEMENTATION
	If you worry for build-times then you may create a MinBlit.cpp file with just:
		#define MINBLIT_IMPLEMENTATION
		#include "MinBlit.hpp"
	And all MinBlit implementations will be defined within this source file only.

LICENSE

	MIT License

	Copyright (c) 2018 Wunkolo

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <climits>
#include <type_traits>
#include <limits>
#include <memory>
#include <algorithm>

namespace MinBlit
{
// Typedefs to abstract underlaying types
using ScalarT = std::float_t;
static_assert(
	std::is_floating_point<ScalarT>::value,
	"BltScalar floating point type required"
);
static constexpr ScalarT ScalarMax = std::numeric_limits<ScalarT>::max();
static constexpr ScalarT ScalarMin = std::numeric_limits<ScalarT>::min();

using IntegerT = std::intmax_t;
static constexpr IntegerT IntegralMax = std::numeric_limits<IntegerT>::max();
static constexpr IntegerT IntegralMin = std::numeric_limits<IntegerT>::min();
static_assert(
	std::is_integral<IntegerT>::value,
	"BltInteger integral type required"
);

using SizeT = std::size_t;
static constexpr SizeT SizeMax = std::numeric_limits<SizeT>::max();
static constexpr SizeT SizeMin = std::numeric_limits<SizeT>::min();
static_assert(
	std::is_unsigned<SizeT>::value,
	"BltSize integral type required"
);

// Utility functions

template< typename T >
T Sign(T Value)
{
	static_assert(
		std::is_arithmetic<T>::value,
		"T must be an arithmetic type"
	);
	return (T(0) < Value) - (Value < T(0));
}

// Integer square-root
inline IntegerT isqrt(IntegerT x)
{
	IntegerT Odd(1);
	IntegerT Count(0);
	IntegerT Sum(0);
	while( Sum < x )
	{
		Count++;
		Sum += Odd;
		Odd += IntegerT(2);
	}
	return Count--;
}

template< typename ScalarType >
class Point
{
public:
	ScalarType X, Y;

	Point()
		: X(0),
		Y(0)
	{
	}

	Point(ScalarType X, ScalarType Y)
		: X(X),
		Y(Y)
	{
	}

	template< typename T >
	explicit operator Point<T>() const
	{
		return Point<T>(
			static_cast<T>(X),
			static_cast<T>(Y)
		);
	}

	bool operator==(const Point& Other) const
	{
		return ((X == Other.X) && (Y == Other.Y));
	}

	template< typename T >
	Point operator+(const Point<T>& Other) const
	{
		return Point(
			X + static_cast<ScalarType>(Other.X),
			Y + static_cast<ScalarType>(Other.Y)
		);
	}

	template< typename T >
	Point operator-(const Point<T>& Other) const
	{
		return Point(
			X - static_cast<ScalarType>(Other.X),
			Y - static_cast<ScalarType>(Other.Y)
		);
	}

	template< typename T >
	Point operator*(const Point<T>& Other) const
	{
		return Point(
			X * static_cast<ScalarType>(Other.X),
			Y * static_cast<ScalarType>(Other.Y)
		);
	}

	template< typename T >
	Point operator/(const Point<T>& Other) const
	{
		return Point(
			X / static_cast<ScalarType>(Other.X),
			Y / static_cast<ScalarType>(Other.Y)
		);
	}

	template< typename T >
	Point operator*(const T Value) const
	{
		return Point(
			X * Value,
			Y * Value
		);
	}

	template< typename T >
	Point operator/(const T Value) const
	{
		return Point(
			X / Value,
			Y / Value
		);
	}

	ScalarT Length() const
	{
		return std::sqrt(
			static_cast<ScalarT>(
				X * X + Y * Y
			)
		);
	}

	ScalarT Dot(const Point& Other) const
	{
		return X * Other.X + Y * Other.Y;
	}
};

using PointSize = Point<SizeT>;
using PointInt = Point<IntegerT>;
using PointScalar = Point<ScalarT>;

template< typename ScalarType >
class Rect
{
public:
	Point<ScalarType> Center, HalfDimensions;

	Rect()
		: Center(0, 0),
		HalfDimensions(0, 0)
	{
	}

	Rect(
		const Point<ScalarType>& Center,
		const Point<ScalarType>& HalfDimensions
	)
		: Center(Center),
		HalfDimensions(HalfDimensions)
	{
	}

	Rect(
		ScalarType CenterX,
		ScalarType CenterY,
		ScalarType HalfWidth,
		ScalarType HalfHeight
	)
		: Center(CenterX, CenterY),
		HalfDimensions(HalfWidth, HalfHeight)
	{
	}

	bool Contains(const Point<ScalarType>& Point) const
	{
		Point<ScalarType> Dist = Center - Point;
		if( Abs(Dist.X) == Abs(HalfDimensions.X) )
		{
			if( Abs(Dist.Y) <= Abs(HalfDimensions.Y) )
			{
				return true;
			}
		}
		return false;
	}
};

using RectSize = Rect<SizeT>;
using RectInt = Rect<IntegerT>;
using RectScalar = Rect<ScalarT>;

// Channel traits type
template<
	typename TypePacked,
	typename TypeChannel,
	SizeT RedBits,
	SizeT GreenBits,
	SizeT BlueBits,
	SizeT AlphaBits,
	SizeT RedOffset = 0,
	SizeT GreenOffset = RedBits,
	SizeT BlueOffset = GreenOffset + GreenBits,
	SizeT AlphaOffset = BlueOffset + BlueBits
>
struct PixelTraits
{
	using PixelType = TypePacked;
	using ChannelType = TypeChannel;

	static constexpr SizeT BitsPerPixel =
		RedBits + GreenBits + BlueBits + AlphaBits;
	static constexpr SizeT BytesPerPixel =
		((BitsPerPixel + 8 - 1) & (~(8 - 1))) / 8;

	static constexpr SizeT RedDepth = RedBits;
	static constexpr SizeT RedShift = RedOffset;

	static constexpr PixelType RedMask()
	{
		return ((PixelType(1) << RedDepth) - PixelType(1)) << RedShift;
	};

	static constexpr SizeT GreenDepth = GreenBits;
	static constexpr SizeT GreenShift = GreenOffset;

	static constexpr PixelType GreenMask()
	{
		return ((PixelType(1) << GreenDepth) - PixelType(1)) << GreenShift;
	};

	static constexpr SizeT BlueDepth = BlueBits;
	static constexpr SizeT BlueShift = BlueOffset;

	static constexpr PixelType BlueMask()
	{
		return ((PixelType(1) << BlueDepth) - PixelType(1)) << BlueShift;
	};

	static constexpr SizeT AlphaDepth = AlphaBits;
	static constexpr SizeT AlphaShift = AlphaOffset;

	static constexpr PixelType AlphaMask()
	{
		return ((PixelType(1) << AlphaDepth) - PixelType(1)) << AlphaShift;
	};
};

namespace PixelFormats
{
using RGBA8888 = PixelTraits<
	std::uint32_t,
	std::uint8_t,
	8, 8, 8, 8
>;

using RGBA4444 = PixelTraits<
	std::uint16_t,
	std::uint8_t,
	4, 4, 4, 4
>;

using RGBA5551 = PixelTraits<
	std::uint16_t,
	std::uint8_t,
	5, 5, 5, 1
>;

using RGB888 = PixelTraits<
	std::uint32_t,
	std::uint8_t,
	8, 8, 8, 0
>;

using RGB565 = PixelTraits<
	std::uint16_t,
	std::uint8_t,
	5, 6, 5, 0
>;
}

template<
	class PixTraits
>
class Pixel
{
public:
	using Traits = PixTraits;
	using ChannelType = typename Traits::ChannelType;
	using PixelType = typename Traits::PixelType;

	Pixel()
		: PixelData(0)
	{
	}

	Pixel(PixelType Value)
		: PixelData(Value)
	{
	}

	Pixel(
		ChannelType Red,
		ChannelType Green,
		ChannelType Blue,
		ChannelType Alpha
	)
		: PixelData(0)
	{
		SetPixel(
			Red,
			Green,
			Blue,
			Alpha
		);
	}

	explicit operator PixelType() const
	{
		return PixelData;
	}

	constexpr ChannelType GetRed() const
	{
		return (PixelData & Traits::RedMask()) >> Traits::RedShift;
	}

	constexpr ChannelType GetGreen() const
	{
		return (PixelData & Traits::GreenMask()) >> Traits::GreenShift;
	}

	constexpr ChannelType GetBlue() const
	{
		return (PixelData & Traits::BlueMask()) >> Traits::BlueShift;
	}

	constexpr ChannelType GetAlpha() const
	{
		return (PixelData & Traits::AlphaMask()) >> Traits::AlphaShift;
	}

	void SetRed(ChannelType Value)
	{
		PixelData &= ~Traits::RedMask();
		PixelData |= (Value << Traits::RedShift) & Traits::RedMask();
	}

	void SetGreen(ChannelType Value)
	{
		PixelData &= ~Traits::GreenMask();
		PixelData |= (Value << Traits::GreenShift) & Traits::GreenMask();
	}

	void SetBlue(ChannelType Value)
	{
		PixelData &= ~Traits::BlueMask();
		PixelData |= (Value << Traits::BlueShift) & Traits::BlueMask();
	}

	void SetAlpha(ChannelType Value)
	{
		PixelData &= ~Traits::AlphaMask();
		PixelData |= (Value << Traits::AlphaShift) & Traits::AlphaMask();
	}

	constexpr PixelType GetPixel() const
	{
		return PixelData;
	}

	void SetPixel(PixelType PackedPixel)
	{
		PixelData = PackedPixel;
	}

	void SetPixel(
		ChannelType Red,
		ChannelType Green,
		ChannelType Blue,
		ChannelType Alpha
	)
	{
		SetRed(Red);
		SetGreen(Green);
		SetBlue(Blue);
		SetAlpha(Alpha);
	}

private:
	typename Traits::PixelType PixelData;
};

using PixelRGBA8888 = Pixel<PixelFormats::RGBA8888>;
using PixelRGBA4444 = Pixel<PixelFormats::RGBA4444>;
using PixelRGBA5551 = Pixel<PixelFormats::RGBA5551>;
using PixelRGB888 = Pixel<PixelFormats::RGB888>;
using PixelRGB565 = Pixel<PixelFormats::RGB565>;

template<
	class PixTraits
>
class BltSurface
{
public:
	using Traits = PixTraits;
	using PixelType = typename Traits::PixelType;

	BltSurface()
		: Width(0),
		Height(0),
		Pixels(nullptr)
	{
	}

	BltSurface(SizeT Width, SizeT Height)
		: Width(Width),
		Height(Height),
		Pixels(nullptr)
	{
		if( Width && Height )
		{
			Pixels.reset(new PixelType[Width * Height]());
		}
		else
		{
			this->Width = this->Height = SizeT(0);
		}
	}

	BltSurface(const BltSurface& Other)
		: Width(Other.GetWidth()),
		Height(Other.GetHeight()),
		Pixels(nullptr)
	{
		Pixels.reset(
			new PixelType[Other.GetWidth() * Other.GetHeight()]()
		);
		std::copy_n(
			Other.GetPixels(),
			Other.GetWidth() * Other.GetHeight(),
			Pixels.get()
		);
	}

	BltSurface& operator=(const BltSurface& Other)
	{
		Width = Other.Width;
		Height = Other.Height;
		Pixels.reset(
			new PixelType[Other.GetWidth() * Other.GetHeight()]()
		);
		std::copy_n(
			Other.GetPixels(),
			Other.GetWidth() * Other.GetHeight(),
			Pixels.get()
		);
		return *this;
	}

	constexpr SizeT GetWidth() const
	{
		return Width;
	}

	constexpr SizeT GetHeight() const
	{
		return Height;
	}

	constexpr PixelType GetPixel(SizeT X, SizeT Y) const
	{
		return Pixels[X + (Y * GetWidth())];
	}

	constexpr const PixelType* GetPixels() const
	{
		return Pixels.get();
	}

	void SetPixel(PointSize Position, Pixel<Traits> Pixel)
	{
		SetPixel(Position.X, Position.Y, Pixel);
	}

	void SetPixel(SizeT X, SizeT Y, Pixel<Traits> Pixel)
	{
		if(
			(X < GetWidth())
			&&
			(Y < GetHeight())
		)
		{
			Pixels[X + (Y * GetWidth())] = static_cast<PixelType>(Pixel);
		}
	}

	void Fill(PixelType Pixel)
	{
		std::fill_n(
			Pixels.get(),
			Width * Height,
			Pixel
		);
	}

	void Line(
		PointSize From,
		PointSize To,
		Pixel<Traits> Color
	)
	{
		const PointInt Delta(
			static_cast<IntegerT>(To.X) - From.X,
			static_cast<IntegerT>(To.Y) - From.Y
		);
		const PointSize DeltaAbs(
			std::abs(Delta.X),
			std::abs(Delta.Y)
		);
		const PointInt DeltaSign(
			Sign<IntegerT>(Delta.X),
			Sign<IntegerT>(Delta.Y)
		);

		PointSize Error(
			DeltaAbs / 2
		);

		PointSize Pen = From;

		if( DeltaAbs.X >= DeltaAbs.Y ) // Horizontal
		{
			for( SizeT i = 0; i < DeltaAbs.X; i++ )
			{
				Error.Y += DeltaAbs.Y;
				if( Error.Y >= DeltaAbs.X )
				{
					Error.Y -= DeltaAbs.X;
					Pen.Y += DeltaSign.Y;
				}
				Pen.X += DeltaSign.X;
				SetPixel(
					Pen,
					Color
				);
			}
		}
		else // Vertical
		{
			for( SizeT i = 0; i < DeltaAbs.Y; i++ )
			{
				Error.X += DeltaAbs.X;
				if( Error.X >= DeltaAbs.Y )
				{
					Error.X -= DeltaAbs.Y;
					Pen.X += DeltaSign.X;
				}
				Pen.Y += DeltaSign.Y;
				SetPixel(
					Pen,
					Color
				);
			}
		}
	}

	void LineStipple(
		PointSize From,
		PointSize To,
		Pixel<Traits> Color,
		SizeT Pattern = 0xAAAAAAAA
	)
	{
		const PointInt Delta(
			static_cast<IntegerT>(To.X) - From.X,
			static_cast<IntegerT>(To.Y) - From.Y
		);
		const PointSize DeltaAbs(
			static_cast<SizeT>(std::abs(Delta.X)),
			static_cast<SizeT>(std::abs(Delta.Y))
		);
		const PointInt DeltaSign(
			Sign<IntegerT>(Delta.X),
			Sign<IntegerT>(Delta.Y)
		);

		PointSize Error(
			DeltaAbs / 2
		);

		PointSize Pen = From;

		if( DeltaAbs.X >= DeltaAbs.Y ) // Horizontal
		{
			for( SizeT i = 0; i < DeltaAbs.X; i++ )
			{
				Error.Y += DeltaAbs.Y;
				if( Error.Y >= DeltaAbs.X )
				{
					Error.Y -= DeltaAbs.X;
					Pen.Y += DeltaSign.Y;
				}
				Pen.X += DeltaSign.X;
				if( Pattern & 1 )
				{
					SetPixel(
						Pen,
						Color
					);
				}
				Pattern = (Pattern << 1) | (Pattern >> (sizeof(SizeT) * CHAR_BIT) - 1);
			}
		}
		else // Vertical
		{
			for( SizeT i = 0; i < DeltaAbs.Y; i++ )
			{
				Error.X += DeltaAbs.X;
				if( Error.X >= DeltaAbs.Y )
				{
					Error.X -= DeltaAbs.Y;
					Pen.X += DeltaSign.X;
				}
				Pen.Y += DeltaSign.Y;
				if( Pattern & 1 )
				{
					SetPixel(
						Pen,
						Color
					);
				}
				Pattern = (Pattern << 1) | (Pattern >> (sizeof(SizeT) * CHAR_BIT) - 1);
			}
		}
	}

	void Circle(
		SizeT CenterX,
		SizeT CenterY,
		SizeT Radius,
		Pixel<Traits> Color
	)
	{
		Circle(
			{CenterX,CenterY},
			Radius,
			Color
		);
	}

	void Circle(
		PointSize Center,
		SizeT Radius,
		Pixel<Traits> Color
	)
	{
		if( Radius )
		{
			PointSize Offset(0, Radius);
			IntegerT Balance = -static_cast<IntegerT>(Radius);
			while( Offset.X <= Offset.Y )
			{
				PointInt P = PointInt(Center.X, Center.X) - Offset;
				const PointInt W = PointInt(Offset * 2);

				SetPixel(P.X, Center.Y + Offset.Y, Color);
				SetPixel(P.X + W.X, Center.Y + Offset.Y, Color);

				SetPixel(P.X, Center.Y - Offset.Y, Color);
				SetPixel(P.X + W.X, Center.Y - Offset.Y, Color);

				SetPixel(P.Y, Center.Y + Offset.X, Color);
				SetPixel(P.Y + W.Y, Center.Y + Offset.X, Color);

				SetPixel(P.Y, Center.Y - Offset.X, Color);
				SetPixel(P.Y + W.Y, Center.Y - Offset.X, Color);

				if( (Balance += Offset.X++ + Offset.X) >= 0 )
				{
					Balance -= --Offset.Y + Offset.Y;
				}
			}
		}
	}

private:
	SizeT Width, Height;
	std::unique_ptr<PixelType[]> Pixels;
};

using SurfaceRGBA8888 = BltSurface<PixelFormats::RGBA8888>;
using SurfaceRGBA4444 = BltSurface<PixelFormats::RGBA4444>;
using SurfaceRGBA5551 = BltSurface<PixelFormats::RGBA5551>;
using SurfaceRGB888 = BltSurface<PixelFormats::RGB888>;
using SurfaceRGB565 = BltSurface<PixelFormats::RGB565>;
}

#ifdef MINBLIT_IMPLEMENTATION
namespace MinBlit
{
}
#endif
