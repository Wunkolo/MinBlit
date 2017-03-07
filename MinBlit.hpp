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

	Copyright (c) 2017 Wunkolo

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
TODO
	Aligned allocation for BltSurface for future SIMD refactor
*/

#pragma once
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <limits>
#include <memory>
#include <algorithm>

namespace MinBlit
{
// Typedefs to abstract underlaying types
using BltScalar = std::float_t;
static constexpr BltScalar ScalarMax = std::numeric_limits<BltScalar>::max();
static constexpr BltScalar ScalarMin = std::numeric_limits<BltScalar>::min();

using BltInteger = std::intmax_t;
static constexpr BltInteger IntegralMax = std::numeric_limits<BltInteger>::max();
static constexpr BltInteger IntegralMin = std::numeric_limits<BltInteger>::min();

using BltSize = std::size_t;
static constexpr BltSize SizeMax = std::numeric_limits<BltSize>::max();
static constexpr BltSize SizeMin = std::numeric_limits<BltSize>::min();

// Utility functions
template< typename T >
inline T Max(T A, T B)
{
	static_assert(
		std::is_arithmetic<T>::value,
		"T must be an arithmetic type"
		);
	return (A >= B) ? A : B;
}

template< typename T >
inline T Min(T A, T B)
{
	static_assert(
		std::is_arithmetic<T>::value,
		"T must be an arithmetic type"
		);
	return (A <= B) ? A : B;
}

template <typename T>
inline T Clamp(T Value, T Lower, T Upper)
{
	static_assert(
		std::is_arithmetic<T>::value,
		"T must be an arithmetic type"
		);
	return (Value >= Upper) ? Upper : ((Value <= Lower) ? Lower : Value);
}

template <typename T>
inline T Abs(T Value)
{
	static_assert(
		std::is_arithmetic<T>::value,
		"T must be an arithmetic type"
		);
	return Value < 0 ? -Value : Value;
}

template <typename T>
inline T Sign(T Value)
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
	operator BltPoint<T>() const
	{
		return BltPoint<T>(
			static_cast<T>(X),
			static_cast<T>(Y)
			);
	}

	inline bool operator==(const BltPoint& Other) const
	{
		return ((X == Other.X) && (Y == Other.Y));
	}

	template< typename T >
	inline BltPoint operator+(const BltPoint<T>& Other) const
	{
		return BltPoint(
			X + static_cast<ScalarType>(Other.X),
			Y + static_cast<ScalarType>(Other.Y)
		);
	}

	template< typename T >
	inline BltPoint operator-(const BltPoint<T>& Other) const
	{
		return BltPoint(
			X - static_cast<ScalarType>(Other.X),
			Y - static_cast<ScalarType>(Other.Y)
		);
	}

	template< typename T >
	inline BltPoint operator*(const BltPoint<T>& Other) const
	{
		return BltPoint(
			X * static_cast<ScalarType>(Other.X),
			Y * static_cast<ScalarType>(Other.Y)
		);
	}

	template< typename T >
	inline BltPoint operator/(const BltPoint<T>& Other) const
	{
		return BltPoint(
			X / static_cast<ScalarType>(Other.X),
			Y / static_cast<ScalarType>(Other.Y)
		);
	}

	template< typename T >
	inline BltPoint operator*(const T Value) const
	{
		return BltPoint(
			X * Value,
			Y * Value
		);
	}

	template< typename T >
	inline BltPoint operator/(const T Value) const
	{
		return BltPoint(
			X / Value,
			Y / Value
		);
	}

	inline BltScalar Length() const
	{
		return std::sqrt(
			static_cast<BltScalar>(
			X * X + Y * Y
		)
		);
	}

	inline BltScalar Dot(const BltPoint& Other) const
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
		const BltPoint& Center,
		const BltPoint& HalfDimensions
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
		if( Abs(Dist.X) = Abs(HalfDimensions.X) )
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
	size_t RedBits,
	size_t GreenBits,
	size_t BlueBits,
	size_t AlphaBits,
	size_t RedOffset = 0,
	size_t GreenOffset = RedBits,
	size_t BlueOffset = GreenOffset + GreenBits,
	size_t AlphaOffset = BlueOffset + BlueBits
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
	uint32_t,
	uint8_t,
	8, 8, 8, 8
>;

using RGBA4444 = BltPixelTraits<
	uint16_t,
	uint8_t,
	4, 4, 4, 4
>;

using RGBA5551 = BltPixelTraits<
	uint16_t,
	uint8_t,
	5, 5, 5, 1
>;

using RGB888 = BltPixelTraits <
	uint32_t,
	uint8_t,
	8, 8, 8, 0
>;

using RGB565 = BltPixelTraits<
	uint16_t,
	uint8_t,
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

	BltPixel(PixelType Value)
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

	operator PixelType() const
	{
		return PixelData;
	}

	inline constexpr ChannelType GetRed() const
	{
		return (PixelData & Traits::RedMask()) >> Traits::RedShift;
	}

	inline constexpr ChannelType GetGreen() const
	{
		return (PixelData & Traits::GreenMask()) >> Traits::GreenShift;
	}

	inline constexpr ChannelType GetBlue() const
	{
		return (PixelData & Traits::BlueMask()) >> Traits::BlueShift;
	}

	inline constexpr ChannelType GetAlpha() const
	{
		return (PixelData & Traits::AlphaMask()) >> Traits::AlphaShift;
	}

	inline void SetRed(ChannelType Value)
	{
		PixelData &= ~Traits::RedMask();
		PixelData |= (Value << Traits::RedShift) & Traits::RedMask();
	}

	inline void SetGreen(ChannelType Value)
	{
		PixelData &= ~Traits::GreenMask();
		PixelData |= (Value << Traits::GreenShift) & Traits::GreenMask();
	}

	inline void SetBlue(ChannelType Value)
	{
		PixelData &= ~Traits::BlueMask();
		PixelData |= (Value << Traits::BlueShift) & Traits::BlueMask();
	}

	inline void SetAlpha(ChannelType Value)
	{
		PixelData &= ~Traits::AlphaMask();
		PixelData |= (Value << Traits::AlphaShift) & Traits::AlphaMask();
	}

	inline constexpr PixelType GetPixel() const
	{
		return PixelData;
	}

	inline void SetPixel(PixelType PackedPixel)
	{
		PixelData = PackedPixel;
	}

	inline void SetPixel(
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
		Pixels(nullptr),
		Width(0),
		Height(0)
	{
	}

	BltSurface(BltSize Width, BltSize Height)
		:
		Pixels(nullptr),
		Width(Width),
		Height(Height)
	{
		if( Width && Height )
		{
			Pixels.reset(new PixelType[Width * Height]());
		}
		else
		{
			Width = Height = BltSize(0);
		}
	}

	BltSurface(const BltSurface& Other)
		:
		Pixels(nullptr),
		Width(Other.GetWidth()),
		Height(Other.GetHeight())
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
	}

	inline constexpr BltSize GetWidth() const
	{
		return Width;
	}

	inline constexpr BltSize GetHeight() const
	{
		return Height;
	}

	inline constexpr PixelType GetPixel(BltSize X, BltSize Y) const
	{
		return Pixels[X + (Y * GetWidth())];
	}

	inline constexpr const PixelType* GetPixels() const
	{
		return Pixels.get();
	}

	inline void SetPixel(BltPointSize Position, PixelType Pixel)
	{
		SetPixel(Position.X, Position.Y, Pixel);
	}

	inline void SetPixel(BltSize X, BltSize Y, PixelType Pixel)
	{
		if(
			(X < GetWidth())
			&&
			(Y < GetHeight())
			)
		{
			Pixels[X + (Y * GetWidth())] = Pixel;
		}
	}

	inline void Fill(PixelType Pixel)
	{
		std::fill_n(
			Pixels.get(),
			Width * Height,
			Pixel
		);
	}

	inline void Line(
		BltPointSize From,
		BltPointSize To,
		BltPixel<Traits> Color
	)
	{
		BltPointInt Delta(
			static_cast<BltInteger>(To.X) - From.X,
			static_cast<BltInteger>(To.Y) - From.Y
		);
		BltPointSize DeltaAbs(
			Abs(Delta.X),
			Abs(Delta.Y)
		);
		BltPointInt Sign(
			Sign(Delta.X),
			Sign(Delta.Y)
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
					Pen.Y += Sign.Y;
				}
				Pen.X += Sign.X;
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
					Pen.X += Sign.X;
				}
				Pen.Y += Sign.Y;
				SetPixel(
					Pen,
					Color
				);
			}
		}
	}

	inline void LineStipple(
		BltPointSize From,
		BltPointSize To,
		BltPixel<Traits> Color,
		uint32_t Pattern = 0xAAAAAAAA
	)
	{
		BltPointInt Delta(
			static_cast<BltInteger>(To.X) - From.X,
			static_cast<BltInteger>(To.Y) - From.Y
		);
		BltPointSize DeltaAbs(
			Abs(Delta.X),
			Abs(Delta.Y)
		);
		BltPointInt Sign(
			Sign(Delta.X),
			Sign(Delta.Y)
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
					Pen.Y += Sign.Y;
				}
				Pen.X += Sign.X;
				if( Pattern & 1 )
				{
					SetPixel(
						Pen,
						Color
					);
				}
				Pattern = (Pattern << 1) | (Pattern >> 31);
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
					Pen.X += Sign.X;
				}
				Pen.Y += Sign.Y;
				if( Pattern & 1 )
				{
					SetPixel(
						Pen,
						Color
					);
				}
				Pattern = (Pattern << 1) | (Pattern >> 31);
			}
		}
	}

	inline void Circle(
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

	inline void Circle(
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
				BltPointInt W = Offset * 2;;

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