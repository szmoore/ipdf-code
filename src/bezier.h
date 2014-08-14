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
		Real x0((b + Sqrt(b*b - Real(4)*a*c))/(Real(2)*a));
		Real x1((b - Sqrt(b*b - Real(4)*a*c))/(Real(2)*a));
		return std::pair<Real,Real>(x0,x1);
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
		
		/** Convert absolute control points to control points relative to bounds
		 * (This basically does the opposite of the Copy constructor)
		 * ie: If this is absolute, the returned Bezier will be relative to the bounds rectangle
		 */
		Bezier CopyInverse(const Rect & bounds) const
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
