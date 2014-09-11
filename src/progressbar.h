#include <sys/time.h>
#include <unistd.h>
#include <cstdio>

/**
 * Print a progress bar to stderr + elapsed time since execution
 * Stolen from some of my Codejam stuff, but modified to actually print the correct times...
 */
inline void ProgressBar(int64_t c, int64_t nCases, int len=20)
{
	static timeval startTime;
	static timeval thisTime;
	static timeval lastTime;
	static int64_t total_usec = 0;
	gettimeofday(&thisTime, NULL);
	if (c <= 0)
	{
		startTime = thisTime;
		lastTime = thisTime;
		total_usec = 0;
	}

	
	fprintf(stderr, "\33[2K\r");
	//fprintf(stderr, "\n");
	
	fprintf(stderr, "[");
	int i = 0;
	for (i = 0; i < ((len*c)/nCases); ++i)
		fprintf(stderr, "=");
	fprintf(stderr, ">");
	for (; i < len; ++i)
		fprintf(stderr, " ");
	
	
	int64_t delta_usec = 1000000*(thisTime.tv_sec-lastTime.tv_sec) + (thisTime.tv_usec-lastTime.tv_usec);
	total_usec += delta_usec;
	
	int64_t ds = delta_usec / 1000000;
	int64_t dus = delta_usec - 1000000*ds;
	int64_t ts = total_usec / 1000000;
	int64_t tus = total_usec - 1000000*ts;
	
	fprintf(stderr, "] Case #%li: %02li:%02li.%06li Total: %02li:%02li.%06li| ", c,
			ds/60, ds%60, dus,
			ts/60, ts%60, tus);
			
	lastTime = thisTime;
}
