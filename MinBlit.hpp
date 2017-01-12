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
}

#ifdef MINBLIT_IMPLEMENTATION
namespace MinBlit
{
}
#endif