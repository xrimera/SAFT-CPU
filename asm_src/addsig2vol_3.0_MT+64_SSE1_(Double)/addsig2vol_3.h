#ifndef H_ADDSIG2VOL_3
#define H_ADDSIG2VOL_3

#ifndef __WIN32__
    #ifdef WIN32
    #define __WIN32__
    #endif
    #ifdef _WIN32
    #define __WIN32__
    #endif
    #ifdef __WIN32
    #define __WIN32__
    #endif
    #ifdef _WIN64
    #define __WIN32__
    #endif
    #ifdef WIN64
    #define __WIN32__
    #endif
    #ifdef _WINDOWS
    #define __WIN32__
    #endif
#endif

#ifndef __linux__
    #ifdef linux
    #define __linux__
    #endif
    #ifdef __linux
    #define __linux__
    #endif
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "as2v_array.h"

typedef struct
{
    cArrayDouble* out_real;
    cArrayDouble* out_complex;
    cArrayDouble* buffer_real;
    cArrayDouble* buffer_complex;
} as2v_results;

//run algorithm
as2v_results as2v_addsig2vol_3(cArrayDouble* AScan_realz, cArrayDouble* AScan_complexz,
            cArrayFloat* pix_vectz, cArrayFloat* rec_posz, cArrayFloat* send_posz, cArrayFloat* speedz, float* resz, float* timeintz,
		    cArrayDouble* IMAGE_SUM_realz, cArrayDouble* IMAGE_SUM_complexz, cArrayDouble* outputImage, cArrayDouble* buffer);

//Some general information, also about the build version
void printIntro();

// store benchmark results static to addsig2vol_3.c
void as2v_benchLocal();

// store benchmark results whereever
void as2v_bench(uint64_t throughput[], uint64_t latency[]);

void as2v_overwriteBenchresultToThreadcount_n(uint32_t n);

void as2v_setPrintCallback(void (*callback)(char*, ...));
void as2v_setUpstreamCallback(void (*callback)(cArray*));
//setUpstreamFloatArray_callback(upstreamCopiedFloatArray);

size_t getCache3Size();


#endif //H_ADDSIG2VOL_3
