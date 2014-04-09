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

/**
 * Tell the VFPU to execute an instruction, wait for it to finish, return the result
 * TODO: Generalise for non 32bit Registers
 */
Register Exec(const Register & opa, const Register &  opb, Opcode op)
{
	assert(g_running);
	
	// Copy floats into 32 bits (casting will alter the representation) 
	unsigned a; memcpy(&a, &opa, 8);
	unsigned b; memcpy(&b, &opb, 8);

	
	char buffer[BUFSIZ];
	int len = sprintf(buffer, "%08x\n%08x\n%03x\n",a, b, op);  // This is... truly awful... why am I doing this
	//fprintf(stderr, "Writing:\n%s", buffer);

	assert(len == 9+9+4); 
	assert(write(g_fpu_socket[1], buffer, len) == len);
	//fprintf(stderr, "Wrote!\n");

	len = read(g_fpu_socket[1], buffer, sizeof(buffer));
	assert(len == 9);
	buffer[len] = '\0';
	
	
	unsigned result = 0x00000000;
	for (int i = 0; i < len/2; ++i)
	{
		unsigned byte2; // cos its two bytes
		sscanf(buffer+2*i, "%02x", &byte2);
		result |= (byte2 << 8*(len/2-i-1));
	}
	
	//fprintf(stderr, "Buffer: %s\nResult: %08x\n", buffer, result);
	
	Register r;
	memcpy(&r, &result, 8); // Amazing.
	return r;
}

}


