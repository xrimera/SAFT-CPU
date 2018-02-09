
#include "addsig2vol_3.h"
#include "addsig2vol_3_unittests.h"
#include "as2v_array.h"
#include "timestats.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#include "munit/munit.h"
// #include "mex.h"

// For printing in colors
#include <stdarg.h>
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


//
// static MunitResult
// test_mx_conversion(const MunitParameter params[], void* user_data) {
//     (void) params;
//
//     cArray* real = ((cArray**)user_data)[0];
//     cArray* complex = ((cArray**)user_data)[1];
//
//     munit_assert_int(real->type, ==, complex->type);
//
//     mxArray* mxarray = caNewMxarrayFromCArray(real, complex);
//
//     switch(real->type){
//         case CARRAY_DOUBLE: ;
//             cArrayDouble* realD = (cArrayDouble*) real;
//             cArrayDouble* complexD = (cArrayDouble*) complex;
//             munit_assert_memory_equal(realD->len*sizeof(double), realD->data, mxGetPr(mxarray));
//             munit_assert_memory_equal(complexD->len*sizeof(double), complexD->data, mxGetPi(mxarray));
//             break;
//         case CARRAY_FLOAT: ;
//             cArrayFloat* realF = (cArrayFloat*) real;
//             cArrayFloat* complexF = (cArrayFloat*) complex;
//             munit_assert_memory_equal(realF->len*sizeof(double), realF->data, mxGetPr(mxarray));
//             munit_assert_memory_equal(complexF->len*sizeof(double), complexF->data, mxGetPi(mxarray));
//             break;
//         default: ;
//         break;
//     }
//     return MUNIT_OK;
// }



static MunitResult
test_ca_saveAndLoad(const MunitParameter params[], void* user_data) {
    (void) params;

    cArray* origin = ((cArray**)user_data)[0];
    cArray* reload = ((cArray**)user_data)[1];

    munit_assert_int(origin->type, ==, reload->type);

    caSave(origin, "test_ca_saveAndLoad");
    caLoad(reload, "test_ca_saveAndLoad");
    remove("test_ca_saveAndLoad");

    unsigned int len = fmax(origin->len, reload->len);

    switch(origin->type){
        case CARRAY_DOUBLE: ;
            cArrayDouble* downcastOriginD = (cArrayDouble*) origin;
            cArrayDouble* downcastReloadD = (cArrayDouble*) reload;
            if(downcastOriginD->data && downcastReloadD->data){
                munit_assert_memory_equal(len*sizeof(double), downcastOriginD->data, downcastReloadD->data);
                free(downcastOriginD->data);
                free(downcastReloadD->data);
            } else {
                munit_assert_null(downcastOriginD->data);
                munit_assert_null(downcastReloadD->data);
            }
            break;
        case CARRAY_FLOAT: ;
            cArrayFloat* downcastOriginF = (cArrayFloat*) origin;
            cArrayFloat* downcastReloadF = (cArrayFloat*) reload;
            if(downcastOriginF->data && downcastReloadF->data){
                munit_assert_memory_equal(len*sizeof(float), downcastOriginF->data, downcastReloadF->data);
                free(downcastOriginF->data);
                free(downcastReloadF->data);
            } else {
                munit_assert_null(downcastOriginF->data);
                munit_assert_null(downcastReloadF->data);
            }
            break;
        default: ;
            printf("\norigin datatype unknown, can't compare");
            return MUNIT_ERROR;
            break;
    }
    return MUNIT_OK;
}

void* pairToCompare(void* a, void* b){
    static void* pair[] = {NULL, NULL};
    pair[0] = a;
    pair[1] = b;
    static void** fixture = pair; // Need a pointer here, not array (compiler will generate different codes)
    return fixture;
}

static void* setup_ca_saveAndLoad_floatMargins(const MunitParameter params[], void* user_data) {
    float* data = malloc(6*sizeof(float));
    data[0] = 0.;
    data[1] = -0.;
    data[2] = FLT_MAX;
    data[3] = -FLT_MAX;
    data[4] = FLT_MIN;
    data[5] = -FLT_MIN;
    static cArrayFloat origin; origin = caNewArrayFloatFromData(data, 6, 1, 1);
    static cArrayFloat reload; reload = caNewArrayFloat();
    return pairToCompare(&origin, &reload);
}

static void* setup_ca_saveAndLoad_doubleMargins(const MunitParameter params[], void* user_data) {
    double* data = malloc(6*sizeof(double));
    data[0] =  0.;
    data[1] =  -0.;
    data[2] =  DBL_MAX;
    data[3] = -DBL_MAX;
    data[4] = DBL_MIN;
    data[5] = -DBL_MIN;
    static cArrayDouble origin; origin = caNewArrayDoubleFromData( data, 6, 1, 1);
    static cArrayDouble reload; reload = caNewArrayDouble();
    return pairToCompare(&origin, &reload);
}


static void* setup_ca_saveAndLoad_floatRandom(const MunitParameter params[], void* user_data) {
    unsigned int x=10, y=10, z=10, len=x*y*z;
    float* data = malloc(len*sizeof(float));
    //fill data with random floats
    for(int i=0; i<len; i++) data[i] = (float)(munit_rand_double() * munit_rand_int_range( -100000, 100000));
    static cArrayFloat origin; origin = caNewArrayFloatFromData( data, x, y, z);
    static cArrayFloat reload; reload = caNewArrayFloat();
    return pairToCompare(&origin, &reload);
}

static void* setup_ca_saveAndLoad_doubleRandom(const MunitParameter params[], void* user_data) {
    unsigned int x=10, y=10, z=10, len=x*y*z;
    double* data = malloc(len*sizeof(double));
    //fill data with random doubles
    for(int i=0; i<len; i++) data[i] = (double)(munit_rand_double() * munit_rand_int_range( -100000, 100000));
    static cArrayDouble origin; origin = caNewArrayDoubleFromData(data, x, y, z);
    static cArrayDouble reload; reload = caNewArrayDouble();
    return pairToCompare(&origin, &reload);
}

static void* setup_ca_saveAndLoad_emptyData(const MunitParameter params[], void* user_data) {
    unsigned int x=5, y=5, z=5;
    static cArrayDouble origin; origin = caNewArrayDoubleFromData( NULL, x, y, z);
    static cArrayDouble reload; reload = caNewArrayDouble();
    return pairToCompare(&origin, &reload);
}

static void* setup_ca_saveAndLoad_emptyDim(const MunitParameter params[], void* user_data) {
    unsigned int x=5, y=5, z=0;
    static cArrayDouble origin; origin = caNewArrayDoubleFromData( NULL, x, y, z);
    static cArrayDouble reload; reload = caNewArrayDouble();
    return pairToCompare(&origin, &reload);
}

static MunitResult
test_as2v_IO(const MunitParameter params[], void* user_data) {

    //silence compiler warnings about parameters being unused
    (void) params;
    (void) user_data;

    cArrayFloat rec_pos = caNewArrayFloatFromFile("data_ref/rec");
    cArrayFloat send_pos = caNewArrayFloatFromFile("data_ref/send");
    cArrayFloat pix_vect = caNewArrayFloatFromFile("data_ref/pix");
    cArrayFloat speed = caNewArrayFloatFromFile("data_ref/speed");
    cArrayDouble AScan_real = caNewArrayDoubleFromFile("data_ref/AScan");
    cArrayDouble out_real = caNewArrayDoubleFromFile("data_ref/out");
    cArrayDouble IMAGE_SUM = caNewArrayDoubleFromFile("data_ref/IMAGE_SUM");

    float res= 0.001;
    float timeintz=1e-7;

    //skip local benchmarking
    as2v_overwriteBenchresultToThreadcount_n(4);

    as2v_results results = as2v_addsig2vol_3(&AScan_real, NULL,
        &pix_vect, &rec_pos, &send_pos, &speed, &res, &timeintz,
        &IMAGE_SUM, NULL);

    cArrayDouble* outz = results.out_real;
    cArrayDouble* buffer = results.buffer_real;

    caSave(outz, "orig");
    caSave(&out_real, "ref");

    munit_assert_memory_equal( out_real.len*sizeof(double), outz->data, out_real.data);
    free(rec_pos.data);
    free(send_pos.data);
    free(pix_vect.data);
    free(AScan_real.data);
    free(out_real.data);
    free(IMAGE_SUM.data);
    free(speed.data);
    free(outz->data);
    free(buffer->data);
    return MUNIT_OK;
}

/* Creating a test suite is pretty simple.  First, you'll need an
 * array of tests: */
static MunitTest
test_suite_as2v_arrays[] = {
   //test name, function, setupfunction, teardownfunction, options, params
  { (char*) "/test_ca_saveAndLoad_floatMargins", test_ca_saveAndLoad, setup_ca_saveAndLoad_floatMargins, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { (char*) "/test_ca_saveAndLoad_doubleMargins", test_ca_saveAndLoad, setup_ca_saveAndLoad_doubleMargins, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { (char*) "/test_ca_saveAndLoad_doubleRandom", test_ca_saveAndLoad, setup_ca_saveAndLoad_doubleRandom, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { (char*) "/test_ca_saveAndLoad_floatRandom", test_ca_saveAndLoad, setup_ca_saveAndLoad_floatRandom, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { (char*) "/test_ca_saveAndLoad_emptyData", test_ca_saveAndLoad, setup_ca_saveAndLoad_emptyData, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { (char*) "/test_ca_saveAndLoad_emptyDim", test_ca_saveAndLoad, setup_ca_saveAndLoad_emptyDim, NULL, MUNIT_TEST_OPTION_NONE, NULL },

 // { (char*) "/test_mx_conversion", test_mx_conversion, setup_ca_saveAndLoad_floatMargins, NULL, MUNIT_TEST_OPTION_NONE, NULL },

  //end of tests: add NULL entry
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static MunitTest
test_suite_as2v_algorithm[] = {
   //test name, function, setupfunction, teardownfunction, options, params
   { (char*) "/test_as2v_IO", test_as2v_IO, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  //end of tests: add NULL entry
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

 static const MunitSuite
    all_suites[] = {
        { "as2v_arrays", test_suite_as2v_arrays, NULL, 1, MUNIT_SUITE_OPTION_NONE },
   { "as2v_algorithm", test_suite_as2v_algorithm, NULL, 1, MUNIT_SUITE_OPTION_NONE },
     { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }
 };



/* Now we'll actually declare the test suite.  You could do this in
 * the main function, or on the heap, or whatever you want. */
static const MunitSuite test_suite = {
  (char*) "µnit/",  NULL, all_suites, 1, MUNIT_SUITE_OPTION_NONE };



void print(char* string, ...){
    char buf[strlen(string)+20];
    snprintf(buf, sizeof buf, "%s%s%s", BLU, string, RESET);
    va_list args;
    va_start(args, buf);
    vprintf(buf, args);
    va_end(args);
}

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
  /* Finally, we'll actually run our test suite!  That second argument
   * is the user_data parameter which will be passed either to the
   * test or (if provided) the fixture setup function. */

   // fix seed for random data generation
   //munit_rand_seed(5);
   // set printer color so we can see output easier


   //RUN TEST
   as2v_setPrintCallback(&print);
   caSetPrintCallback(&print);
   printIntro();
   as2v_benchLocal();
   return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);

   // RUN SINGLE TASK


       //
       // unsigned int seed = time(NULL);
       // srand(seed);
       //
       // unsigned int count = 10;
       // unsigned int n_AScan = 1000;
       // unsigned int x=128;
       // float speedz[1] ={1500};
       //
       // double* array0 = malloc(x*x*x*sizeof(double));
       // double* array1 = malloc(n_AScan*count*1*sizeof(double));
       // float* array2 = malloc(3*sizeof(float));
       // float* array3 = malloc(3*count*sizeof(float));
       // float* array4 = malloc(3*count*sizeof(float));
       // float* array5 = malloc(1*sizeof(float));
       //
       // cArrayDouble AScan    = caNewArrayDoubleFromData(array1, n_AScan, count, 1);
       // cArrayFloat pix_vect  = caNewArrayFloatFromData(array2, 3, 1, 1);
       // cArrayFloat rec_posz  = caNewArrayFloatFromData(array3, 3, count, 1);
       // cArrayFloat send_posz = caNewArrayFloatFromData(array4, 3, count, 1);
       // cArrayFloat speed     = caNewArrayFloatFromData(array5, 1, 1, 1);
       //
       // cArrayDouble IMAGE_SUM = caNewArrayDoubleFromData(array0, x, x, x);
       //
       // float timeintz[1] ={1e-7};
       // float resz[1] ={0.001};
       //
       // for(int i =0; i<n_AScan;i++ ){
       //     AScan.data[i] = (int)(rand()%2);
       // }
       // for(int j=1; j < count; j++){
       //     for(int i =0; i<n_AScan;i++){
       //         AScan.data[i+n_AScan*j] = AScan.data[i];
       //     }
       // }
       // for(int i =0; i<count*3;i++)
       // {   rec_posz.data[i] = 0.01*rand()/RAND_MAX;
       //     send_posz.data[i] = 0.01*rand()/RAND_MAX;
       // }
       //
       // tsclear(3);
       // tsclear(11);
       // for (int i =0; i< 10; i++){
       // tsclock(49);
       // as2v_results results = as2v_addsig2vol_3(&AScan, NULL,
       //     &pix_vect, &rec_posz, &send_posz, &speed, resz, timeintz,
       //     &IMAGE_SUM, NULL);
       //     tsclock(49);
       // }
       // tsprint(49, TS_MIKRO);
       // tsprint(3, TS_MIKRO);
       // tsprint(11, TS_MIKRO);
}


void runTests(){
    as2v_setPrintCallback(&print);
    caSetPrintCallback(&print);
    printIntro();

    return munit_suite_main(&test_suite, (void*) "µnit", 0, NULL);
}




// int main(){
//
//     unsigned int seed = time(NULL);
//     srand(seed);
//
//     unsigned int count = 10;
//     unsigned int n_AScan = 1000;
//     unsigned int x=100;
//     float speedz[1] ={1500};
//
//
//     upstreamDoubleArray(NULL);
//      setUpstreamDoubleArray_callback(NULL);
//      setUpstreamFloatArray_callback(NULL);
//
//     as2v_doubleArray AScan    = as2v_mallocDoubleArray(n_AScan, count, 1);
//     as2v_floatArray pix_vect  = as2v_mallocFloatArray(3, 1, 1);
//     as2v_floatArray rec_posz  = as2v_mallocFloatArray(3, count, 1);
//     as2v_floatArray send_posz = as2v_mallocFloatArray(3, count, 1);
//     as2v_floatArray speed     = as2v_boxFloatArray(speedz, 1, 1, 1);
//
//     as2v_doubleArray IMAGE_SUM = as2v_mallocDoubleArray(x, x, x);
//
//     float timeintz[1] ={1e-7};
//     float resz[1] ={0.001};
//
//     for(int i =0; i<n_AScan;i++ ){
//         AScan.data[i] = (int)(rand()%2);
//     }
//     for(int j=1; j < count; j++){
//         for(int i =0; i<n_AScan;i++){
//             AScan.data[i+n_AScan*j] = AScan.data[i];
//         }
//     }
//     for(int i =0; i<count*3;i++)
//     {   rec_posz.data[i] = 0.01*rand()/RAND_MAX;
//         send_posz.data[i] = 0.01*rand()/RAND_MAX;
//     }
//
//     saveFloatArray(&rec_posz, "rec");
//     as2v_doubleArray newrec_posz = as2v_boxDoubleArray(NULL, 0,0,0);
//     loadFloatArray(&newrec_posz, "rec");
//     saveFloatArray(&newrec_posz, "newrec");
//
//
//     munit_assert_memory_equal((size_t)rec_posz.len, rec_posz.data, newrec_posz.data);
//
//     as2v_results results = as2v_addsig2vol_3(&AScan, NULL,
//         &pix_vect, &rec_posz, &send_posz, &speed, resz, timeintz,
//         &IMAGE_SUM, NULL);
//
//     as2v_doubleArray outz = results.out_real;
//
//
//
//     PRINT("data size: %i\n”", outz.len);
//     PRINT("x  y  z: %i  %i  %i\n", outz.x, outz.y, outz.z);
//
//     free(AScan.data);
//     free(pix_vect.data);
//     free(rec_posz.data);
//     free(send_posz.data);
//     free(IMAGE_SUM.data);
//
//     return 0;
// }
