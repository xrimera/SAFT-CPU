#ifndef TIMESTATS_H
#define TIMESTATS_H

#include "time.h"

#define TS_TIMERS_N (32)
#define TS_SEC   (1)
#define TS_MILI  (1000)
#define TS_MIKRO (1000000)
#define TS_NANO  (1000000000)

//NOTE: clock() measures time the CPU is executing instruction belonging to this binary. Sleep/Wait doesn't perform any work;
clock_t tsclock(unsigned int number);

void tsprintAll(unsigned int scale);
void tsprint(unsigned int number, unsigned int scale);

void tsclear(unsigned int number);
void tsclearAll();

#endif // TIMESTATS_H
