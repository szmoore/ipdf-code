#ifndef _BEZIER_H
#define _BEZIER_H

#include "real.h"
#include "rect.h"
namespace IPDF
{
	extern int Factorial(int n);
	extern int BinomialCoeff(int n, int k);
	extern Real Bernstein(int k, int n, const Real & u);

	/** A _quadratic_ bezier. **/
	struct Bezier
	{
		Real x0; Real y0;
		Real x1; Real y1;
		Real x2; Real y2;
		Bezier() = default; // Needed so we can fread/fwrite this struct... for now.
		Bezier(Real _x0, Real _y0, Real _x1, Real _y1, Real _x2, Real _y2) : x0(_x0), y0(_y0), x1(_x1), y1(_y1), x2(_x2), y2(_y2) {}
		std::string Str() const
		{
			std::stringstream s;
			s << "Bezier{" << Float(x0) << "," << Float(y0) << " -> " << Float(x1) << "," << Float(y1) << " -> " << Float(x2) << "," << Float(y2) << "}";
			return s.str();
		}
		Bezier(const Bezier & cpy, const Rect & t = Rect(0,0,1,1)) : x0(cpy.x0+t.x), y0(cpy.y0+t.y), x1(cpy.x1+t.x), y1(cpy.y1+t.y), x2(cpy.x2+t.x),y2(cpy.y2+t.y)
		{
			x1 = x0 + (x1-x0)*t.w;
			y1 = y0 + (y1-y0)*t.h;
			x2 = x0 + (x2-x0)*t.w;
			y2 = y0 + (y2-y0)*t.h;
		}

		Rect ToRect() {return Rect(x0,y0,x2-x0,y2-y0);}

		/** Evaluate the Bezier at parametric parameter u, puts resultant point in (x,y) **/
		void Evaluate(Real & x, Real & y, const Real & u)
		{
			Real coeff[3];
			for (unsigned i = 0; i < 3; ++i)
				coeff[i] = Bernstein(i,2,u);
			x = x0*coeff[0] + x1*coeff[1] + x2*coeff[2];
			y = y0*coeff[0] + y1*coeff[1] + y2*coeff[2];
		}

	};



}

#endif //_BEZIER_H
