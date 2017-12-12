#ifndef H_AS2V_ARRAY
#define H_AS2V_ARRAY

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <float.h>

#ifdef MATLAB_MEX_FILE
    #define BUILD_WITH_MEX
#endif

#ifdef BUILD_WITH_MEX
    #include "mex.h"
#endif

#ifndef PRINT(string, ...)
    #define PRINT(string, ...) printf(string, ##__VA_ARGS__)
#endif


// Get all significant digits for double precision
// See: https://stackoverflow.com/questions/16839658/printf-width-specifier-to-maintain-precision-of-floating-point-value
#ifdef DBL_DECIMAL_DIG
  #define OP_DBL_Digs (DBL_DECIMAL_DIG)
#else
  #ifdef DECIMAL_DIG
    #define OP_DBL_Digs (DECIMAL_DIG)
  #else
    #define OP_DBL_Digs (DBL_DIG + 3)
  #endif
#endif

// Get all significant digits for float precision
#ifdef FLT_DECIMAL_DIG
  #define OP_FLT_Digs (FLT_DECIMAL_DIG)
#else
  #ifdef DECIMAL_DIG
    #define OP_FLT_Digs (DECIMAL_DIG)
  #else
    #define OP_FLT_Digs (FLT_DIG + 3)
  #endif
#endif


typedef struct {
    double* data;
    unsigned long x, y, z, len;
} as2v_doubleArray;

typedef struct {
    float* data;
    unsigned long x, y, z, len;
} as2v_floatArray;


static void (*upstreamDoubleArray_callback)(as2v_doubleArray*) = NULL;
static void (*upstreamFloatArray_callback)(as2v_floatArray*) = NULL;



void upstreamDoubleArray(as2v_doubleArray* array);
void upstreamFloatArray(as2v_floatArray* array);

as2v_doubleArray as2v_boxDoubleArray(double* data, unsigned int x, unsigned int y, unsigned int z);
as2v_floatArray as2v_boxFloatArray(float* data, unsigned int x, unsigned int y, unsigned int z);

void saveDoubleArray(as2v_doubleArray* array, char* filename);
void saveFloatArray(as2v_floatArray* array, char* filename);

void loadDoubleArray(as2v_doubleArray* targetArray, char* filename);
void loadFloatArray(as2v_floatArray* targetArray, char* filename);


void briefDoubleArray(as2v_doubleArray* array);
void briefFloatArray(as2v_doubleArray* array);

void setUpstreamDoubleArray_callback(void (*callback)(as2v_doubleArray*));
void setUpstreamFloatArray_callback(void (*callback)(as2v_floatArray*));


as2v_doubleArray as2v_mallocDoubleArray(unsigned int x, unsigned int y, unsigned int z);
as2v_floatArray as2v_mallocFloatArray(unsigned int x, unsigned int y, unsigned int z);

#ifdef BUILD_WITH_MEX
    as2v_doubleArray as2v_mxArray2doubleArray(mxArray* mxa, mxComplexity flag);
    as2v_floatArray as2v_mxArray2floatArray(mxArray* mxa, mxComplexity flag);

    mxArray* as2v_doubleArray2mxArray(as2v_doubleArray* array_real, as2v_doubleArray* array_complex);
    mxArray* as2v_floatArray2mxArray(as2v_floatArray* array_real, as2v_floatArray* array_complex);
#endif //BUILD_WITH_MEX


#endif //H_AS2V_ARRAY
