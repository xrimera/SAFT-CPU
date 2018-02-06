#include "addsig2vol_3.h"
#include "as2v_array.h"
#include "mex.h"
#include "addsig2vol_3_unittests.h"

// use mexPrints to flush text from addsig2vol_3.c into MATLAB
#define PRINT(string, ...) mexPrintf(string, ##__VA_ARGS__)

//define matlab in and out
#define out       (plhs[0])
#define out2      (plhs[1])

#define AScan     (prhs[0])
#define pix_vect  (prhs[1])
#define rec_pos   (prhs[2])
#define send_pos  (prhs[3])
#define speed     (prhs[4])
#define res       (prhs[5])
#define timeint   (prhs[6])
#define IMAGE_XYZ (prhs[7])
#define IMAGE_SUM (prhs[8])

void upstreamCopiedDoubleArray(cArray* array)
{
    if(array){
    //data gets copied, so freeing mxArray done by the MATLAB memory manager dosn't destroy C data
    mxArray* mxa = caNewMxarrayFromCArray(array, NULL);
    // NOTE: this does a segfault work in octave
    //mexCallMATLAB(0, NULL, 1, mxa, "disp");
    }
}

void upstreamCopiedFloatArray(cArray* array)
{
    if(array){
    //data gets copied, so freeing mxArray done by the MATLAB memory manager dosn't destroy C data
    mxArray* mxa = caNewMxarrayFromCArray(array, NULL);
    // NOTE: this does a segfault work in octave
    //mexCallMATLAB(0, NULL, 1, mxa, "disp");
    }
}

//Jumping point for mex
void mexFunction (int nlhs, mxArray*plhs[], int nrhs, const mxArray*prhs[]) {
    //use mexCallMATLAB for debugging data
    //setUpstreamDoubleArray_callback(upstreamCopiedDoubleArray);
    //setUpstreamFloatArray_callback(upstreamCopiedFloatArray);

    if (nlhs > 2) mexErrMsgTxt("Too many output arguments.");

    switch (nrhs) {
        default:
            mexErrMsgTxt("Incorrect number of arguments.\n");
            break;

        case 0:
            // Say hello
            printIntro();
            // Give plhs and prhs definitions
            mexPrintf("\n\n#define out       plhs[0] (Double(1:end))\n#define out2      plhs[1] DEBUG (Double(1:end))\n#define AScan     prhs[0] (Double(NxM))\n#define pix_vect  prhs[1] (Single(1:3))\n#define rec_pos   prhs[2] (Single(1:3xM) or Single(1:3x1))\n#define send_pos  prhs[3] (Single(1:3xM) or Single(1:3x1))\n#define speed     prhs[4] (Single (1x1 or 1xM))\n#define res       prhs[5] (Single)\n#define timeint   prhs[6] (Single)\n#define IMAGE_XYZ prhs[7] (UINT32(1:3))\n#define IMAGE_SUM prhs[8] (Double(1:end))\n");
            // Run benchmarks
            as2v_benchLocal();
            break;

        case 1:
            // Force code to run on prhs[0] threads
            as2v_overwriteBenchresultToThreadcount_n((uint32_t) ceil(*((double*)mxGetPr(prhs[0]))));

            runTests();
            break;

        case 9:
            // Pre C99: No declarations next after label
            ;

            // Box mxarrays into input arrays (data linked, not copied) -> Freeing will be done by MATLAB memory management
            cArrayDouble AScan_realz = caNewDoubleArrayFromMxarray(&AScan, mxREAL);
            cArrayDouble AScan_complexz = caNewDoubleArrayFromMxarray(&AScan, mxCOMPLEX);
            cArrayDouble IMAGE_SUM_realz = caNewDoubleArrayFromMxarray(&IMAGE_SUM, mxREAL);
            cArrayDouble IMAGE_SUM_complexz = caNewDoubleArrayFromMxarray(&IMAGE_SUM, mxCOMPLEX);
            cArrayFloat pix_vectz = caNewFloatArrayFromMxarray(&pix_vect,  mxREAL);
            cArrayFloat rec_posz = caNewFloatArrayFromMxarray(&rec_pos, mxREAL);
            cArrayFloat send_posz = caNewFloatArrayFromMxarray(&send_pos,  mxREAL);
            cArrayFloat speedz = caNewFloatArrayFromMxarray(&speed, mxREAL);

            // Run algorithm
            as2v_results result = as2v_addsig2vol_3(&AScan_realz, &AScan_complexz,
            &pix_vectz, &rec_posz, &send_posz, &speedz, mxGetPr(res), mxGetPr(timeint),
            &IMAGE_SUM_realz, &IMAGE_SUM_complexz);

            // Copy output arrays into mxarrays
            // These will be free'd by MATLAB memory management
            out = caNewMxarrayFromCArray((result.out_real), (result.out_complex));
            out2 = caNewMxarrayFromCArray((result.buffer_real), (result.buffer_complex));

            // combined REAL & COMPLEX VERSION
            //no sizeof(double) needed because compilers assumes already double as datatype for pointer!!!
            //matlab seems to do some nasty errors on ptr to float...adding 8bytes instead of 4!!!! workaround implemented

            // Results can be destroyed
            free(result.out_real->data);
            //free(result.out_complex.data);
            // free(result.buffer_real.data);
            // free(result.buffer_complex.data);
            break;
        return;
    }
}
