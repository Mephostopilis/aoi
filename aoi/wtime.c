#include "wtime.h"

#if WIN32

#include <winnt.h>
#include <profileapi.h>

int gettimeofday(struct timeval *val, struct timezone *tz) {

	if (val) {
		LARGE_INTEGER liTime, liFreq;
		QueryPerformanceFrequency(&liFreq);
		QueryPerformanceCounter(&liTime);
		val->tv_sec = (long)(liTime.QuadPart / liFreq.QuadPart);
		val->tv_usec = (long)(liTime.QuadPart * 1000000.0 / liFreq.QuadPart - val->tv_sec * 1000000.0);
	}
	return 0;
}


#endif // WIN32

uint64_t getcurs() {
	uint64_t ret;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	ret = tv.tv_sec;
	return ret;
}

uint64_t getcurti() {
	uint64_t ret;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	ret = tv.tv_sec * 100 + tv.tv_usec / 10000;
	return ret;
}

uint64_t getcurms() {
	uint64_t ret;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	ret = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return ret;
}

uint64_t getcurus() {
	uint64_t ret;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	ret = tv.tv_sec * 1000000 + tv.tv_usec;
	return ret;
}