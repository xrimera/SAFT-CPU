#ifndef THREADSTATS_H
#define THREADSTATS_H

#define THREADSTATS_ON

//------------------------------
//Switches
//#undef  TS_ON              //Exclude code from compiling
//------------------------------

#include <pthread.h>
#include "as2v_array.h"
#include "timestats.h"

extern cArrayDouble threadstats_arrayTimestamps;
extern cArrayDouble threadstats_arrayMoveToTask;
extern cArrayDouble threadstats_arrayThreadnumber;

void threadstats_init(unsigned int _nThreads, unsigned int _nTasks, unsigned int _nReruns);
void threadstats_mark(unsigned int thread, unsigned int currentTask, unsigned int run);

cArrayDouble* threadstats_get_timestamps();
cArrayDouble* threadstats_get_moveToTask();
cArrayDouble* threadstats_get_threadnumber();


#endif // THREADSTATS_H
