#include "threadstats.h"

pthread_mutex_t threadsafeMutex;

unsigned int nThreads = 0;
unsigned int nTasks = 0;
unsigned int nReruns = 0;

double* dataTimestamps = NULL;
double* dataMoveToTask = NULL;
double* dataThreadnumber = NULL;

cArrayDouble arrayTimestamps;
cArrayDouble arrayMoveToTask;
cArrayDouble arrayThreadnumber;

void threadstats_init(unsigned int _nThreads, unsigned int _nTasks, unsigned int _nReruns){
    nThreads = _nThreads;
    nTasks = _nTasks;
    nReruns = _nReruns;
    free(dataTimestamps);
    free(dataMoveToTask);
    free(dataThreadnumber);
    dataTimestamps = malloc(nTasks*nReruns*sizeof(double));
    dataMoveToTask = malloc(nTasks*nReruns*sizeof(double));
    dataThreadnumber = malloc(nTasks*nReruns*sizeof(double));
    arrayTimestamps = caNewArrayDoubleFromData(dataTimestamps, nTasks, nReruns, 1);
    arrayMoveToTask = caNewArrayDoubleFromData(dataMoveToTask, nTasks, nReruns, 1);
    arrayThreadnumber = caNewArrayDoubleFromData(dataThreadnumber, nTasks, nReruns, 1);
    printf("nThreads %i, nTasks %i, nReruns %i\n", nThreads, nTasks, nReruns);
}

void threadstats_mark(unsigned int thread, unsigned int currentTask, unsigned int run){
    pthread_mutex_lock(&threadsafeMutex);
    static unsigned int counter = 0;
    static unsigned int pointer;
    pointer = run*nTasks+counter;

    *(dataTimestamps+pointer) = getSumTime(thread, TS_MIKRO);
    *(dataMoveToTask+pointer) = (double) currentTask;
    *(dataThreadnumber+pointer) = (double) thread;
    printf("counter %i, timestamp %f, currentTask %f, thread %f\n", counter, *(dataTimestamps+pointer), *(dataMoveToTask+pointer), *(dataThreadnumber+pointer));
    counter++;
    if(counter == nTasks) counter=0;
    pthread_mutex_unlock(&threadsafeMutex);
}

cArrayDouble* threadstats_get_timestamps(){
    return &arrayTimestamps;
}

cArrayDouble* threadstats_get_moveToTask(){
    return &arrayMoveToTask;
}

cArrayDouble* threadstats_get_threadnumber(){
    return &arrayThreadnumber;
}
