#include "main.h"

#include <bitset>

using namespace std;



int main(int argc, char ** argv)
{
	char buffer[BUFSIZ];
	double input;
	printf("Enter a double: ");
	fgets(buffer, BUFSIZ, stdin);
	sscanf(buffer, "%lf", &input);


	float f = (float)(input);

	unsigned long long i;
	memcpy(&i, &f, 4);
	bitset<32> b32(i);
	memcpy(&i, &input, 8);
	bitset<64> b64(i);

	printf("\nAs float: %s\n", b32.to_string().c_str());
	printf("\nAs double: %s\n", b64.to_string().c_str());
	#ifdef REAL_BITSET
		Real r(input);
		printf("\nAs real: %s\n", r.repr.to_string().c_str());
	#endif //REAL_BITSET
	
}
