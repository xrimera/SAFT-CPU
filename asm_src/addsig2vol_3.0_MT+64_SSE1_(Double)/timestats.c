#include "timestats.h"

// TODO: Make switch for whole measurements
// TODO: MAKE switch for difference measurements


typedef struct{
    clock_t sum;
    clock_t diffsum;
    clock_t last;
    unsigned long num;
    char ticking;
    clock_t start;
} timer;


static timer timers[TS_TIMERS_N] = { 0 };


//NOTE: clock() measures time the CPU is executing instruction belonging to this binary. Sleep/Wait doesn't perform any work;
clock_t tsclock(unsigned int number){
    if(timers[number].ticking){
        clock_t measured = clock() - timers[number].start;
        timers[number].diffsum += abs(measured - timers[number].last);
        timers[number].last = measured;
        timers[number].sum += measured;
        timers[number].num ++;
    }
    else{
        timers[number].start = clock();
    }
    timers[number].ticking = !timers[number].ticking;
}


void tsprintAll(unsigned int scale){
    for(int i=0; i<TS_TIMERS_N; i++){
        printf("timer %i avg [s]: %f, diff avg [s]: %f, n: %i \n", i, \
        ((double) timers[i].sum) / CLOCKS_PER_SEC / timers[i].num * scale, \
        ((double) timers[i].diffsum) / CLOCKS_PER_SEC / timers[i].num * scale, timers[i].num);
    }
}

void tsprint(unsigned int number, unsigned int scale){
        printf("timer %i avg: %f, diff avg: %f, n: %i \n", number, \
        ((double) timers[number].sum) / CLOCKS_PER_SEC / timers[number].num * scale, \
        ((double) timers[number].diffsum) / CLOCKS_PER_SEC / timers[number].num * scale, timers[number].num );
}

void tsclear(unsigned int number){
    timers[number].sum = 0;
    timers[number].diffsum = 0;
    timers[number].last = 0;
    timers[number].num = 0;
    timers[number].ticking = 0;
    timers[number].start = 0;
}

void tsclearAll(){
    char* memory = &timers;
    for(int i=0; i<TS_TIMERS_N*sizeof(timer); i++){
        *memory = 0;
        memory++;
    }
}
