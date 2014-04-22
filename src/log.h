/**
 * @file log.h
 * @brief Declaration of functions for printing log messages and/or terminating program after a fatal error
 */

#ifndef _LOG_H
#define _LOG_H

#include <cstdio>
#include <cstdlib>
#include <string>

inline std::string methodName(const std::string& prettyFunction)
{
    size_t colons = prettyFunction.find("::");
    size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
    size_t end = prettyFunction.rfind("(") - begin;

    return prettyFunction.substr(begin,end) + "()";
}

#define __METHOD_NAME__ methodName(__PRETTY_FUNCTION__).c_str()

//#define LOG_SYSLOG

#ifdef LOG_SYSLOG
	#include <syslog.h>
#else
	enum {LOG_EMERG, LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG};
#endif //LOG_SYSLOG

//To get around a 'pedantic' C99 rule that you must have at least 1 variadic arg, combine fmt into that.
#define Log(level, ...) LogEx(level, __METHOD_NAME__, __FILE__, __LINE__, __VA_ARGS__)
#define Fatal(...) FatalEx(__METHOD_NAME__, __FILE__, __LINE__, __VA_ARGS__)

#define Debug(...) LogEx(LOG_DEBUG, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define Error(...) LogEx(LOG_ERR, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define Warn(...) LogEx(LOG_WARNING, __func__, __FILE__, __LINE__, __VA_ARGS__)


extern void LogEx(int level, const char * funct, const char * file, int line,  ...); // General function for printing log messages to stderr
extern void FatalEx(const char * funct, const char * file, int line, ...); // Function that deals with a fatal error (prints a message, then exits the program).

#endif //_LOG_H

//EOF
