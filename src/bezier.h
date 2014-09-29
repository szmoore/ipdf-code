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
	
	extern std::vector<Real> SolveQuadratic(const Real & a, const Real & b, const Real & c, const Real & min = 0, const Real & max = 1);

	extern std::vector<Real> SolveCubic(const Real & a, const Real & b, const Real & c, const Real & d, const Real & min = 0, const Real & max = 1, const Real & delta = 1e-9);

	/** A _cubic_ bezier. **/
	struct Bezier
	{
		Real x0; Real y0;
		Real x1; Real y1;
		Real x2; Real y2;
		Real x3; Real y3;
		
		typedef enum {UNKNOWN, LINE, QUADRATIC, CUSP, LOOP, SERPENTINE} Type;
		Type type;
		
		//Bezier() = default; // Needed so we can fread/fwrite this struct... for now.
		Bezier(Real _x0=0, Real _y0=0, Real _x1=0, Real _y1=0, Real _x2=0, Real _y2=0, Real _x3=0, Real _y3=0) : x0(_x0), y0(_y0), x1(_x1), y1(_y1), x2(_x2), y2(_y2), x3(_x3), y3(_y3), type(UNKNOWN)
		{

		}
		
		Type GetType()
		{
			if (type != Bezier::UNKNOWN)
				return type;
			// From Loop-Blinn 2005, with w0 == w1 == w2 == w3 = 1
			// Transformed control points: (a0 = x0, b0 = y0)
			Real a1 = (x1-x0)*Real(3);
			Real a2 = (x0- x1*Real(2) +x2)*Real(3);
			Real a3 = (x3 - x0 + (x1 - x2)*Real(3));
			
			Real b1 = (y1-y0)*Real(3);
			Real b2 = (y0- y1*Real(2) +y2)*Real(3);
			Real b3 = (y3 - y0 + (y1 - y2)*Real(3));
			
			// d vector (d0 = 0 since all w = 1)
			Real d1 = a2*b3 - a3*b2;
			Real d2 = a3*b1 - a1*b3;
			Real d3 = a1*b2 - a2*b1;
			
			if (Abs(d1+d2+d3) < Real(1e-6))
			{
				type = LINE;
				//Debug("LINE %s", Str().c_str());
				return type;
			}
			
			Real delta1 = -(d1*d1);
			Real delta2 = d1*d2;
			Real delta3 = d1*d3 -(d2*d2);
			if (Abs(delta1+delta2+delta3) < Real(1e-6))
			{
				type = QUADRATIC;
				
				//Debug("QUADRATIC %s", Str().c_str());
				return type;
			}
			
			Real discriminant = d1*d3*Real(4) -d2*d2;
			if (Abs(discriminant) < Real(1e-6))
			{
				type = CUSP;
				//Debug("CUSP %s", Str().c_str());
			}
			else if (discriminant > Real(0))
			{
				type = SERPENTINE;
				//Debug("SERPENTINE %s", Str().c_str());
			}
			else
			{
				type = LOOP;
				//Debug("LOOP %s", Str().c_str());
			}
			//Debug("disc %.30f", discriminant);
			return type;
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
			if (bounds.w == Real(0))
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

			if (bounds.h == Real(0))
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
		Bezier DeCasteljauSubdivideLeft(const Real& t)
		{
			Real one_minus_t = Real(1) - t;

			// X Coordinates
			Real x01 = x1*t + x0*one_minus_t;
			Real x12 = x2*t + x1*one_minus_t;
			Real x23 = x3*t + x2*one_minus_t;

			Real x012 = x12*t + x01*one_minus_t;
			Real x123 = x23*t + x12*one_minus_t;

			Real x0123 = x123*t + x012*one_minus_t;

			// Y Coordinates
			Real y01 = y1*t + y0*one_minus_t;
			Real y12 = y2*t + y1*one_minus_t;
			Real y23 = y3*t + y2*one_minus_t;

			Real y012 = y12*t + y01*one_minus_t;
			Real y123 = y23*t + y12*one_minus_t;

			Real y0123 = y123*t + y012*one_minus_t;

			return Bezier(x0, y0, x01, y01, x012, y012, x0123, y0123);
		}
		// Performs one round of De Casteljau subdivision and returns the [t,1] part.
		Bezier DeCasteljauSubdivideRight(const Real& t)
		{
			Real one_minus_t = Real(1) - t;

			// X Coordinates
			Real x01 = x1*t + x0*one_minus_t;
			Real x12 = x2*t + x1*one_minus_t;
			Real x23 = x3*t + x2*one_minus_t;

			Real x012 = x12*t + x01*one_minus_t;
			Real x123 = x23*t + x12*one_minus_t;

			Real x0123 = x123*t + x012*one_minus_t;

			// Y Coordinates
			Real y01 = y1*t + y0*one_minus_t;
			Real y12 = y2*t + y1*one_minus_t;
			Real y23 = y3*t + y2*one_minus_t;

			Real y012 = y12*t + y01*one_minus_t;
			Real y123 = y23*t + y12*one_minus_t;

			Real y0123 = y123*t + y012*one_minus_t;

			return Bezier(x0123, y0123, x123, y123, x23, y23, x3, y3);
		}

		Bezier ReParametrise(const Real& t0, const Real& t1)
		{
			//Debug("Reparametrise: %f -> %f",Double(t0),Double(t1));
			Bezier new_bezier;
			// Subdivide to get from [0,t1]
			new_bezier = DeCasteljauSubdivideLeft(t1);
			// Convert t0 from [0,1] range to [0, t1]
			Real new_t0 = t0 / t1;
			//Debug("New t0 = %f", Double(new_t0));
			new_bezier = new_bezier.DeCasteljauSubdivideRight(new_t0);

			//Debug("%s becomes %s", this->Str().c_str(), new_bezier.Str().c_str());
			return new_bezier;
		}
		
		std::vector<Bezier> ClipToRectangle(const Rect& r)
		{
			// Find points of intersection with the rectangle.
			Debug("Clipping Bezier to Rect %s", r.Str().c_str());


			// Find its roots.
			std::vector<Real> x_intersection = SolveXParam(r.x);
			//Debug("Found %d intersections on left edge", x_intersection.size());

			// And for the other side.

			std::vector<Real> x_intersection_pt2 = SolveXParam(r.x + r.w);
			x_intersection.insert(x_intersection.end(), x_intersection_pt2.begin(), x_intersection_pt2.end());
			//Debug("Found %d intersections on right edge (total x: %d)", x_intersection_pt2.size(), x_intersection.size());

			// Find its roots.
			std::vector<Real> y_intersection = SolveYParam(r.y);
			//Debug("Found %d intersections on top edge", y_intersection.size());

			std::vector<Real> y_intersection_pt2 = SolveYParam(r.y+r.h);
			y_intersection.insert(y_intersection.end(), y_intersection_pt2.begin(), y_intersection_pt2.end());
			//Debug("Found %d intersections on bottom edge (total y: %d)", y_intersection_pt2.size(), y_intersection.size());

			// Merge and sort.
			x_intersection.insert(x_intersection.end(), y_intersection.begin(), y_intersection.end());
			x_intersection.push_back(Real(0));
			x_intersection.push_back(Real(1));
			std::sort(x_intersection.begin(), x_intersection.end());

			//Debug("Found %d intersections.\n", x_intersection.size());
			/*for(auto t : x_intersection)
			{
				Real ptx, pty;
				Evaluate(ptx, pty, t);
				Debug("Root: t = %f, (%f,%f)", Double(t), Double(ptx), Double(pty));
			}*/
			
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
				//Debug(" -- t0: %f to t1: %f: %f", Double(t0), Double(t1), Double((t1 + t0)/Real(2)));
				Real ptx, pty;
				Evaluate(ptx, pty, ((t1 + t0) / Real(2)));
				if (r.PointIn(ptx, pty))
				{
					//Debug("Adding segment: (point at %f, %f)", Double(ptx), Double(pty));
					all_beziers.push_back(this->ReParametrise(t0, t1));
				}
				else
				{
					//Debug("Segment removed (point at %f, %f)", Double(ptx), Double(pty));
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
		std::vector<Vec2> Evaluate(const std::vector<Real> & u) const;
		
		std::vector<Real> SolveXParam(const Real & x) const;
		std::vector<Real> SolveYParam(const Real & x) const;
		
		// Get points with same X
		inline std::vector<Vec2> SolveX(const Real & x) const
		{
			return Evaluate(SolveXParam(x));
		}
		// Get points with same Y
		inline std::vector<Vec2> SolveY(const Real & y) const
		{
			return Evaluate(SolveYParam(y));
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

#undef Real

#endif //_BEZIER_H
