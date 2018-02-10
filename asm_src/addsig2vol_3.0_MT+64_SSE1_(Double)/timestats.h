#ifndef TIMESTATS_H
#define TIMESTATS_H

#define TS_ON

//------------------------------
//Switches
//#undef  TS_ON //schalte Timing aus
//------------------------------

#include <sys/time.h>
#include <stdlib.h>

#define TS_TIMERS_N (50)
#define TS_SEC   (.000000001)
#define TS_MILI  (.000001)
#define TS_MIKRO (.001)
#define TS_NANO  (.1)

//NOTE: clock() measures time the CPU is executing instruction belonging to this binary. Sleep/Wait doesn't perform any work;
void tsclock(unsigned int number);

void tsprintAll(double timescale);
void tsprint(unsigned int number, double timescale);

void tsclear(unsigned int number);
void tsclearAll();

#endif // TIMESTATS_H
