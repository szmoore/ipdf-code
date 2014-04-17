#include "main.h"

#include "vfpu.h"
using namespace std;


int main(int argc, char ** argv)
{
	if (argc > 1)
		VFPU::Start(argv[1]);
	else
		VFPU::Start();
	float result = VFPU::Exec(25,10, VFPU::SUB);
	Debug("%f\n", result);
	VFPU::Halt();

	return 0;
}
