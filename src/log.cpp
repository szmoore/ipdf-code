/**
 * @file log.cpp
 * @brief Implement logging and error handling functions
 */


// --- Custom headers --- //
#include "common.h"

#include <cstdio>
#include <unistd.h>
#include <stdarg.h>
#ifndef __MINGW32__
#include <execinfo.h>
#endif

#ifdef LOG_SYSLOG

static void InitSyslog()
{
	static bool init = false;
	if (!init)
	{
		openlog(Options::program_name, LOG_PERROR | LOG_PID, LOG_USER);
		init = true;
	}
}

#endif //LOG_SYSLOG

static const char * unspecified_funct = "???";



/**
 * Print a message to stderr and log it via syslog. The message must be
 * less than BUFSIZ characters long, or it will be truncated.
 * @param level - Specify how severe the message is.
	If level is higher (less urgent) than the program's verbosity (see options.h) no message will be printed
 * @param funct - String indicating the function name from which this function was called.
	If this is NULL, Log will show the unspecified_funct string instead
 * @param file - Source file containing the function
 * @param line - Line in the source file at which Log is called
 * @param fmt - A format string
 * @param ... - Arguments to be printed according to the format string
 */
void LogEx(int level, const char * funct, const char * file, int line, ...)
{
	//Todo: consider setlogmask(3) to filter messages
	const char *fmt;
	char buffer[BUFSIZ];
	va_list va;

	// Don't print the message unless we need to
	//if (level > Globals::Verbosity())
	//	return;

	va_start(va, line);
	fmt = va_arg(va, const char*);
	
	if (fmt == NULL) // sanity check
		Fatal("Format string is NULL");

	vsnprintf(buffer, BUFSIZ, fmt, va);
	va_end(va);

	if (funct == NULL)
		funct = unspecified_funct;

	// Make a human readable severity string
	const char *severity;
	switch (level)
	{
		case LOG_ERR:
			severity = "ERROR";
			break;
		case LOG_WARNING:
			severity = "WARNING";
			break;
		case LOG_NOTICE:
			severity = "notice";
			break;
		case LOG_INFO:
			severity = "info";
			break;
		default:
			severity = "debug";
			break;
	}

	#ifdef LOG_SYSLOG
		InitSyslog();
		syslog(level, "%s: %s (%s:%d) - %s", severity, funct, file, line, buffer);
	#else
		//fprintf(stderr, "%s[%d]: %s: %s (%s:%d) - %s\n", Options::program_name, getpid(), severity, funct, file, line, buffer);
		fprintf(stderr, "%s: %s (%s:%d) - %s\n", severity, funct, file, line, buffer);
	#endif //LOG_SYSLOG
	
}

/**
 * Handle a Fatal error in the program by printing a message and exiting the program
 *	CALLING THIS FUNCTION WILL CAUSE THE PROGAM TO EXIT
 * @param funct - Name of the calling function
 * @param file - Name of the source file containing the calling function
 * @param line - Line in the source file at which Fatal is called
 * @param fmt - A format string
 * @param ... - Arguments to be printed according to the format string
 */
void FatalEx(const char * funct, const char * file, int line, ...)
{
	const char *fmt;
	char buffer[BUFSIZ];
	va_list va;
	va_start(va, line);
	fmt = va_arg(va, const char*);
	
	if (fmt == NULL)
	{
		// Fatal error in the Fatal function.
		// (This really shouldn't happen unless someone does something insanely stupid)
		Fatal("Format string is NULL");
		return; // Should never get here
	}

	vsnprintf(buffer, BUFSIZ, fmt,va);
	va_end(va);

	if (funct == NULL)
		funct = unspecified_funct;

	#ifdef LOG_SYSLOG
		InitSyslog();
		syslog(LOG_CRIT, "FATAL: %s (%s:%d) - %s", funct, file, line, buffer);
	#else
		//fprintf(stderr, "%s[%d]: FATAL: %s (%s:%d) - %s\n", Options::program_name, getpid(), funct, file, line, buffer);
		fprintf(stderr, "FATAL: %s (%s:%d) - %s\n", funct, file, line, buffer);

	#endif //LOG_SYSLOG

	exit(EXIT_FAILURE);
}


/**
 * Print a backtrace
 */
void Backtrace(int size)
{
	#ifndef __MINGW32__
	void * buffer[100];
	int actual_size = backtrace(buffer, size);
	backtrace_symbols_fd(buffer, actual_size, fileno(stderr));
	#else
		Error("Backtrace not supported by compiler");
	#endif
}

