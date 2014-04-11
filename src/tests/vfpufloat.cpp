#include "main.h"

#include "vfpu.h"
using namespace std;


int main(int argc, char ** argv)
{
	VFPU::Start();
	float result = VFPU::Exec(25,10, VFPU::SUB);
	Debug("%f\n", result);
	VFPU::Halt();

	return 0;
}
