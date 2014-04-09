#include "vfpu.h"
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>

#include <sstream>
#include <iomanip>

using namespace std;

namespace VFPU
{

static const char g_fpu[] = "../vhdl/tb_fpu";

static bool g_running = false;
static int g_fpu_socket[2] = {-1,-1};
static pid_t g_fpu_pid = -1;

/**
 * Starts the VFPU
 * @returns 0 on success, errno of the failing function on failure
 */
int Start()
{
	assert(!g_running);
	// create unix socket pair
	
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, g_fpu_socket) != 0)
		return errno;
	

	g_fpu_pid = fork();
	if (g_fpu_pid < 0) // error check
		return errno;


	// Child branch
	if (g_fpu_pid == 0)
	{
		
		// Remap stdio to the socket
		dup2(g_fpu_socket[0],fileno(stdin));
		dup2(g_fpu_socket[0],fileno(stdout));
		dup2(open("/dev/null", O_APPEND), fileno(stderr)); //LALALA I AM NOT LISTENING TO YOUR STUPID ERRORS GHDL
		
		// Unbuffer things; buffers are a pain
		setbuf(stdin, NULL); setbuf(stdout, NULL); setbuf(stderr, NULL);

		//fprintf(stderr, "Goodbye!\n");
		execl(g_fpu, g_fpu,NULL);
		fprintf(stderr, "Uh oh! %s\n", strerror(errno)); // We will never see this if something goes wrong... oh dear
		exit(errno); // Child exits here.
	}

	// Parent branch
	usleep(100);
	g_running = true; // We are ready!
	return 0;
}

/**
 * Halt the VFPU
 */
int Halt()
{
	assert(g_running);
	// Tell the child to stop running the VHDL simulation
	if (close(g_fpu_socket[1]) != 0)
		return errno;
	usleep(1000);
	if (kill(g_fpu_pid, SIGKILL) != 0)
		return errno;
	g_running = false;
	return 0;
}

float Exec(float opa, float opb, Opcode op)
{
	unsigned a; memcpy(&a, &opa, sizeof(float));
	unsigned b; memcpy(&b, &opb, sizeof(float));
	
	unsigned r = (unsigned)(Exec(Register(a), Register(b), op).to_ulong());
	float result; memcpy(&result, &r, sizeof(float));
	return result;
}

/**
 * Tell the VFPU to execute an instruction, wait for it to finish, return the result
 * TODO: Make this not mix C++ and C so badly?
 */
Register Exec(const Register & a, const Register &  b, Opcode op)
{
	assert(g_running);
		
	stringstream s;
	s << hex << a.to_ullong() << "\n" << b.to_ullong() << "\n" << setw(3) << setfill('0') << op << "\n";
	string str(s.str());
	//fprintf(stderr, "Writing:\n%s\n", str.c_str());

	// So we used C++ streams to make our C string...
	assert(write(g_fpu_socket[1], str.c_str(), str.size()) == (int)str.size());
	//fprintf(stderr, "Wrote!\n");

	char buffer[BUFSIZ]; 
	int len = read(g_fpu_socket[1], buffer, sizeof(buffer));
	//assert(len == 9);
	buffer[--len] = '\0'; // Get rid of newline
	//fprintf(stderr, "Read!\n");
	
	Register result(0);
	for (int i = 0; i < len/2; ++i)
	{
		unsigned byte; // It is ONE byte (2 nibbles == 2 hex digits)
		sscanf(buffer+2*i, "%02x", &byte);
		result |= (byte << 8*(len/2-i-1));
	}
	return result;
}

}


