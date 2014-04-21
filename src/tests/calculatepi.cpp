/**
 * Tester
 * Calculates PI using integration
 * Compares results obtained with float, double, and Real.
 */
#include "main.h"
#include <cmath>
#include <ctime> // for performance measurements

/** Function to integrate
 *
 */
template <class T> T f(const T & x)
{
	// Use reference because Real might get big
	return T(4.0) / (T(1.0) + x*x);
}

/**
 * Integrate f using the simpson rule
 */
template <class T> T Integrate(T(*f)(const T & ), const T & xmin, const T & xmax, uint64_t intervals)
{
	T sum = 0.0;
	T dx = (xmax - xmin) / T(intervals);
	
	T odd = 0.0;
	T even = 0.0;
	uint64_t i = 0; 
	for (i = 1; i < intervals-1; i+=2)
		odd += f(xmin + dx*T(i));
	for (i = 2; i < intervals-1; i+=2)
		even += f(xmin + dx*T(i));
	sum = (f(xmin) + T(4.0)*odd + T(2.0)*even + f(xmin+T(intervals)*dx))*dx / T(3.0);
	return sum;

}

#define MAX_INTERVALS 1e10
/**
 * main
 */
int main(int argc, char ** argv)
{
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	long double PI = acosl(-1.0L);
	printf("# intervals\terror_float\tclock_float\terror_double\tclock_double\terror_long\tclock_long");
	#if REAL > REAL_LONG_DOUBLE
		printf("\terror_real\tclock_real\n");
	#else
		printf("\n");
	#endif //REAL
	for (uint64_t intervals = 1; intervals < (uint64_t)(MAX_INTERVALS); intervals*=5)
	{
		clock_t start = clock();
		float error_float = Integrate<float>(f<float>, 0.0f, 1.0f, intervals) - float(PI);
		clock_t clock_float = clock() - start;
		start = clock();
		double error_double = Integrate<double>(f<double>, 0.0, 1.0, intervals) - double(PI);
		clock_t clock_double = clock() - start;
		start = clock();
		long double error_long = Integrate<long double>(f<long double>,0.0L,1.0L, intervals) - PI;
		clock_t clock_long = clock() - start;

		printf("%lu\t%.30f\t%li\t%.30lf\t%li\t%.30llf\t%li", intervals, error_float, clock_float, error_double, clock_double, error_long, clock_long);
		
		#if REAL > REAL_LONG_DOUBLE
			Real error_real = Integrate<Real>(f<Real>,Real(0.0L), Real(1.0L), intervals) - Real(PI);
			clock_t clock_real = clock() - start;	
			printf("\t%.30lf\t%li\n", Float(error_real), clock_real);
		#else
			printf("\n");
		#endif
	}
}

