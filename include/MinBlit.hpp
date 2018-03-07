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
using BltScalar = std::float_t;
static_assert(
	std::is_floating_point<BltScalar>::value,
	"BltScalar floating point type required"
);
static constexpr BltScalar ScalarMax = std::numeric_limits<BltScalar>::max();
static constexpr BltScalar ScalarMin = std::numeric_limits<BltScalar>::min();

using BltInteger = std::intmax_t;
static constexpr BltInteger IntegralMax = std::numeric_limits<BltInteger>::max();
static constexpr BltInteger IntegralMin = std::numeric_limits<BltInteger>::min();
static_assert(
	std::is_integral<BltInteger>::value,
	"BltInteger integral type required"
);

using BltSize = std::size_t;
static constexpr BltSize SizeMax = std::numeric_limits<BltSize>::max();
static constexpr BltSize SizeMin = std::numeric_limits<BltSize>::min();
static_assert(
	std::is_unsigned<BltSize>::value,
	"BltSize integral type required"
);

// Utility functions
template< typename T >
T Max(T A, T B)
{
	static_assert(
		std::is_arithmetic<T>::value,
		"T must be an arithmetic type"
	);
	return (A >= B) ? A : B;
}

template< typename T >
T Min(T A, T B)
{
	static_assert(
		std::is_arithmetic<T>::value,
		"T must be an arithmetic type"
	);
	return (A <= B) ? A : B;
}

template< typename T >
T Clamp(T Value, T Lower, T Upper)
{
	static_assert(
		std::is_arithmetic<T>::value,
		"T must be an arithmetic type"
	);
	return (Value >= Upper) ? Upper : ((Value <= Lower) ? Lower : Value);
}

template< typename T >
T Abs(T Value)
{
	static_assert(
		std::is_arithmetic<T>::value,
		"T must be an arithmetic type"
	);
	return Value < 0 ? -Value : Value;
}

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
inline BltInteger isqrt(BltInteger x)
{
	BltInteger Odd(1);
	BltInteger Count(0);
	BltInteger Sum(0);
	while( Sum < x )
	{
		Count++;
		Sum += Odd;
		Odd += BltInteger(2);
	}
	return Count--;
}

template< typename ScalarType >
class BltPoint
{
public:
	ScalarType X, Y;
	BltPoint()
		:
		X(0),
		Y(0)
	{
	}

	BltPoint(ScalarType X, ScalarType Y)
		:
		X(X),
		Y(Y)
	{
	}

	template<typename T>
	explicit operator BltPoint<T>() const
	{
		return BltPoint<T>(
			static_cast<T>(X),
			static_cast<T>(Y)
		);
	}

	bool operator==(const BltPoint& Other) const
	{
		return ((X == Other.X) && (Y == Other.Y));
	}

	template< typename T >
	BltPoint operator+(const BltPoint<T>& Other) const
	{
		return BltPoint(
			X + static_cast<ScalarType>(Other.X),
			Y + static_cast<ScalarType>(Other.Y)
		);
	}

	template< typename T >
	BltPoint operator-(const BltPoint<T>& Other) const
	{
		return BltPoint(
			X - static_cast<ScalarType>(Other.X),
			Y - static_cast<ScalarType>(Other.Y)
		);
	}

	template< typename T >
	BltPoint operator*(const BltPoint<T>& Other) const
	{
		return BltPoint(
			X * static_cast<ScalarType>(Other.X),
			Y * static_cast<ScalarType>(Other.Y)
		);
	}

	template< typename T >
	BltPoint operator/(const BltPoint<T>& Other) const
	{
		return BltPoint(
			X / static_cast<ScalarType>(Other.X),
			Y / static_cast<ScalarType>(Other.Y)
		);
	}

	template< typename T >
	BltPoint operator*(const T Value) const
	{
		return BltPoint(
			X * Value,
			Y * Value
		);
	}

	template< typename T >
	BltPoint operator/(const T Value) const
	{
		return BltPoint(
			X / Value,
			Y / Value
		);
	}

	BltScalar Length() const
	{
		return std::sqrt(
			static_cast<BltScalar>(
				X * X + Y * Y
				)
		);
	}

	BltScalar Dot(const BltPoint& Other) const
	{
		return X * Other.X + Y * Other.Y;
	}
};

using BltPointSize = BltPoint<BltSize>;
using BltPointInt = BltPoint<BltInteger>;
using BltPointScalar = BltPoint<BltScalar>;

template< typename ScalarType >
class BltRect
{
public:
	BltPoint<ScalarType> Center, HalfDimensions;

	BltRect()
		:
		Center(0, 0),
		HalfDimensions(0, 0)
	{
	}

	BltRect(
		const BltPoint<ScalarType>& Center,
		const BltPoint<ScalarType>& HalfDimensions
	)
		:
		Center(Center),
		HalfDimensions(HalfDimensions)
	{
	}

	BltRect(
		ScalarType CenterX,
		ScalarType CenterY,
		ScalarType HalfWidth,
		ScalarType HalfHeight
	)
		:
		Center(CenterX, CenterY),
		HalfDimensions(HalfWidth, HalfHeight)
	{
	}

	bool Contains(const BltPoint<ScalarType>& Point) const
	{
		BltPoint<ScalarType> Dist = Center - Point;
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

using BltRectSize = BltRect<BltSize>;
using BltRectInt = BltRect<BltInteger>;
using BltRectScalar = BltRect<BltScalar>;

// Channel traits type
template<
	typename TypePacked,
	typename TypeChannel,
	BltSize RedBits,
	BltSize GreenBits,
	BltSize BlueBits,
	BltSize AlphaBits,
	BltSize RedOffset = 0,
	BltSize GreenOffset = RedBits,
	BltSize BlueOffset = GreenOffset + GreenBits,
	BltSize AlphaOffset = BlueOffset + BlueBits
>
struct BltPixelTraits
{
	using PixelType = TypePacked;
	using ChannelType = TypeChannel;

	static constexpr BltSize BitsPerPixel =
		RedBits + GreenBits + BlueBits + AlphaBits;
	static constexpr BltSize BytesPerPixel =
		((BitsPerPixel + 8 - 1) & (~(8 - 1))) / 8;

	static constexpr BltSize RedDepth = RedBits;
	static constexpr BltSize RedShift = RedOffset;
	static constexpr PixelType RedMask()
	{
		return ((PixelType(1) << RedDepth) - PixelType(1)) << RedShift;
	};

	static constexpr BltSize GreenDepth = GreenBits;
	static constexpr BltSize GreenShift = GreenOffset;
	static constexpr PixelType GreenMask()
	{
		return ((PixelType(1) << GreenDepth) - PixelType(1)) << GreenShift;
	};

	static constexpr BltSize BlueDepth = BlueBits;
	static constexpr BltSize BlueShift = BlueOffset;
	static constexpr PixelType BlueMask()
	{
		return ((PixelType(1) << BlueDepth) - PixelType(1)) << BlueShift;
	};

	static constexpr BltSize AlphaDepth = AlphaBits;
	static constexpr BltSize AlphaShift = AlphaOffset;
	static constexpr PixelType AlphaMask()
	{
		return ((PixelType(1) << AlphaDepth) - PixelType(1)) << AlphaShift;
	};
};

namespace PixelFormats
{
using RGBA8888 = BltPixelTraits<
	std::uint32_t,
	std::uint8_t,
	8, 8, 8, 8
>;

using RGBA4444 = BltPixelTraits<
	std::uint16_t,
	std::uint8_t,
	4, 4, 4, 4
>;

using RGBA5551 = BltPixelTraits<
	std::uint16_t,
	std::uint8_t,
	5, 5, 5, 1
>;

using RGB888 = BltPixelTraits <
	std::uint32_t,
	std::uint8_t,
	8, 8, 8, 0
>;

using RGB565 = BltPixelTraits<
	std::uint16_t,
	std::uint8_t,
	5, 6, 5, 0
>;
}

template<
	class PixTraits
>
class BltPixel
{
public:
	using Traits = PixTraits;
	using ChannelType = typename Traits::ChannelType;
	using PixelType = typename Traits::PixelType;

	BltPixel()
		:
		PixelData(0)
	{
	}

	explicit BltPixel(PixelType Value)
		:
		PixelData(Value)
	{
	}

	BltPixel(
		ChannelType Red,
		ChannelType Green,
		ChannelType Blue,
		ChannelType Alpha
	)
		:
		PixelData(0)
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

using BltPixelRGBA8888 = BltPixel<PixelFormats::RGBA8888>;
using BltPixelRGBA4444 = BltPixel<PixelFormats::RGBA4444>;
using BltPixelRGBA5551 = BltPixel<PixelFormats::RGBA5551>;
using BltPixelRGB888 = BltPixel<PixelFormats::RGB888>;
using BltPixelRGB565 = BltPixel<PixelFormats::RGB565>;

template<
	class PixTraits
>
class BltSurface
{
public:
	using Traits = PixTraits;
	using PixelType = typename Traits::PixelType;

	BltSurface()
		:
		Width(0),
		Height(0),
		Pixels(nullptr)
	{
	}

	BltSurface(BltSize Width, BltSize Height)
		:
		Width(Width),
		Height(Height),
		Pixels(nullptr)
	{
		if( Width && Height )
		{
			Pixels.reset(new PixelType[Width * Height]());
		}
		else
		{
			this->Width = this->Height = BltSize(0);
		}
	}

	BltSurface(const BltSurface& Other)
		:
		Width(Other.GetWidth()),
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

	constexpr BltSize GetWidth() const
	{
		return Width;
	}

	constexpr BltSize GetHeight() const
	{
		return Height;
	}

	constexpr PixelType GetPixel(BltSize X, BltSize Y) const
	{
		return Pixels[X + (Y * GetWidth())];
	}

	constexpr const PixelType* GetPixels() const
	{
		return Pixels.get();
	}

	void SetPixel(BltPointSize Position, BltPixel<Traits> Pixel)
	{
		SetPixel(Position.X, Position.Y, Pixel);
	}

	void SetPixel(BltSize X, BltSize Y, BltPixel<Traits> Pixel)
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
		BltPointSize From,
		BltPointSize To,
		BltPixel<Traits> Color
	)
	{
		const BltPointInt Delta(
			static_cast<BltInteger>(To.X) - From.X,
			static_cast<BltInteger>(To.Y) - From.Y
		);
		const BltPointSize DeltaAbs(
			Abs(Delta.X),
			Abs(Delta.Y)
		);
		const BltPointInt DeltaSign(
			Sign<BltInteger>(Delta.X),
			Sign<BltInteger>(Delta.Y)
		);

		BltPointSize Error(
			DeltaAbs / 2
		);

		BltPointSize Pen = From;

		if( DeltaAbs.X >= DeltaAbs.Y ) // Horizontal
		{
			for( BltSize i = 0; i < DeltaAbs.X; i++ )
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
			for( BltSize i = 0; i < DeltaAbs.Y; i++ )
			{
				Error.X += DeltaAbs.X;
				if( Error.X >= DeltaAbs.Y )
				{
					Error.X -= DeltaAbs.Y;
					Pen.X += DeltaSign.X;
				}
				Pen.Y += Sign.Y;
				SetPixel(
					Pen,
					Color
				);
			}
		}
	}

	void LineStipple(
		BltPointSize From,
		BltPointSize To,
		BltPixel<Traits> Color,
		BltSize Pattern = 0xAAAAAAAA
	)
	{
		const BltPointInt Delta(
			static_cast<BltInteger>(To.X) - From.X,
			static_cast<BltInteger>(To.Y) - From.Y
		);
		const BltPointSize DeltaAbs(
			Abs<BltInteger>(Delta.X),
			Abs<BltInteger>(Delta.Y)
		);
		const BltPointInt DeltaSign(
			Sign<BltInteger>(Delta.X),
			Sign<BltInteger>(Delta.Y)
		);

		BltPointSize Error(
			DeltaAbs / 2
		);

		BltPointSize Pen = From;

		if( DeltaAbs.X >= DeltaAbs.Y ) // Horizontal
		{
			for( BltSize i = 0; i < DeltaAbs.X; i++ )
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
				Pattern = (Pattern << 1) | (Pattern >> (sizeof(BltSize) * CHAR_BIT) - 1);
			}
		}
		else // Vertical
		{
			for( BltSize i = 0; i < DeltaAbs.Y; i++ )
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
				Pattern = (Pattern << 1) | (Pattern >> (sizeof(BltSize) * CHAR_BIT) - 1);
			}
		}
	}

	void Circle(
		BltSize CenterX,
		BltSize CenterY,
		BltSize Radius,
		BltPixel<Traits> Color
	)
	{
		Circle(
		{ CenterX,CenterY },
			Radius,
			Color
		);
	}

	void Circle(
		BltPointSize Center,
		BltSize Radius,
		BltPixel<Traits> Color
	)
	{
		if( Radius )
		{
			BltPointSize Offset(0, Radius);
			BltInteger Balance = -static_cast<BltInteger>(Radius);
			while( Offset.X <= Offset.Y )
			{
				BltPointInt P = BltPointInt(Center.X, Center.X) - Offset;
				const BltPointInt W = static_cast<BltPointInt>(Offset * 2);

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
	BltSize Width, Height;
	std::unique_ptr<PixelType[]> Pixels;
};

using BltSurfaceRGBA8888 = BltSurface<PixelFormats::RGBA8888>;
using BltSurfaceRGBA4444 = BltSurface<PixelFormats::RGBA4444>;
using BltSurfaceRGBA5551 = BltSurface<PixelFormats::RGBA5551>;
using BltSurfaceRGB888 = BltSurface<PixelFormats::RGB888>;
using BltSurfaceRGB565 = BltSurface<PixelFormats::RGB565>;
}

#ifdef MINBLIT_IMPLEMENTATION
namespace MinBlit
{
}
#endif
