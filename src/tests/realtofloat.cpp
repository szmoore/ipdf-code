#include "main.h"
#include <cmath>
float test_min = 0;
float test_max = 1e-6;
unsigned test_count = 100;
float error_thresh = (test_max - test_min)/1e1;
int main(int argc, char ** argv)
{
	Debug("TEST STARTING - Comparing Float(Real(test)) for %u trials between %.20f and %.20f", test_count, test_min, test_max);
	float error;
	for (unsigned i = 0; i < test_count; ++i)
	{
		float test = test_min + (test_max-test_min)*((float)(rand() % (int)1e6)/1e6);
		Real real(test);
		float thiserror = abs(test - real.value);
		error += thiserror;
		Debug("#%u: |test %.20f - real %.20f| = %.20f [mean %f]", i, test, real.value, thiserror, error/(i+1));
	}

	if (error/test_count > error_thresh)
	{
		Fatal("TEST FAILED - Average error %.20f exceeds threshold %.20f", error/test_count, error_thresh);
	}
	Debug("TEST SUCCEEDED - Average error %.20f is %f percent of range", error/test_count, 1e2*(error/test_count) / (test_max - test_min));
	return 0;
}
