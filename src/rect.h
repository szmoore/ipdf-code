#ifndef _RECT_H
#define _RECT_H

#include "common.h"
#include "real.h"

namespace IPDF
{
	template <class T = IPDF::Real>
	struct TRect
	{
		T x; T y; T w; T h;
		//TRect() = default; // Needed so we can fread/fwrite this struct
		TRect(T _x=0, T _y=0, T _w=1, T _h=1) : x(_x), y(_y), w(_w), h(_h) {}
		template <class B> TRect(const TRect<B> & cpy) : x(T(cpy.x)), y(T(cpy.y)), w(T(cpy.w)), h(T(cpy.h)) {}
		
		std::string Str() const
		{
			std::stringstream s;
			// float conversion needed because it is fucking impossible to get ostreams working with template classes
			s << "{" << x << ", " << y << ", " << (x + w) << ", " << (y + h) << " (w: " << w <<", h: " << h <<")}";
			return s.str();
		}
		inline bool PointIn(T pt_x, T pt_y) const
		{
			if (pt_x <= x) return false;
			if (pt_y <= y) return false;
			if (pt_x >= x + w) return false;
			if (pt_y >= y + h) return false;
			return true;
		}

		inline bool Intersects(const TRect& other) const
		{
			if (x + w < other.x) return false;
			if (y + h < other.y) return false;
			if (x > other.x + other.w) return false;
			if (y > other.y + other.h) return false;
			return true;
		}

		inline bool Contains(const TRect& other) const
		{
			return PointIn(other.x, other.y) && PointIn(other.x + other.w, other.y + other.h);
		}
		
		template <class B> TRect<B> Convert() const {return TRect<B>(B(x), B(y), B(w), B(h));}
		
		template <class B> TRect<T> & operator=(const TRect<B> & equ)
		{
			x = T(equ.x);
			y = T(equ.y);
			w = T(equ.w);
			h = T(equ.h);
			return *this;
		}

		// Clips "other" to "this"
		inline TRect Clip(const TRect& other) const
		{
			TRect clipped = other;
			if (clipped.x < x)
			{
				clipped.w += clipped.x - x;
				clipped.x = x;
			}
			if (clipped.y < y)
			{
				clipped.h += clipped.y - y;
				clipped.y = 0;
			}
			if (clipped.x + clipped.w > x + w)
			{
				clipped.w += ((x + w) - (clipped.x + clipped.w));
			}
			if (clipped.y + clipped.h > y + h)
			{
				clipped.h += ((y + h) - (clipped.y + clipped.h));
			}
			return clipped;
		}
	};



	template <class T = IPDF::Real>
	inline TRect<T> TransformRectCoordinates(const TRect<T> & view, const TRect<T> & r)
	{
		TRect<T> out;
		T w = (view.w == T(0))?T(1):view.w;
		T h = (view.h == T(0))?T(1):view.h;
		out.x = (r.x - view.x) / w; //r.x = out.x *w + view.x
		out.y = (r.y - view.y) / h; // r.y = out.y*h + view.y
		out.w = r.w / w; // r.w = out.w * w
		out.h = r.h / h; // r.h = out.h * h
		return out;
	}


	template <class T = IPDF::Real>
	inline Vec2 TransformPointCoordinates(const TRect<T> & view, const Vec2& v)
	{
		Vec2 out;
		out.x = (v.x - view.x) / view.w;
		out.y = (v.y - view.y) / view.h;
		return out;
	}
	
	typedef TRect<Real> Rect;
	

}

#endif //_RECT_H
