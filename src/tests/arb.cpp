#include <cstdlib>
#include <cstdio>
#include <iostream>

#include "arbint.cpp"

using namespace std;
using namespace IPDF;

int main(int argc, char ** argv)
{
	Arbint a(4294967296L);
	printf("%s\n", a.Str().c_str());
}
