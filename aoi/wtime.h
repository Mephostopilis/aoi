#pragma once

#if WIN32
#include <winsock.h>
#include <stdint.h>

struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

int gettimeofday(struct timeval *val, struct timezone *);

#endif // WIN32

uint64_t getcurs();     // s
uint64_t getcurti();    // 1/100s
uint64_t getcurms();    // 1/1000s
uint64_t getcurus();    // 1/1000000s

