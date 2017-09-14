#pragma once
#include <stdint.h>
#include <initializer_list>
#include <memory>
#include <float.h>

/*
A Minimal 256 color index blitter
All images are 8-bit grayscale images

#define MINBLT_IMPLEMENT in at least one cpp file to implement

Todo:
	Implement Mask/clip objects, encapsulate Surfaces and reinterprets calls to SetPixel so that only calls within the mask actually set the pixel.
	Shape rasterization(Circles, Squares, Polygons, Filled, etc).
	Simple operations (fill, fill rect, copy region, scale, mosiac, etc)

Palette + Palette rendering

SSE speedup

May 4, 2016
*/

namespace MinBlt
{
	typedef size_t BltSize;
	typedef ptrdiff_t BltReal;
	typedef uint8_t BltIndex;

#if FLT_EVAL_METHOD == 0
	typedef float BltFloat;
#elif FLT_EVAL_METHOD == 1
	typedef double BltFloat;
#elif FLT_EVAL_METHOD == 2
	typedef long double BltFloat;
#else
	typedef float BltFloat;
#endif

	// Templated utility functions
	template <typename T>
	inline T Max(T a, T b)
	{
		return (a >= b) ? a : b;
	}

	template <typename T>
	inline T Min(T a, T b)
	{
		return (a <= b) ? a : b;
	}

	template <typename T>
	inline T Clamp(T x, T low, T high)
	{
		return (x >= high) ? high : ((x <= low) ? low : x);
	}

	template <typename T>
	inline T Abs(T x)
	{
		return x < 0 ? -x : x;
	}

	template <typename T>
	inline T Sign(T x)
	{
		return (T(0) < x) - (x < T(0));
	}

	template <typename T>
	T swap_endian(T u)
	{
		union
		{
			T u;
			unsigned char u8[sizeof(T)];
		} source, dest;

		source.u = u;

		for( size_t k = 0; k < sizeof(T); k++ )
		{
			dest.u8[k] = source.u8[sizeof(T) - k - 1];
		}

		return dest.u;
	}

	// Utility functions

	inline BltSize isqrt(BltSize x)
	{
		BltSize odd = 1, count = 0, sum = 0;
		while( sum < x )
		{
			count++;
			sum += odd;
			odd += 2;
		}
		return count--;
	}

	// Uses the half lower bits of the Factor variable to determine the lerp factor
	// Factor range is [0.0 - 1.0)
	// A must be lower than B
	inline BltReal ilerp(BltReal A, BltReal B, BltSize Factor)
	{
		const BltSize shift = (sizeof(BltSize) * 8) / 2;
		return A + ((B - A) * (Factor - ((Factor >> shift) << shift)) >> shift);
	}

	//// Point
	struct BltPoint
	{
	public:
		BltReal X, Y;

		BltPoint()
			:
			X(0), Y(0)
		{
		}

		BltPoint(BltReal X, BltReal Y)
			:
			X(X), Y(Y)
		{
		}

		// Vector Arithmetic
		inline bool operator == (const BltPoint& Other) const
		{
			return ((X == Other.X) && (Y == Other.Y));
		}

		inline BltPoint operator + (const BltPoint& Other) const
		{
			return BltPoint(X + Other.X, Y + Other.Y);
		}

		inline BltPoint operator - (const BltPoint& Other) const
		{
			return BltPoint(X - Other.X, Y - Other.Y);
		}

		// Scalar Arithmetic
		inline BltPoint operator * (const BltReal& Other) const
		{
			return BltPoint(X * Other, Y * Other);
		}

		inline BltPoint operator / (const BltReal& Other) const
		{
			return BltPoint(X / Other, Y / Other);
		}

		// Vector Arithetic

		inline BltFloat Length() const
		{
			return std::sqrt(
				static_cast<BltFloat>(X*X + Y*Y)
				);
		}

		// Integer length of vector
		inline BltSize iLength() const
		{
			return isqrt(X*X + Y*Y);
		}

		// Integer unnormalized Dot product
		inline BltReal Dot(const BltPoint& Other) const
		{
			return (X*Other.X + Y*Other.Y);
		}
	};

	struct BltRect
	{
	public:
		BltPoint Center, HalfWidth;

		BltRect()
			:
			Center(0, 0), HalfWidth(0, 0)
		{
		}

		BltRect(const BltPoint& Center, const BltPoint& HalfWidths)
			:
			Center(Center), HalfWidth(HalfWidths)
		{
		}
		bool Contains(const BltPoint& Point) const
		{
			BltPoint Dist = Center - Point;
			if( Abs(Dist.X) <= Abs(HalfWidth.X) )
			{
				if( Abs(Dist.Y) <= Abs(HalfWidth.Y) )
				{
					return true;
				}
			}
			return false;
		}
	};

	// 32 bit color. 8bpp XRGB
	struct BltColor
	{
	public:
		union
		{
			struct
			{
				uint8_t X, R, G, B;
			};
			uint32_t XRGB;
		};

		BltColor() : XRGB(0)
		{
		}
		BltColor(uint32_t RGBX)
		{
			XRGB = swap_endian<uint32_t>(RGBX);
			X = 0;
		}
		BltColor(uint8_t R, uint8_t G, uint8_t B) :
			R(R), G(G), B(B), X(0)
		{
		}
	};

	// Palette of 256 colors
	struct BltPalette
	{
	public:
		BltPalette() : Size(0)
		{
			/*
			for( size_t i = 0; i < 256; i++ )
			{
			Palette[i] = BltColor(0);
			}
			*/
			memset(Palette, 0, 256 * 4);
		}

		// Returns a read-only array of colors
		const inline BltColor* Colors() const
		{
			return static_cast<const BltColor*>(Palette);
		}

		// Returns the total number of pushed colors
		const inline BltSize Count() const
		{
			return Size;
		}

		// Returns the new size of the palette if pushed successfully
		// Use the returned size to know what offset to use to get your color back
		// ex: PushColor(0xFF0000) - 1 would return the location of your added color;
		const inline BltSize PushColor(const BltColor& Color)
		{
			if( Size < 256 )
			{
				Palette[Size++] = Color;
			}
			return Size;
		}
		const inline BltSize PushColor(uint8_t R, uint8_t G, uint8_t B)
		{
			return PushColor(BltColor(R, G, B));
		}
		const inline BltSize PushColor(uint32_t XRGB)
		{
			return PushColor(BltColor(XRGB));
		}

	private:
		BltColor Palette[256];
		BltSize Size;
	};

	//// Surface
	class BltSurface
	{
	public:
		BltSurface();
		BltSurface(BltSize Width, BltSize Height);
		BltSurface(BltSize Width, BltSize Height, const uint8_t* Pixels);
		~BltSurface();

		inline BltSize BltSurface::GetWidth() const
		{
			return this->Width;
		}
		inline BltSize BltSurface::GetHeight() const
		{
			return this->Height;
		}
		inline uint8_t* Data() const
		{
			return this->Indices;
		}

		uint8_t GetPixel(BltSize X, BltSize Y) const;

		void SetPixel(BltPoint Location, uint8_t Index);
		void SetPixel(BltSize X, BltSize Y, uint8_t Index);

		void Fill(uint8_t Index);
	private:
		// Non-copyable
		BltSurface(const BltSurface& other) = delete;
		BltSurface& operator= (const BltSurface&) = delete;

		BltSize Width, Height;
		uint8_t *Indices;
	};

	//// Operations
	// Arithmetic ( Saturated )
	BltSurface& Add(BltSurface& Surface, uint8_t Param);
	BltSurface& Sub(BltSurface& Surface, uint8_t Param);
	BltSurface& Mul(BltSurface& Surface, uint8_t Param);
	BltSurface& Div(BltSurface& Surface, uint8_t Param);
	BltSurface& Mod(BltSurface& Surface, uint8_t Param);

	// Bitwise
	BltSurface& And(BltSurface& Surface, uint8_t Param);
	BltSurface& Or(BltSurface& Surface, uint8_t Param);
	BltSurface& Xor(BltSurface& Surface, uint8_t Param);
	BltSurface& Not(BltSurface& Surface);
	BltSurface& SHL(BltSurface& Surface, uint8_t Param);
	BltSurface& SHR(BltSurface& Surface, uint8_t Param);

	//// Drawing// Blitting
	void Render(const BltSurface& Surface, const BltPalette& Palette, uint8_t* Dest);

	// Matches an indexed surface with a palette and generated a fully colored
	// 32 bit RGBA image into Dest
	// Dest should be an array of size Surface.Width()*Height.Height()*4
	// Transparent is the color index assigned to be transparent
	void Render(const BltSurface& Surface, const BltPalette& Palette, uint8_t* Dest, const uint8_t Transparent);

	// Blitting
	// Straight Blit
	void Blit(BltSurface& Dest, const BltSurface& Source);

	// Alpha-enabled blit
	void Blit(BltSurface& Dest, const BltSurface& Source, BltIndex Transparent);

	// Shape Drawing
	// Line
	BltSurface& Line(BltSurface& Surface,
		BltPoint From, BltPoint To,
		BltIndex Index);
	BltSurface& Line(BltSurface& Surface,
		BltReal FromX, BltReal FromY, BltReal ToX, BltReal ToY,
		BltIndex Index);

	// Fill
	BltSurface& Fill(BltSurface& Surface,
		BltIndex Index);

	// Rectangle
	BltSurface& Rect(BltSurface& Surface,
		BltPoint P1, BltPoint P2,
		BltIndex Index);
	BltSurface& Rect(BltSurface& Surface,
		BltReal FromX, BltReal FromY, BltReal ToX, BltReal ToY,
		BltIndex Index);

	BltSurface& FillRect(BltSurface& Surface,
		BltPoint P1, BltPoint P2,
		BltIndex Index);
	BltSurface& FillRect(BltSurface& Surface,
		BltReal FromX, BltReal FromY, BltReal ToX, BltReal ToY,
		BltIndex Index);

	// Circle
	BltSurface& Circle(BltSurface& Surface,
		BltPoint Center, BltSize Radius,
		BltIndex Index);
	BltSurface& Circle(BltSurface& Surface,
		BltReal CenterX, BltReal CenterY, BltSize Radius,
		BltIndex Index);

	BltSurface& FillCircle(BltSurface& Surface,
		BltPoint Center, BltSize Radius,
		BltIndex Index);
	BltSurface& FillCircle(BltSurface& Surface,
		BltReal CenterX, BltReal CenterY, BltSize Radius,
		BltIndex Index);

	/////////////////////////// Implementation
#ifdef MINBLT_IMPLEMENT

//// Surface
	BltSurface::BltSurface()
		:
		Width(1), Height(1)
	{
		Indices = new BltIndex[1]();
	}

	BltSurface::BltSurface(BltSize Width, BltSize Height)
		:
		Width(Width), Height(Height)
	{
		if( Width && Height )
		{
			Indices = new uint8_t[Width * Height]();
		}
		else
		{
			Width = Height = 1;
			Indices = new BltIndex[1]();
		}
	}

	BltSurface::BltSurface(BltSize Width, BltSize Height, const uint8_t* Pixels)
		:
		Width(Width), Height(Height)
	{
		if( Width && Height && Pixels )
		{
			Indices = new uint8_t[Width * Height]();
			memcpy(Indices, Pixels, Width*Height);
		}
		else
		{
			Width = Height = 1;
			Indices = new BltIndex[1]();
		}
	}

	BltSurface::~BltSurface()
	{
		if( Indices != nullptr )
		{
			delete[] Indices;
		}
	}

	uint8_t BltSurface::GetPixel(BltSize X, BltSize Y) const
	{
		if( Indices != nullptr &&
			X < Width && Y < Height )
		{
			return Indices[Width * Y + X];
		}
		return 0;
	}

	void BltSurface::SetPixel(BltPoint Location, uint8_t Index)
	{
		SetPixel(Location.X, Location.Y, Index);
	}

	void BltSurface::SetPixel(BltSize X, BltSize Y, BltIndex Index)
	{
		if( Indices != nullptr &&
			X < Width && Y < Height )
		{
			Indices[Width * Y + X] = Index;
		}
	}

	void BltSurface::Fill(BltIndex Index)
	{
		if( Indices != nullptr )
		{
			/*
			for( size_t i = 0; i < _Width*_Height; i++ )
			{
			_Pixels[i] = Index;
			}
			*/
			memset(Indices, Index, Width*Height);
		}
	}

	////Operations
	// Saturated Arithmetic
	BltSurface& Add(BltSurface& Surface, uint8_t Param)
	{
		for( size_t i = 0; i < Surface.GetWidth()*Surface.GetHeight(); i++ )
		{
			uint8_t result = Surface.Data()[i] + Param;
			result |= -(result < Param);
			Surface.Data()[i] = result;
		}
		return Surface;
	}

	BltSurface& Sub(BltSurface& Surface, uint8_t Param)
	{
		for( size_t i = 0; i < Surface.GetWidth()*Surface.GetHeight(); i++ )
		{
			uint8_t result = Surface.Data()[i] - Param;
			result &= -(result <= Param);
			Surface.Data()[i] = result;
		}
		return Surface;
	}

	BltSurface& Mul(BltSurface& Surface, uint8_t Param)
	{
		if( Param == 0 )
		{
			Surface.Fill(0);
		}
		else if( Param > 1 )
		{
			for( size_t i = 0; i < Surface.GetWidth()*Surface.GetHeight(); i++ )
			{
				uint16_t result = (uint16_t)Surface.Data()[i] * (uint16_t)Param;
				uint8_t hi = result >> 8;
				uint8_t lo = (uint8_t)result;
				Surface.Data()[i] = lo | -!!hi;
			}
		}
		return Surface;
	}

	BltSurface& Div(BltSurface& Surface, uint8_t Param)
	{
		if( Param )
		{
			for( size_t i = 0; i < Surface.GetWidth()*Surface.GetHeight(); i++ )
			{
				Surface.Data()[i] /= Param;
			}
		}
		return Surface;
	}

	BltSurface& Mod(BltSurface& Surface, uint8_t Param)
	{
		if( Param )
		{
			for( size_t i = 0; i < Surface.GetWidth()*Surface.GetHeight(); i++ )
			{
				Surface.Data()[i] %= Param;
			}
		}
		return Surface;
	}

	// Bitwise
	BltSurface& And(BltSurface& Surface, uint8_t Param)
	{
		for( size_t i = 0; i < Surface.GetWidth()*Surface.GetHeight(); i++ )
		{
			Surface.Data()[i] &= Param;
		}
		return Surface;
	}

	BltSurface& Or(BltSurface& Surface, uint8_t Param)
	{
		if( Param )
		{
			for( size_t i = 0; i < Surface.GetWidth()*Surface.GetHeight(); i++ )
			{
				Surface.Data()[i] |= Param;
			}
		}
		return Surface;
	}

	BltSurface& Xor(BltSurface& Surface, uint8_t Param)
	{
		if( Param )
		{
			for( size_t i = 0; i < Surface.GetWidth()*Surface.GetHeight(); i++ )
			{
				Surface.Data()[i] ^= Param;
			}
		}
		return Surface;
	}

	BltSurface& Not(BltSurface& Surface)
	{
		for( size_t i = 0; i < Surface.GetWidth()*Surface.GetHeight(); i++ )
		{
			Surface.Data()[i] = ~Surface.Data()[i];
		}
		return Surface;
	}

	BltSurface& SHL(BltSurface& Surface, uint8_t Param)
	{
		if( Param )
		{
			for( size_t i = 0; i < Surface.GetWidth()*Surface.GetHeight(); i++ )
			{
				Surface.Data()[i] <<= Param;
			}
		}
		return Surface;
	}

	BltSurface& SHR(BltSurface& Surface, uint8_t Param)
	{
		if( Param )
		{
			for( size_t i = 0; i < Surface.GetWidth()*Surface.GetHeight(); i++ )
			{
				Surface.Data()[i] >>= Param;
			}
		}
		return Surface;
	}

	//// Drawing
	void Blit(BltSurface& Dest, const BltSurface& Source)
	{
		for( BltSize y = 0; y < Source.GetHeight(); y++ )
		{
			for( BltSize x = 0; x < Source.GetWidth(); x++ )
			{
				Dest.SetPixel(x, y, Source.GetPixel(x, y));
			}
		}
	}

	void Blit(BltSurface& Dest, const BltSurface& Source, BltIndex Transparent)
	{
		for( BltSize y = 0; y < Source.GetHeight(); y++ )
		{
			for( BltSize x = 0; x < Source.GetWidth(); x++ )
			{
				if( Source.GetPixel(x, y) != Transparent )
				{
					Dest.SetPixel(x, y, Source.GetPixel(x, y));
				}
			}
		}
	}

	// Generate image
	void Render(const BltSurface& Surface, const BltPalette& Palette, uint8_t* Dest)
	{
		for( BltSize Row = 0; Row < Surface.GetHeight(); Row++ )
		{
			for( BltSize Column = 0; Column < Surface.GetWidth(); Column++ )
			{
				BltColor CurColor = Palette.Colors()[Surface.GetPixel(Column, Row)];
				*Dest++ = CurColor.R;
				*Dest++ = CurColor.G;
				*Dest++ = CurColor.B;
			}
		}
	}

	void Render(const BltSurface& Surface, const BltPalette& Palette, uint8_t* Dest, const uint8_t Transparent)
	{
		for( BltSize Row = 0; Row < Surface.GetHeight(); Row++ )
		{
			for( BltSize Column = 0; Column < Surface.GetWidth(); Column++ )
			{
				BltColor CurColor = Palette.Colors()[Surface.GetPixel(Column, Row)];
				*Dest++ = CurColor.R;
				*Dest++ = CurColor.G;
				*Dest++ = CurColor.B;
				*Dest++ = (Surface.GetPixel(Column, Row) == Transparent) ? 0 : 0xff;
			}
		}
	}

	// Shape Drawing
	BltSurface& Line(BltSurface& Surface, BltPoint From, BltPoint To, BltIndex Index)
	{
		return Line(Surface, From.X, From.Y, To.X, To.Y, Index);
	}
	BltSurface& Line(BltSurface& Surface, BltReal FromX, BltReal FromY, BltReal ToX, BltReal ToY, BltIndex Index)
	{
		BltPoint Delta(ToX - FromX,
			ToY - FromY);
		BltPoint DeltaAbs(Abs(Delta.X), Abs(Delta.Y));
		BltPoint Sign(Sign(Delta.X), Sign(Delta.Y));

		BltPoint Error(DeltaAbs.X >> 1, DeltaAbs.Y >> 1);
		BltPoint Pen = BltPoint(FromX, FromY);

		Surface.SetPixel(Pen, Index);

		if( DeltaAbs.X >= DeltaAbs.Y ) // Horizontal
		{
			for( size_t i = 0; i < (size_t)DeltaAbs.X; i++ )
			{
				Error.Y += DeltaAbs.Y;
				if( Error.Y >= DeltaAbs.X )
				{
					Error.Y -= DeltaAbs.X;
					Pen.Y += Sign.Y;
				}
				Pen.X += Sign.X;
				Surface.SetPixel(Pen, Index);
			}
		}
		else // Vertical
		{
			for( size_t i = 0; i < (size_t)DeltaAbs.Y; i++ )
			{
				Error.X += DeltaAbs.X;
				if( Error.X >= DeltaAbs.Y )
				{
					Error.X -= DeltaAbs.Y;
					Pen.X += Sign.X;
				}
				Pen.Y += Sign.Y;
				Surface.SetPixel(Pen, Index);
			}
		}

		return Surface;
	}

	BltSurface& Fill(BltSurface& Surface,
		BltIndex Index)
	{
		if( Surface.Data() != nullptr )
		{
			memset(Surface.Data(), Index, Surface.GetWidth()*Surface.GetHeight());
		}
		return Surface;
	}

	// Rectangle Drawing
	BltSurface& Rect(BltSurface& Surface,
		BltPoint P1, BltPoint P2,
		BltIndex Index)
	{
		return Rect(Surface, P1.X, P1.Y, P2.X, P2.Y, Index);
	}
	BltSurface& Rect(BltSurface& Surface,
		BltReal FromX, BltReal FromY, BltReal ToX, BltReal ToY,
		BltIndex Index)
	{
		Line(Surface, FromX, FromY, ToX, FromY, Index);// Top
		Line(Surface, FromX, ToY, ToX, ToY, Index);// Bottom
		Line(Surface, FromX, FromY, FromX, ToY, Index);// Left
		Line(Surface, ToX, FromY, ToX, ToY, Index);// Right
		return Surface;
	}

	BltSurface& FillRect(BltSurface& Surface, BltPoint P1, BltPoint P2, BltIndex Index)
	{
		return FillRect(Surface, P1.X, P1.Y, P2.X, P2.Y, Index);
	}
	BltSurface& FillRect(BltSurface& Surface,
		BltReal FromX, BltReal FromY, BltReal ToX, BltReal ToY,
		BltIndex Index)
	{
		for( BltReal Y = Min(FromY, ToY); Y < Max(FromY, ToY); Y++ )
		{
			for( BltReal X = FromX; X < ToX; X++ )
			{
				Surface.SetPixel(X, Y, Index);
			}
		}
		return Surface;
	}

	// Circle Drawing
	BltSurface& Circle(BltSurface& Surface,
		BltPoint Center, BltSize Radius,
		BltIndex Index)
	{
		return Circle(Surface, Center.X, Center.Y, Radius, Index);
	}
	BltSurface& Circle(BltSurface& Surface,
		BltReal CenterX, BltReal CenterY, BltSize Radius,
		BltIndex Index)
	{
		if( Radius )
		{
			BltPoint Offset(0, Radius);
			BltReal Balance = -static_cast<BltReal>(Radius);
			while( Offset.X <= Offset.Y )
			{
				BltPoint P = BltPoint(CenterX, CenterX) - Offset;
				BltPoint W = Offset * 2;;

				Surface.SetPixel(P.X, CenterY + Offset.Y, Index);
				Surface.SetPixel(P.X + W.X, CenterY + Offset.Y, Index);

				Surface.SetPixel(P.X, CenterY - Offset.Y, Index);
				Surface.SetPixel(P.X + W.X, CenterY - Offset.Y, Index);

				Surface.SetPixel(P.Y, CenterY + Offset.X, Index);
				Surface.SetPixel(P.Y + W.Y, CenterY + Offset.X, Index);

				Surface.SetPixel(P.Y, CenterY - Offset.X, Index);
				Surface.SetPixel(P.Y + W.Y, CenterY - Offset.X, Index);

				if( (Balance += Offset.X++ + Offset.X) >= 0 )
				{
					Balance -= --Offset.Y + Offset.Y;
				}
			}
		}
		return Surface;
	}

	BltSurface& FillCircle(BltSurface& Surface,
		BltPoint Center, BltSize Radius,
		BltIndex Index)
	{
		return FillCircle(Surface, Center.X, Center.Y, Radius, Index);
	}
	BltSurface& FillCircle(BltSurface& Surface,
		BltReal CenterX, BltReal CenterY, BltSize Radius,
		BltIndex Index)
	{
		if( Radius )
		{
			BltSize i;
			BltPoint Offset(0, Radius);
			BltReal Balance = -(BltReal)Radius;
			while( Offset.X <= Offset.Y )
			{
				BltPoint P = BltPoint(CenterX, CenterX) - Offset;
				BltPoint W = Offset * 2;

				for( i = 0; i <= static_cast<BltSize>(W.X); i++ )
				{
					Surface.SetPixel(P.X + i, CenterY + Offset.Y, Index);
					Surface.SetPixel(P.X + i, CenterY - Offset.Y, Index);
				}

				for( i = 0; i <= static_cast<BltSize>(W.Y); i++ )
				{
					Surface.SetPixel(P.Y + i, CenterY + Offset.X, Index);
					Surface.SetPixel(P.Y + i, CenterY - Offset.X, Index);
				}

				if( (Balance += Offset.X++ + Offset.X) >= 0 )
				{
					Balance -= --Offset.Y + Offset.Y;
				}
			}
		}
		return Surface;
	}
#else
#endif
}