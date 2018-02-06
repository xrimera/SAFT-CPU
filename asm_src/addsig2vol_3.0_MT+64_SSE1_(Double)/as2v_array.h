#ifndef H_AS2V_ARRAY
#define H_AS2V_ARRAY


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
#include <float.h>

#ifdef MATLAB_MEX_FILE
    #define BUILD_WITH_MEX
#endif

#ifdef BUILD_WITH_MEX
    #include "mex.h"
#endif

// Define all significant digits for double precision
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

// Define all significant digits for float precision
#ifdef FLT_DECIMAL_DIG
  #define OP_FLT_Digs (FLT_DECIMAL_DIG)
#else
  #ifdef DECIMAL_DIG
    #define OP_FLT_Digs (DECIMAL_DIG)
  #else
    #define OP_FLT_Digs (FLT_DIG + 3)
  #endif
#endif

// Define a custom array (Base class won't do anything)
typedef struct {
    unsigned int type, x, xpadded, y, z, len;
    void* data;
} cArray;

typedef struct {
    unsigned int type, x, xpadded, y, z, len;
    double* data;
} cArrayDouble;

typedef struct {
    unsigned int type, x, xpadded, y, z, len;
    float* data;
} cArrayFloat;

// Supported arrays
static enum CARRAY_TYPE {CARRAY_FLOAT, CARRAY_DOUBLE};

// Get empty new array
cArrayDouble caNewArrayDouble();
cArrayFloat caNewArrayFloat();

// Fill in custom array object with given data
cArrayDouble caNewArrayDoubleFromData(double* data, unsigned int x, unsigned int y, unsigned int z);
cArrayFloat caNewArrayFloatFromData(float* data, unsigned int x, unsigned int y, unsigned int z);

cArrayDouble caNewArrayDoubleFromFile(char* filename);
cArrayFloat caNewArrayFloatFromFile(char* filename);

// Change to custom print function instead of default (printf)
void caSetPrintCallback(void (*callback)(char*, ...));
// Save cArray into file
void caSave(cArray* array, char* filename);
// Print cArray without its full data
void caMiniprint(cArray* array);
// Load cArray from file
void caLoad(cArray* array, char* filename);
void caUpdateData(cArray* array, void* data, unsigned int x, unsigned int y, unsigned int z);


char* concatPath(char* filename);


#ifdef BUILD_WITH_MEX
// NOTE: When mxArray is free'd, cArray will contain dangling pointers
cArrayDouble caNewDoubleArrayFromMxarray(mxArray* mxa, mxComplexity flag);
cArrayFloat caNewFloatArrayFromMxarray(mxArray* mxa, mxComplexity flag);
// memcopy into mxarray
mxArray* caNewMxarrayFromCArray(cArray* real, cArray* complex);
#endif //BUILD_WITH_MEX


#endif //H_AS2V_ARRAY
