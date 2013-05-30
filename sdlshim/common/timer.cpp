
#include <unistd.h>
#include <sys/time.h>

#include "timer.h"

//static struct timeval start_tv;
//bool time_inited = false;

// return the current tick count in milliseconds
tstamp timer(void)
{
struct timeval tv;
uint64_t result;

	// this is done in-line instead of using a separate initilization function
	// because statically-allocated C++ classes may have constructors which
	// could call timer() before the initilization function gets a chance to run.
	/*if (!time_inited)
	{
		time_inited = true;
		gettimeofday(&start_tv, NULL);
		
		// by subtracting one second we make the time the program started = 1000ms;
		// this ensures that timer() couldn't ever return 0, which is used in many places
		// to mean an invalid timestamp.
		start_tv.tv_sec--;
	}*/
	
	gettimeofday(&tv, NULL);
	
	// subtract the epoch time from when the program started,
	// so that 32-bit values wrap only after 49.7 days of actual program uptime,
	// instead of arbitrary points in time spaced 49 days apart.
	//tv.tv_sec -= start_tv.tv_sec;
	//tv.tv_usec -= start_tv.tv_usec;
	
	result = tv.tv_sec;
	result *= 1000;
	result += (tv.tv_usec / 1000);
	return (tstamp)result;
}
