#include "main.h"
float test = 1e4;
int main(int argc, char ** argv)
{
	Real r(test);
	Debug("test float %.20f", test);
	Debug("test real %.20f", Float(r));
	return 0;
}
