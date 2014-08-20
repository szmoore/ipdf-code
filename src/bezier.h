#ifndef _BEZIER_H
#define _BEZIER_H

#include "real.h"
#include "rect.h"
namespace IPDF
{
	extern int Factorial(int n);
	extern int BinomialCoeff(int n, int k);
	extern Real Bernstein(int k, int n, const Real & u);
	
	inline std::pair<Real,Real> SolveQuadratic(const Real & a, const Real & b, const Real & c)
	{
		Real x0((-b + Sqrt(b*b - Real(4)*a*c))/(Real(2)*a));
		Real x1((-b - Sqrt(b*b - Real(4)*a*c))/(Real(2)*a));
		return std::pair<Real,Real>(x0,x1);
	}

	inline std::vector<Real> SolveCubic(const Real & a, const Real & b, const Real & c, const Real & d)
	{
		// This is going to be a big one...
		// See http://en.wikipedia.org/wiki/Cubic_function#General_formula_for_roots

		// delta = 18abcd - 4 b^3 d + b^2 c^2 - 4ac^3 - 27 a^2 d^2
		/*
		Real discriminant = Real(18) * a * b * c * d - Real(4) * (b * b * b) * d 
				+ (b * b) * (c * c) - Real(4) * a * (c * c * c)
				- Real(27) * (a * a) * (d * d);
		*/
		// discriminant > 0 => 3 distinct, real roots.
		// discriminant = 0 => a multiple root (1 or 2 real roots)
		// discriminant < 0 => 1 real root, 2 complex conjugate roots

		////HACK: We know any roots we care about will be between 0 and 1, so...
		Real maxi(100);
		Real prevRes(d);
		std::vector<Real> roots;
		for(int i = 0; i <= 100; ++i)
		{
			Real x(i);
			x /= maxi;
			Real y = a*(x*x*x) + b*(x*x) + c*x + d;
			if (y == Real(0) || (y < Real(0) && prevRes > Real(0)) || (y > Real(0) && prevRes < Real(0)))
			{
				roots.push_back(x);
			}
		}
		return roots;
			
	}

	/** A _cubic_ bezier. **/
	struct Bezier
	{
		Real x0; Real y0;
		Real x1; Real y1;
		Real x2; Real y2;
		Real x3; Real y3;
		Bezier() = default; // Needed so we can fread/fwrite this struct... for now.
		Bezier(Real _x0, Real _y0, Real _x1, Real _y1, Real _x2, Real _y2, Real _x3, Real _y3) : x0(_x0), y0(_y0), x1(_x1), y1(_y1), x2(_x2), y2(_y2), x3(_x3), y3(_y3) 
		{
			
		}
		
		Bezier(Real _x0, Real _y0, Real _x1, Real _y1, Real _x2, Real _y2) : x0(_x0), y0(_y0), x1(_x1), y1(_y1), x2(_x2), y2(_y2), x3(_x2), y3(_y2) {}
		
		std::string Str() const
		{
			std::stringstream s;
			s << "Bezier{" << Float(x0) << "," << Float(y0) << " -> " << Float(x1) << "," << Float(y1) << " -> " << Float(x2) << "," << Float(y2) << " -> " << Float(x3) << "," << Float(y3) << "}";
			return s.str();
		}
		
		/**
		 * Construct absolute control points using relative control points to a bounding rectangle
		 * ie: If cpy is relative to bounds rectangle, this will be absolute
		 */
		Bezier(const Bezier & cpy, const Rect & t = Rect(0,0,1,1)) : x0(cpy.x0), y0(cpy.y0), x1(cpy.x1), y1(cpy.y1), x2(cpy.x2),y2(cpy.y2), x3(cpy.x3), y3(cpy.y3)
		{
			x0 *= t.w;
			y0 *= t.h;
			x1 *= t.w;
			y1 *= t.h;
			x2 *= t.w;
			y2 *= t.h;
			x3 *= t.w;
			y3 *= t.h;
			x0 += t.x;
			y0 += t.y;
			x1 += t.x;
			y1 += t.y;
			x2 += t.x;
			y2 += t.y;
			x3 += t.x;
			y3 += t.y;
		}

		Rect SolveBounds() const;
		
		Bezier ToAbsolute(const Rect & bounds) const
		{
			return Bezier(*this, bounds);
		}
		
		/** Convert absolute control points to control points relative to bounds
		 * (This basically does the opposite of the Copy constructor)
		 * ie: If this is absolute, the returned Bezier will be relative to the bounds rectangle
		 */
		Bezier ToRelative(const Rect & bounds) const
		{
			// x' <- (x - x0)/w etc
			// special cases when w or h = 0
			// (So can't just use the Copy constructor on the inverse of bounds)
			// Rect inverse = {-bounds.x/bounds.w, -bounds.y/bounds.h, Real(1)/bounds.w, Real(1)/bounds.h};
			Bezier result;
			if (bounds.w == 0)
			{
				result.x0 = 0;
				result.x1 = 0;
				result.x2 = 0;
				result.x3 = 0;
			}
			else
			{
				result.x0 = (x0 - bounds.x)/bounds.w;	
				result.x1 = (x1 - bounds.x)/bounds.w;
				result.x2 = (x2 - bounds.x)/bounds.w;
				result.x3 = (x3 - bounds.x)/bounds.w;
			}

			if (bounds.h == 0)
			{
				result.y0 = 0;
				result.y1 = 0;
				result.y2 = 0;
				result.y3 = 0;
			}
			else
			{
				result.y0 = (y0 - bounds.y)/bounds.h;	
				result.y1 = (y1 - bounds.y)/bounds.h;
				result.y2 = (y2 - bounds.y)/bounds.h;
				result.y3 = (y3 - bounds.y)/bounds.h;
			}
			return result;
		}

		Bezier ReParametrise(const Real& t0, const Real& t1)
		{
			// This function is very, very ugly, but with luck my derivation is correct (even if it isn't optimal, performance wise)
			// (Very) rough working for the derivation is at: http://davidgow.net/stuff/cubic_bezier_reparam.pdf
			Bezier new_bezier;
			Real tdiff = t1 - t0;
			Real tdiff_squared = tdiff*tdiff;
			Real tdiff_cubed = tdiff*tdiff_squared;

			Real t0_squared = t0*t0;
			Real t0_cubed = t0*t0_squared;
			
			// X coordinates
			Real Dx0 = x0 / tdiff_cubed;
			Real Dx1 = x1 / (tdiff_squared - tdiff_cubed);
			Real Dx2 = x2 / (tdiff - Real(2)*tdiff_squared + tdiff_cubed);
			Real Dx3 = x3 / (Real(1) - Real(3)*tdiff + Real(3)*tdiff_squared - tdiff_cubed);

			new_bezier.x3 = Dx3*t0_cubed + Real(3)*Dx3*t0_squared + Real(3)*Dx3*t0 + Dx3 - Dx2*t0_cubed - Real(2)*Dx2*t0_squared - Dx2*t0 + Dx1*t0_cubed + Dx1*t0_squared - Dx0*t0_cubed;
			new_bezier.x2 = Real(3)*Dx0*t0_squared - Real(2)*Dx1*t0 - Real(3)*Dx1*t0_squared + Dx2 + Real(4)*Dx2*t0 + Real(3)*Dx2*t0_squared - Real(3)*Dx3 - Real(6)*Dx3*t0 - Real(3)*Dx3*t0_squared + Real(3)*new_bezier.x3;
			new_bezier.x1 = Real(-3)*Dx0*t0 + Real(3)*Dx1*t0 + Dx1 - Real(2)*Dx2 - Real(3)*Dx2*t0 + Real(3)*Dx3 + Real(3)*Dx3*t0 + Real(2)*new_bezier.x2 - Real(3)*new_bezier.x3;
			new_bezier.x0 = Dx0 - Dx1 + Dx2 - Dx3 + new_bezier.x1 - new_bezier.x2 + new_bezier.x3;

			// Y coordinates
			Real Dy0 = y0 / tdiff_cubed;
			Real Dy1 = y1 / (tdiff_squared - tdiff_cubed);
			Real Dy2 = y2 / (tdiff - Real(2)*tdiff_squared + tdiff_cubed);
			Real Dy3 = y3 / (Real(1) - Real(3)*tdiff + Real(3)*tdiff_squared - tdiff_cubed);

			new_bezier.y3 = Dy3*t0_cubed + Real(3)*Dy3*t0_squared + Real(3)*Dy3*t0 + Dy3 - Dy2*t0_cubed - Real(2)*Dy2*t0_squared - Dy2*t0 + Dy1*t0_cubed + Dy1*t0_squared - Dy0*t0_cubed;
			new_bezier.y2 = Real(3)*Dy0*t0_squared - Real(2)*Dy1*t0 - Real(3)*Dy1*t0_squared + Dy2 + Real(4)*Dy2*t0 + Real(3)*Dy2*t0_squared - Real(3)*Dy3 - Real(6)*Dy3*t0 - Real(3)*Dy3*t0_squared + Real(3)*new_bezier.y3;
			new_bezier.y1 = Real(-3)*Dy0*t0 + Real(3)*Dy1*t0 + Dy1 - Real(2)*Dy2 - Real(3)*Dy2*t0 + Real(3)*Dy3 + Real(3)*Dy3*t0 + Real(2)*new_bezier.y2 - Real(3)*new_bezier.y3;
			new_bezier.y0 = Dy0 - Dy1 + Dy2 - Dy3 + new_bezier.y1 - new_bezier.y2 + new_bezier.y3;


			return new_bezier;
		}
		

		/** Evaluate the Bezier at parametric parameter u, puts resultant point in (x,y) **/
		void Evaluate(Real & x, Real & y, const Real & u) const
		{
			Real coeff[4];
			for (unsigned i = 0; i < 4; ++i)
				coeff[i] = Bernstein(i,3,u);
			x = x0*coeff[0] + x1*coeff[1] + x2*coeff[2] + x3*coeff[3];
			y = y0*coeff[0] + y1*coeff[1] + y2*coeff[2] + y3*coeff[3];
		}

	};



}

#endif //_BEZIER_H
