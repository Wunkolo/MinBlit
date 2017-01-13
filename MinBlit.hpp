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
*/

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <type_traits>
#include <limits>

namespace MinBlit
{
// Typedefs to abstract underlaying types
using BltScalar = float_t;
static constexpr BltScalar ScalarMax = std::numeric_limits<BltScalar>::max();
static constexpr BltScalar ScalarMin = std::numeric_limits<BltScalar>::min();

using BltIntegral = intmax_t;
static constexpr BltIntegral IntegralMax = std::numeric_limits<BltIntegral>::max();
static constexpr BltIntegral IntegralMin = std::numeric_limits<BltIntegral>::min();

using BltSize = size_t;
static constexpr BltSize SizeMax = std::numeric_limits<BltSize>::max();
static constexpr BltSize SizeMin = std::numeric_limits<BltSize>::min();

// Utility functions
template< typename T >
inline T Max(T A, T B)
{
	static_assert(
		std::enable_if<std::is_arithmetic<T>>::value
		"T must be an arithmetic type"
		);
	return (A >= B) ? A : B;
}

template< typename T >
inline T Min(T A, T B)
{
	static_assert(
		std::enable_if<std::is_arithmetic<T>>::value
		"T must be an arithmetic type"
		);
	return (A <= B) ? A : B;
}

template <typename T>
inline T Clamp(T Value, T Lower, T Upper)
{
	static_assert(
		std::enable_if<std::is_arithmetic<T>>::value
		"T must be an arithmetic type"
		);
	return (Value >= Upper) ? Upper : ((Value <= Lower) ? Lower : Value);
}

template <typename T>
inline T Abs(T Value)
{
	static_assert(
		std::enable_if<std::is_arithmetic<T>>::value
		"T must be an arithmetic type"
		);
	return Value < 0 ? -Value : Value;
}

template <typename T>
inline T Sign(T Value)
{
	static_assert(
		std::enable_if<std::is_arithmetic<T>>::value
		"T must be an arithmetic type"
		);
	return (T(0) < Value) - (Value < T(0));
}

// Integer square-root
inline BltIntegral isqrt(BltIntegral x)
{
	BltIntegral Odd(1);
	BltIntegral Count(0);
	BltIntegral Sum(0);
	while( Sum < x )
	{
		Count++;
		Sum += Odd;
		Odd += BltIntegral(2);
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

	inline bool operator==(const BltPoint& Other) const
	{
		return ((X == Other.X) && (Y == Other.Y));
	}

	inline BltPoint operator+(const BltPoint& Other) const
	{
		return BltPoint(
			X + Other.X,
			Y + Other.Y
		);
	}

	inline BltPoint operator-(const BltPoint& Other) const
	{
		return BltPoint(
			X - Other.X,
			Y - Other.Y
		);
	}
	inline BltPoint operator*(const BltPoint& Other) const
	{
		return BltPoint(
			X * Other.X,
			Y * Other.Y
		);
	}
	inline BltPoint operator/(const BltPoint& Other) const
	{
		return BltPoint(
			X / Other.X,
			Y / Other.Y
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
}

template<
	class PixTraits
>
class BltPixel
{
public:
	BltPixel()
		:
		PixelData(0)
	{
	}

	BltPixel(typename PixTraits::PixelType Value)
		:
		PixelData(Value)
	{
	}

	BltPixel(
		typename PixTraits::ChannelType Red,
		typename PixTraits::ChannelType Green,
		typename PixTraits::ChannelType Blue,
		typename PixTraits::ChannelType Alpha
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

	inline constexpr typename PixTraits::ChannelType GetRed() const
	{
		return (PixelData & PixTraits::RedMask()) >> PixTraits::RedShift;
	}

	inline constexpr typename PixTraits::ChannelType GetGreen() const
	{
		return (PixelData & PixTraits::GreenMask()) >> PixTraits::GreenShift;
	}

	inline constexpr typename PixTraits::ChannelType GetBlue() const
	{
		return (PixelData & PixTraits::BlueMask()) >> PixTraits::BlueShift;
	}

	inline constexpr typename PixTraits::ChannelType GetAlpha() const
	{
		return (PixelData & PixTraits::AlphaMask()) >> PixTraits::AlphaShift;
	}

	inline void SetRed(typename PixTraits::ChannelType Value)
	{
		PixelData &= ~PixTraits::RedMask();
		PixelData |= (Value << PixTraits::RedShift) & PixTraits::RedMask();
	}

	inline void SetGreen(typename PixTraits::ChannelType Value)
	{
		PixelData &= ~PixTraits::GreenMask();
		PixelData |= (Value << PixTraits::GreenShift) & PixTraits::GreenMask();
	}

	inline void SetBlue(typename PixTraits::ChannelType Value)
	{
		PixelData &= ~PixTraits::BlueMask();
		PixelData |= (Value << PixTraits::BlueShift) & PixTraits::BlueMask();
	}

	inline void SetAlpha(typename PixTraits::ChannelType Value)
	{
		PixelData &= ~PixTraits::AlphaMask();
		PixelData |= (Value << PixTraits::AlphaShift) & PixTraits::AlphaMask();
	}

	inline constexpr typename PixTraits::PixelType GetPixel() const
	{
		return PixelData;
	}

	inline void SetPixel(typename PixTraits::PixelType PackedPixel)
	{
		PixelData = PackedPixel;
	}

	inline void SetPixel(
		typename PixTraits::ChannelType Red,
		typename PixTraits::ChannelType Green,
		typename PixTraits::ChannelType Blue,
		typename PixTraits::ChannelType Alpha
	)
	{
		SetRed(Red);
		SetGreen(Green);
		SetBlue(Blue);
		SetAlpha(Alpha);
	}

private:
	typename PixTraits::PixelType PixelData;
};

using BltPixelRGBA8888 = BltPixel<PixelFormats::RGBA8888>;
using BltPixelRGBA4444 = BltPixel<PixelFormats::RGBA4444>;
using BltPixelRGBA5551 = BltPixel<PixelFormats::RGBA5551>;
}

#ifdef MINBLIT_IMPLEMENTATION
namespace MinBlit
{
}
#endif