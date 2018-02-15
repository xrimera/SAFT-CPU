#ifndef TIMESTATS_H
#define TIMESTATS_H

#define TS_ON
#define TS_THREADSAFE

//------------------------------
//Switches
//#undef  TS_ON              //Exclude code from compiling
//#undef  TS_THREADSAFE      //For threadsafe writing on/off
//------------------------------

#include <sys/time.h>
#include <stdlib.h>
#ifdef TS_THREADSAFE
    #include <pthread.h>
#endif

#define TS_TIMERS_N (50)
#define TS_SEC   (.000000001)
#define TS_MILI  (.000001)
#define TS_MIKRO (.001)
#define TS_NANO  (.1)

#ifdef TS_THREADSAFE
    pthread_mutex_t threadsafeMutex;
#endif

void tsclock(unsigned int number);

void tsprintAll(double timescale);
void tsprint(unsigned int number, double timescale);

void tsclear(unsigned int number);
void tsclearAll();

double getAverageTime(unsigned int number, double timescale);
double getSumTime(unsigned int number, double timescale);

#endif // TIMESTATS_H
