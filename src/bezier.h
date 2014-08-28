#ifndef _BEZIER_H
#define _BEZIER_H

#include <vector>
#include <algorithm>

#include "real.h"
#include "rect.h"
namespace IPDF
{
	extern int Factorial(int n);
	extern int BinomialCoeff(int n, int k);
	extern Real Bernstein(int k, int n, const Real & u);
	extern std::pair<Real,Real> BezierTurningPoints(const Real & p0, const Real & p1, const Real & p2, const Real & p3);
	
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

		std::vector<Real> roots;
		// delta = 18abcd - 4 b^3 d + b^2 c^2 - 4ac^3 - 27 a^2 d^2
		
#if 0
		Real discriminant = Real(18) * a * b * c * d - Real(4) * (b * b * b) * d 
				+ (b * b) * (c * c) - Real(4) * a * (c * c * c)
				- Real(27) * (a * a) * (d * d);
		
		Debug("Trying to solve %fx^3 + %fx^2 + %fx + %f (Discriminant: %f)", a,b,c,d, discriminant);
		// discriminant > 0 => 3 distinct, real roots.
		// discriminant = 0 => a multiple root (1 or 2 real roots)
		// discriminant < 0 => 1 real root, 2 complex conjugate roots

		Real delta0 = (b*b) - Real(3) * a * c;
		Real delta1 = Real(2) * (b * b * b) - Real(9) * a * b * c + Real(27) * (a * a) * d;


		Real C = pow((delta1 + Sqrt((delta1 * delta1) - 4 * (delta0 * delta0 * delta0)) ) / Real(2), 1/3);

		if (false && discriminant < 0)
		{
			Real real_root = (Real(-1) / (Real(3) * a)) * (b + C + delta0 / C);

			roots.push_back(real_root);

			return roots;

		}
#endif
		////HACK: We know any roots we care about will be between 0 and 1, so...
		Real maxi(100);
		Real prevRes(d);
		for(int i = 0; i <= 100; ++i)
		{
			Real x(i);
			x /= maxi;
			Real y = a*(x*x*x) + b*(x*x) + c*x + d;
			if (((y < Real(0)) && (prevRes > Real(0))) || ((y > Real(0)) && (prevRes < Real(0))))
			{
				Debug("Found root of %fx^3 + %fx^2 + %fx + %f at %f (%f)", a, b, c, d, x, y);
				roots.push_back(x);
			}
			prevRes = y;
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
		
		typedef enum {LINE, QUADRATIC, CUSP, LOOP, SERPENTINE} Type;
		Type type;
		
		Bezier() = default; // Needed so we can fread/fwrite this struct... for now.
		Bezier(Real _x0, Real _y0, Real _x1, Real _y1, Real _x2, Real _y2, Real _x3, Real _y3) : x0(_x0), y0(_y0), x1(_x1), y1(_y1), x2(_x2), y2(_y2), x3(_x3), y3(_y3) 
		{
			//TODO: classify the curve
			type = SERPENTINE;
		}
		
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
		Bezier(const Bezier & cpy, const Rect & t = Rect(0,0,1,1)) : x0(cpy.x0), y0(cpy.y0), x1(cpy.x1), y1(cpy.y1), x2(cpy.x2),y2(cpy.y2), x3(cpy.x3), y3(cpy.y3), type(cpy.type)
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
		
		std::pair<Real,Real> GetTop() const;
		std::pair<Real,Real> GetBottom() const;
		std::pair<Real,Real> GetLeft() const;
		std::pair<Real,Real> GetRight() const;
		
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

		// Performs one round of De Casteljau subdivision and returns the [t,1] part.
		Bezier DeCasteljauSubdivideRight(const Real& t)
		{
			Real one_minus_t = Real(1) - t;

			// X Coordinates
			Real x01 = x0*t + x1*one_minus_t;
			Real x12 = x1*t + x2*one_minus_t;
			Real x23 = x2*t + x3*one_minus_t;

			Real x012 = x01*t + x12*one_minus_t;
			Real x123 = x12*t + x23*one_minus_t;

			Real x0123 = x012*t + x123*one_minus_t;

			// Y Coordinates
			Real y01 = y0*t + y1*one_minus_t;
			Real y12 = y1*t + y2*one_minus_t;
			Real y23 = y2*t + y3*one_minus_t;

			Real y012 = y01*t + y12*one_minus_t;
			Real y123 = y12*t + y23*one_minus_t;

			Real y0123 = y012*t + y123*one_minus_t;

			return Bezier(x0, y0, x01, y01, x012, y012, x0123, y0123);
		}
		// Performs one round of De Casteljau subdivision and returns the [0,t] part.
		Bezier DeCasteljauSubdivideLeft(const Real& t)
		{
			Real one_minus_t = Real(1) - t;

			// X Coordinates
			Real x01 = x0*t + x1*one_minus_t;
			Real x12 = x1*t + x2*one_minus_t;
			Real x23 = x2*t + x3*one_minus_t;

			Real x012 = x01*t + x12*one_minus_t;
			Real x123 = x12*t + x23*one_minus_t;

			Real x0123 = x012*t + x123*one_minus_t;

			// Y Coordinates
			Real y01 = y0*t + y1*one_minus_t;
			Real y12 = y1*t + y2*one_minus_t;
			Real y23 = y2*t + y3*one_minus_t;

			Real y012 = y01*t + y12*one_minus_t;
			Real y123 = y12*t + y23*one_minus_t;

			Real y0123 = y012*t + y123*one_minus_t;

			return Bezier(x0123, y0123, x123, y123, x23, y23, x3, y3);
		}

		Bezier ReParametrise(const Real& t0, const Real& t1)
		{
			Debug("Reparametrise: %f -> %f",t0,t1);
			Bezier new_bezier;
			// Subdivide to get from [0,t1]
			new_bezier = DeCasteljauSubdivideLeft(t1);
			// Convert t0 from [0,1] range to [0, t1]
			Real new_t0 = t0 / t1;
			Debug("New t0 = %f", new_t0);
			new_bezier = new_bezier.DeCasteljauSubdivideRight(new_t0);

			Debug("%s becomes %s", this->Str().c_str(), new_bezier.Str().c_str());
			return new_bezier;
		}
		
		std::vector<Bezier> ClipToRectangle(const Rect& r)
		{
			// Find points of intersection with the rectangle.
			Debug("Clipping Bezier to Rect %s", r.Str().c_str());

			// Convert bezier coefficients -> cubic coefficients
			Real xd = x0 - r.x;
			Real xc = Real(3)*(x1 - x0);
			Real xb = Real(3)*(x2 - x1) - xc;
			Real xa = x3 - x0 - xc - xb;

			// Find its roots.
			std::vector<Real> x_intersection = SolveCubic(xa, xb, xc, xd);

			// And for the other side.
			xd = x0 - r.x - r.w;

			std::vector<Real> x_intersection_pt2 = SolveCubic(xa, xb, xc, xd);
			x_intersection.insert(x_intersection.end(), x_intersection_pt2.begin(), x_intersection_pt2.end());

			// Similarly for y-coordinates.
			// Convert bezier coefficients -> cubic coefficients
			Real yd = y0 - r.y;
			Real yc = Real(3)*(y1 - y0);
			Real yb = Real(3)*(y2 - y1) - yc;
			Real ya = y3 - y0 - yc - yb;

			// Find its roots.
			std::vector<Real> y_intersection = SolveCubic(ya, yb, yc, yd);

			// And for the other side.
			yd = y0 - r.y - r.h;

			std::vector<Real> y_intersection_pt2 = SolveCubic(ya, yb, yc, yd);
			y_intersection.insert(y_intersection.end(), y_intersection_pt2.begin(), y_intersection_pt2.end());

			// Merge and sort.
			x_intersection.insert(x_intersection.end(), y_intersection.begin(), y_intersection.end());
			x_intersection.push_back(Real(0));
			x_intersection.push_back(Real(1));
			std::sort(x_intersection.begin(), x_intersection.end());

			Debug("Found %d intersections.\n", x_intersection.size());
			
			std::vector<Bezier> all_beziers;
			if (x_intersection.size() <= 2)
			{
				all_beziers.push_back(*this);
				return all_beziers;
			}
			Real t0 = *(x_intersection.begin());
			for (auto it = x_intersection.begin()+1; it != x_intersection.end(); ++it)
			{
				Real t1 = *it;
				if (t1 == t0) continue;
				Debug(" -- t0: %f to t1: %f", t0, t1);
				Real ptx, pty;
				Evaluate(ptx, pty, ((t1 + t0) / Real(2)));
				if (r.PointIn(ptx, pty))
				{
					all_beziers.push_back(this->ReParametrise(t0, t1));
				}
				else
				{
					Debug("Segment removed (point at %f, %f)", ptx, pty);
				}
				t0 = t1;
			}
			return all_beziers;
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
		
		bool operator==(const Bezier & equ) const
		{
			return (x0 == equ.x0 && y0 == equ.y0
				&&  x1 == equ.x1 && y1 == equ.y1
				&&	x2 == equ.x2 && y2 == equ.y2
				&&	x3 == equ.x3 && y3 == equ.y3);
		}
		bool operator!=(const Bezier & equ) const {return !this->operator==(equ);}

	};



}

#endif //_BEZIER_H
