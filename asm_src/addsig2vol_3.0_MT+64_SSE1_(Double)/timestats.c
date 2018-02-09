#include "timestats.h"

// TODO: Make switch for whole measurements
// TODO: MAKE switch for difference measurements
// TODO: switch between linux und windows


typedef struct{
    long sum;
    long diffsum;
    long last;
    unsigned int num;
    char ticking;
    struct timeval tic;
    struct timeval toc;
    struct timeval result;
} timer;





static timer timers[TS_TIMERS_N] = { 0 };


//NOTE: clock() measures time the CPU is executing instruction belonging to this binary. Sleep/Wait doesn't perform any work;
// time() measures wallclock time
void tsclock(unsigned int number){
    #ifdef TS_ON
    timer* instance = &(timers[number]);
    if(instance->ticking){
        gettimeofday(&(instance->toc), NULL);
        //timers[number].diffsum += abs(measured - timers[number].last);
        //timers[number].last = measured;
        timersub(&(instance->toc), &(instance->tic), &(instance->result));
        instance->sum += instance->result.tv_sec*1000000 + instance->result.tv_usec;
        instance->num ++;
    }
    else{
        gettimeofday(&(instance->tic), NULL);
    }
    instance->ticking = !(instance->ticking);
    #endif
}


void tsprintAll(double timescale){
    #ifdef TS_ON
    for(int i=0; i<TS_TIMERS_N; i++){
        printf("timer %i avg [s]: %f, diff avg [s]: %f, n: %i \n", i, \
        ((double) timers[i].sum) / timers[i].num * timescale, \
        ((double) timers[i].diffsum) / timers[i].num * timescale, timers[i].num);
    }
    #endif
}

void tsprint(unsigned int number, double timescale){
    #ifdef TS_ON
        printf("timer %i avg: %f, diff avg: %f, n: %i, sum: %f \n", \
        number, \
        ((double) timers[number].sum) / timers[number].num * timescale, \
        ((double) timers[number].diffsum) / timers[number].num * timescale, \
        timers[number].num, \
        ((double) timers[number].sum) * timescale );
    #endif
}

void tsclear(unsigned int number){
    #ifdef TS_ON
    timers[number].sum = 0;
    timers[number].diffsum = 0;
    timers[number].last = 0;
    timers[number].num = 0;
    timers[number].ticking = 0;
    #endif
}

void tsclearAll(){
    #ifdef TS_ON
    char* memory = &timers;
    for(int i=0; i<TS_TIMERS_N*sizeof(timer); i++){
        *memory = 0;
        memory++;
    }
    #endif
}
