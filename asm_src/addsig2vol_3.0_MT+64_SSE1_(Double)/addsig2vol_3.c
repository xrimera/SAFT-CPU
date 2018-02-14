#include "addsig2vol_3.h"

#ifdef __WIN32__
    #include <windows.h>
    #include <float.h>
    #include <stdlib.h>
#else
    #define _POSIX_C_SOURCE 199309L
    #define _POSIX_TIMERS 1
#endif

#ifdef __linux__
    #include <time.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <stdarg.h>
#endif

//#include "mex.h"
#include <math.h>

#ifndef _PORTABLEFPU_H_INCLUDED
#include "portableFPU.h"
#endif

#ifndef _PSTDINT_H_INCLUDED
#include "pstdint_new.h"
#endif

#include "timestats.h"



//extern __declspec(dllimport) int _imp_pthread_join();
//extern __declspec(dllimport) int _imp_pthread_create();
/*#ifdef __cplusplus
extern "C" {
#endif
 __declspec(dllimport) int __imp_pthread_join();

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
__declspec(dllimport) __imp_pthread_create();

#ifdef __cplusplus
}
#endif*/


//enable/disable debugging output
#define addsig2vol_debug
#undef addsig2vol_debug

// enable/disable pthreads
#define p_threads
//#undef p_threads

// enable/disable creating dataset
#define SAVEDATA
#undef SAVEDATA

#define addsig2vol_3_MUTED
#undef addsig2vol_3_MUTED

//enable/disable C-CODE version (disabled is Asm-code)
//#define C_CODE
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//#undef C_CODE

#ifdef p_threads
#include "pthread.h"
#define NUMCORES 16 //32 //potential maximum value for init structs etc.
#else
#define NUMCORES 1
#endif

//define interpol-ratio of AScan (for asm fix)
#define INTERP_RATIO 5    // resizes ascan from 3000x1 -> 15000x1 (FIXED SIZE lin. interp.)

//define min-Voxel Number for X (parallel voxels in X_pipe; for 64bit code = 4voxel, for 32 bit = 2voxel)
#define MIN_VOXEL 4

//global variable
unsigned int addsig2vol_mode=0; //mode = single ascan, constant speed, 1=blocked, constant speed, 2= unblocked with soundmap

//for MT handling
static int64_t latency[NUMCORES]={0}; //uint64_t nSec
static int64_t throughput[NUMCORES]={0}; //uint64 MVoxel/s
static uint32_t nCores_bench = -1;
static uint32_t nCores = NUMCORES; //used value might be reduced by imagesize, benchmark etc

static double* out_real = NULL;
static double* out_complex = NULL;
static double* buffer_real = NULL;
static double* buffer_complex = NULL;

static cArrayDouble cAout_real;
static cArrayDouble cAout_complex;
static cArrayDouble cAbuffer_real;
static cArrayDouble cAbuffer_complex;

#ifdef SAVEDATA
    static unsigned int count = 0;
#endif


//TEST-CASE 0
//tic; x=100; image_1=addsig2vol_2(rand([3000 1]),single(ones(3,1)),single(ones(3,1)),single(ones(3,1)),single(ones(1,1)),single(ones(1,1)),single(ones(3,1)),uint32([x,x,x]),zeros([x,x,x]),2); toc, j=reshape(image_1,[x x x]); imagesc(j(:,:,1));
//inter_p test
//ascan=[repmat(1000,[1 100]) repmat(0,[1 2900])]';
//tic; x=100; image_1=addsig2vol_2(ascan,single(ones(3,1)),single(ones(3,1)),single(ones(3,1)),single(ones(1,1)),single(ones(1,1)),single(ones(3,1)),uint32([x,x,x]),zeros([x,x,x]),2); toc, j=reshape(image_1,[x x x]); imagesc(j(:,:,1));

//TEST-CASE 1
//x=128; addsig2vol_3(4), image_1=zeros([x,x,x]); rand('seed',0); x=128; for i=1:2 ascan=rand([1 3000])'; [image_1,kkk]=addsig2vol_3(ascan,rand([3 1],'single'),10.*rand([3 1],'single'),400.*rand([3 1],'single'),rand([1 1],'single'),rand([1 1],'single'),rand([1 1],'single'),uint32([x,x,x]),image_1); end, j=reshape(image_1,[x x x]); imagesc(j(:,:,1));
//j=reshape(image_1-image_2,[x x x]); figure; imagesc(j(:,:,128));

//// TEST--CASE Blocked
//count=2; senderPos = 0.01.*rand(3,count); receiverPos = 0.01.*rand(3,count); IMAGE_STARTPOINT = [0,0,0]; IMAGE_RESOLUTION= 0.001; Speed=1500; TimeInterval=1e-7; DataLength=3000; Data=zeros(3000,count); Data(floor(DataLength.*rand(count,1)),1:count)=1;
//x=100; bild=addsig2vol_3(Data,single(IMAGE_STARTPOINT),single(receiverPos),single(senderPos),single(Speed),single(IMAGE_RESOLUTION),single(TimeInterval),uint32([x,x,x]),zeros([x,x,x]));

// 4x times same parameter to be have compatible win64&linux64 calling convention
 typedef   struct   /*struct reordered to circumvent alignment problems, 64pointer then 32bit values*/
        {
        double*outz;
        double*AScanz;
        double*out_complexz;
        double*bufferz;
        float*pix_vectz;
		double*buffer_complexz;
        float*rec_posz;
        float*send_posz;
        float*speedz;
        float*resz;
        float*timeintz;
        double*AScan_complexz;
		    double *IMAGE_SUMz;
        double *IMAGE_SUM_complexz;
        unsigned int n_Yz;
        unsigned int n_Zz;
        unsigned int n_AScanz;
        unsigned int n_Xz;
        unsigned int qwb0;
        unsigned int qwb1;
        unsigned int qwb2;
        unsigned int qwb3;
		    } Addsig2vol_param;

  /* typedef   struct   //struct reorded to circumvent alignment problems, 64pointer then 32bit values
        {
        double*outz;
        double*out_complexz;
        double*AScanz;
        double*AScan_complexz;
        double*bufferz;
        double*buffer_complexz;
        double *IMAGE_SUMz;
        double *IMAGE_SUM_complexz;
        float*pix_vectz;
        float*rec_posz;
        float*send_posz;
        float*speedz;
        float*resz;
        float*timeintz;
        unsigned int n_AScanz;
		    unsigned int n_Xz;
        unsigned int n_Yz;
        unsigned int n_Zz;
        } Addsig2vol_param;*/

        typedef   struct   /*struct reordered to circumvent alignment problems, 64pointer then 32bit values*/
               {
               unsigned int x;
               unsigned int y;
               unsigned int z;
               unsigned int currentJob;
               unsigned int currentTotalJob;
           } coordinate;

           enum axis { XAXIS, YAXIS, ZAXIS };

        static unsigned int L3CACHE_SIZE = 3072000;
        static unsigned int L3CACHE_LINESIZE = 64;
        static unsigned int jobs = 0;
        static unsigned int nextJobWaiting = 0;
        static coordinate threadInfo[NUMCORES];
        pthread_mutex_t threadMutex;
        static float globalPixPointer[3];
        static int segmentedAxis = ZAXIS;

        //char *myArray[100] = { "str1", "str2", ... "str100" };

        // For interlaced threads
        static unsigned int  stepZ = 0;
        static unsigned int  stepY = 0;
        static unsigned int  stepX = 0;
        static unsigned int  posZ = 0;
        static unsigned int  posY = 0;
        static unsigned int  posX = 0;

        static unsigned int imgX = 0;
        static unsigned int imgY = 0;
        static unsigned int imgZ = 0;
        static unsigned int halfJobs = 0;
        static unsigned int fullJobs = 0;
        static unsigned int halfStepZ = 0;
        static unsigned int halfStepY = 0;
        static unsigned int halfStepX = 0;
        int algostarts = 0;
//CPUcount
uint64_t CPUCount(void);
uint64_t TimeCounter(void);

//fpu
void fpu_check(void);
void as2v_MT(double*outz, double*AScanz, unsigned int n_AScanz, double*bufferz, float*pix_vectz,
		    unsigned int n_Xz, float*rec_posz, float*send_posz, float*speedz, float*resz,
		    float*timeintz, double*AScan_complexz,
		    double*buffer_complexz, double*out_complexz, unsigned int n_Yz, unsigned int n_Zz,
		    double* IMAGE_SUMz, double* IMAGE_SUM_complexz);


//thread function
void *thread_function(void *arg);

//thread benchmark
void as2v_bench( uint64_t lat[],  uint64_t through[]);

//ellipsoide backprojection
void as2v_complex(Addsig2vol_param *, Addsig2vol_param*, Addsig2vol_param*, Addsig2vol_param*);
void as2v_complex_sm(Addsig2vol_param *, Addsig2vol_param*, Addsig2vol_param*, Addsig2vol_param*);
void as2v_c(Addsig2vol_param *, Addsig2vol_param*, Addsig2vol_param*, Addsig2vol_param*);
//Xsum and interpol
void xsum_complex(Addsig2vol_param *, Addsig2vol_param*, Addsig2vol_param*, Addsig2vol_param*);
void xsum_c(Addsig2vol_param *, Addsig2vol_param*, Addsig2vol_param*, Addsig2vol_param*);
void resetJobList(unsigned int x, unsigned int y, unsigned int z);

//Empty functions
void mutePrint(char* string, ...){}
void muteUpstream(cArray* array){}

#ifdef addsig2vol_3_MUTED
    static void (*print)(char*, ...) = &mutePrint;
#else
    static void (*print)(char*, ...) = &printf;
#endif

static void (*upStreamCArray)(cArray*) = &muteUpstream;

//////////////////////////////////////////////////////////////End declarations


void as2v_setPrintCallback(void (*callback)(char*, ...)) {
        if (callback) print = callback;
}
void as2v_setUpstreamCallback(void (*callback)(cArray*)) {
    if (callback) upStreamCArray = callback;
}


void resetJobList(unsigned int x, unsigned int y, unsigned int z)
{

}


void as2v_MT(double*outz, double*AScanz, unsigned int n_AScanz, double*bufferz, float*pix_vectz,
    unsigned int n_Xz, float*rec_posz, float*send_posz, float*speedz, float*resz,
    float*timeintz, double*AScan_complexz,
    double*buffer_complexz, double*out_complexz, unsigned int n_Yz, unsigned int n_Zz,
    double *IMAGE_SUMz, double *IMAGE_SUM_complexz)
    {
        tsclock(0);
        tsclock(1);
        //pthread variables
        #ifdef p_threads
        pthread_t mythread[NUMCORES]; //numCPU -1
        Addsig2vol_param threadArg[NUMCORES]; //numCPU -1
        int rc = 0; //return-value from thread functions
        #endif
        //NOTE: This must be set on 0, otherwise results doesnt match
        float pix_vecz_buffer[NUMCORES][3]= {0};
        unsigned int n_Zz_start = 0;
        //unsigned int nCores = NUMCORES;
        unsigned int i,j;

        // Hier muss pix pointer für aktuellen AScan rein
        globalPixPointer[0] = *pix_vectz;
        globalPixPointer[1] = *(pix_vectz+1);
        globalPixPointer[2] = *(pix_vectz+2);

    //    nCores = 1;
        L3CACHE_SIZE = 3027000;

        // Definition nach Cache
        float cacheFracture = 2;
        unsigned int imagePrecision = 8;
        //Anzahl doubles per thread
        float workSegment = (float) L3CACHE_SIZE/(cacheFracture*imagePrecision*nCores); //[Doubles per thread]
        //Anzahl der jobs mindestens, um L3 Größe einzuhalten (Anzahl der Stücke, die mind. geschnitten werden müssen)
        float workPackages = (float)n_Xz*n_Yz*n_Zz/workSegment;

        // Check that there is at least one package for each thread
        if(workPackages < nCores) {
            workPackages = nCores;
        }
        //berechne Elemente für jedes package
        float imageFraction = 1/workPackages;
        unsigned int elementsPerPackage = (unsigned int) n_Xz*n_Yz*n_Zz*imageFraction;
        // TODO Hier kann man Größe auch forcen für tests
        if (elementsPerPackage < 4) elementsPerPackage = 4;

        //elementsPerPackage = 4;

        imgX = n_Xz;
        imgY = n_Yz;
        imgZ = n_Zz;

        // force layout: Für Interlacing muss hier einfach weiter geteilt werden, s.d. fullZ und fullY 0 werden, fullX = 0;
        posZ = floor((float)elementsPerPackage/(n_Xz*n_Yz));
        posY = floor((float)(elementsPerPackage-n_Xz*n_Yz*posZ)/n_Xz);
        posX = elementsPerPackage-posZ*n_Yz*n_Zz-posY*n_Xz;
        // posX needs to multiple of 4;
        posX -= posX%4;


        if (posZ > 0){
            segmentedAxis = ZAXIS;
            //printf("ZAXIS\n");
            posX = 0;
            posY = 0;
            jobs = ceil((float)n_Zz/posZ);
            stepZ = posZ;
            stepX = n_Xz;
            stepY = n_Yz;
            fullJobs = floor((float)n_Zz/posZ);
            halfStepZ = n_Zz - fullJobs*stepZ;
            if (halfStepZ == 0) halfStepZ = stepZ;
            else halfJobs = 1;
            jobs = ceil((float)n_Zz/posZ);
        }
        if (posY > 0){
            segmentedAxis = YAXIS;
            //printf("YAXIS\n");

            posX = 0;
            posZ = 0;
            stepY=posY;
            stepX = n_Xz;
            stepZ = 1;
            fullJobs = floor((float)n_Yz/posY);
            halfStepY = n_Yz - fullJobs*stepY;
            if (halfStepY == 0) halfStepY = stepY;
            else halfJobs = 1;
            jobs = ceil((float)n_Yz/posY)*n_Zz;
        }
        if (posX > 0){
            segmentedAxis = XAXIS;
            //printf("XAXIS\n");

            posY = 0;
            posZ = 0;
            stepY = 1;
            stepZ = 1;
            stepX = posX;
            fullJobs = floor((float)n_Xz/posX);
            halfStepX = n_Xz - fullJobs*stepX;
            if (halfStepX == 0) halfStepX = stepX;
            else halfJobs = 1;
            jobs = ceil((float)n_Xz/posX)*n_Yz*n_Zz;
        }

        #ifdef addsig2vol_debug
        print("Z-Dim multithreading\n");
        #endif

        //print("elementsPerPackage %i, jobs %i | posX, posY, posZ: %i %i %i | x, y, z: %i %i %i \n", elementsPerPackage, jobs, posX, posY, posZ, n_Xz, n_Yz, n_Zz);
        unsigned int currentZ = 0;
        unsigned int currentX = 0;
        unsigned int currentY = 0;
        unsigned int nextStepX = 0;
        unsigned int nextStepY = 0;
        unsigned int nextStepZ = 0;

        //Generate parameter structs for Z-multithreading

        for (j = 0; j<nCores; j++)
        {

            //set picture startpoint


            pix_vecz_buffer[j][0]= *pix_vectz+currentX* (*resz);
            pix_vecz_buffer[j][1]= *(pix_vectz+1)+currentY* (*resz);
            pix_vecz_buffer[j][2]= (*(pix_vectz+2))+currentZ* (*resz);
            int n_Zz_start = currentZ*n_Xz*n_Yz + currentY*n_Xz + currentX;
            threadInfo[j].x = currentX;
            threadInfo[j].y = currentY;
            threadInfo[j].z = currentZ;
            threadInfo[j].currentJob = j%(fullJobs+halfJobs);
            threadInfo[j].currentTotalJob = j;

            nextStepX = stepX;
            if(n_Xz-currentX<stepX) nextStepX = n_Xz-currentX;
            nextStepY = stepY;
            if(n_Yz-currentY<stepY) nextStepY = n_Yz-currentY;
            nextStepZ = stepZ;
            if(n_Zz-currentZ<stepZ) nextStepZ = n_Zz-currentZ;

            if (nextStepX == 0) nextStepX++;
            if (nextStepY == 0) nextStepY++;
            if (nextStepZ == 0) nextStepZ++;

            //fill parameter struct
            threadArg[j].outz=outz+n_Zz_start;//
            threadArg[j].AScanz=AScanz;
            threadArg[j].n_AScanz=n_AScanz;
            threadArg[j].bufferz=bufferz;
            threadArg[j].pix_vectz=&(pix_vecz_buffer[j][0]);/*mxGetPr(pix_vect)*/
            threadArg[j].n_Xz=nextStepX;
            threadArg[j].rec_posz=rec_posz;
            threadArg[j].send_posz=send_posz;
            threadArg[j].speedz=speedz;
            threadArg[j].resz=resz;
            threadArg[j].timeintz=timeintz;
            threadArg[j].AScan_complexz=AScan_complexz;
            threadArg[j].buffer_complexz=buffer_complexz;
            threadArg[j].out_complexz=out_complexz+n_Zz_start;//
            threadArg[j].n_Yz=nextStepY;
            threadArg[j].n_Zz=nextStepZ;/*n_Zz*/
            threadArg[j].IMAGE_SUMz=IMAGE_SUMz+n_Zz_start;//
            threadArg[j].IMAGE_SUM_complexz=IMAGE_SUM_complexz+n_Zz_start; //
            threadArg[j].qwb0 = j; // Thread ID, this is a hack
            // next start point
            currentX += posX;
            currentY += posY;
            currentZ += posZ;
            if(currentX >= n_Xz){ currentX =0; currentY++;}
            if(currentY >= n_Yz){ currentY =0; currentZ++;}
            if(currentZ >= n_Zz){} // finished, last case
        }

        tsclock(1);

    //    interpol & X-SUM (in the case of NUMCORE=1 only call)
            #ifdef C_CODE
        xsum_c(&threadArg[0],&threadArg[0],&threadArg[0],&threadArg[0]);
        #else
        xsum_complex(&threadArg[0],&threadArg[0],&threadArg[0],&threadArg[0]);
         #endif

        #ifdef SAVEDATA    ///// save buffers
        mkdir("data/buffers", 0777);
        char p[50];
        sprintf(p, "data/buffers/buffer_%d", count);
        saveDoubleArrayUnstruct(buffers, INTERP_RATIO * threadArg[nCores-1].n_AScanz, 1, 1, p)
        #endif
        nextJobWaiting = nCores;

        tsclock(2);
        ////release threads

        for (i=0;i<nCores;i++)
        {

            #ifdef p_threads
            rc = pthread_create( &(mythread[i]), NULL, thread_function, &(threadArg[i]));
            if (rc) { print("ERROR: return code from pthread_create() is %d\n", rc); return;}
            #endif
        }
        // AT THIS POINT IMAGE_SUM_1 not OUT_0 anymore (from 0 t0 8?)
        //-> Because of threading! Not all finish the same time

        //catches threads again
        for (i=0;i<nCores;i++)
        {
            #ifdef p_threads
            rc = pthread_join ( mythread[i], NULL );
            if (rc) { print("ERROR: return code from pthread_join() is %d\n", rc); return;}
            #endif
        }
        tsclock(2);

        #ifdef SAVEDATA          ///// save outputs
        mkdir("data/outputs", 0777);
        char p1[50];
        sprintf(p1, "data/outputs/outs_%d", count);
        saveDoubleArrayUnstruct(outz, n_Xz, n_Yz, n_Zz, p1);
        #endif

        //set because because potentially reduced by imagesize
        //if (nCores_bench >0) nCores = nCores_bench;
        #ifdef SAVEDATA
        count++;
        #endif
        tsclock(0);
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void as2v_c(Addsig2vol_param* tt, Addsig2vol_param* t1, Addsig2vol_param* t2, Addsig2vol_param* t3)
{
    ///TODO: COMPLEX part!!!!

    float dist_sv[3] = {0,0,0};
    float dist_rv[3] = {0,0,0};
    float factor = 0;

    unsigned int index, image_index = 0;
    unsigned int z, y, x, sampl = 0;

    //decompose variables from struct
    double*outz = tt->outz;
    double*AScanz = tt->AScanz;
    double*out_complexz=tt->out_complexz;
    double*bufferz = tt->bufferz;
    float*pix_vectz = tt->pix_vectz;
    double*buffer_complexz=tt->buffer_complexz;
    float*rec_posz = tt->rec_posz;
    float*send_posz = tt->send_posz;
    float*speedz = tt->speedz;
    float*resz = tt->resz;
    float*timeintz = tt->timeintz;
    double*AScan_complexz = tt->AScan_complexz;
    double *IMAGE_SUMz = tt->IMAGE_SUMz;
    double *IMAGE_SUM_complexz = tt->IMAGE_SUM_complexz;
    unsigned int n_Yz = tt->n_Yz;
    unsigned int n_Zz = tt->n_Zz;
    unsigned int n_AScanz = tt->n_AScanz;
    unsigned int n_Xz = tt->n_Xz;

    ///bildgebung
    factor =  INTERP_RATIO / (*speedz * *timeintz);

    for (z=1; z<=n_Zz; z++)
    {
        //dist_sv[2] = pow(send_posz[2] - (((float)z* *resz)+pix_vectz[2]) ,2);
        dist_sv[2]  = send_posz[2] - (((float)z* *resz)+pix_vectz[2]);
        dist_sv[2] *= dist_sv[2];
        //dist_rv[2] = pow(rec_posz[2] - (((float)z* *resz)+pix_vectz[2]) ,2);
        dist_rv[2]  = rec_posz[2] - (((float)z* *resz)+pix_vectz[2]);
        dist_rv[2] *= dist_rv[2];

        for (y=1; y<=n_Yz; y++)
        {
            //dist_sv[1] = pow(send_posz[1] - ((y* *resz)+pix_vectz[1]) ,2);
            dist_sv[1]  = send_posz[1] - (((float)y* *resz)+pix_vectz[1]);
            dist_sv[1] *= dist_sv[1];
            //dist_rv[1] =  pow(rec_posz[1] - ((y* *resz)+pix_vectz[1]) ,2);
            dist_rv[1]  = rec_posz[1] - (((float)y* *resz)+pix_vectz[1]);
            dist_rv[1] *= dist_rv[1];

            for (x=1; x<=n_Xz; x++)
            {
                //dist_sv[0] = pow(send_posz[0] - (x* *resz),2);
                //dist_rv[0] =  pow(rec_posz[0] - (x* *resz),2);
                dist_sv[0] = send_posz[0] - ((x* (*resz))+pix_vectz[0]);
                dist_rv[0] =  rec_posz[0] - ((x* (*resz))+pix_vectz[0]);

                //dist = (sqrt(dist_sv[1]+ dist_sv[2] + (dist_sv[0]*dist_sv[0])) + sqrt(dist_rv[1]+ dist_rv[2]+ (dist_rv[0]*dist_rv[0])) );
                index = (unsigned int) floor( ( sqrt(dist_sv[1]+ dist_sv[2] + (dist_sv[0]*dist_sv[0])) + sqrt(dist_rv[1]+ dist_rv[2]+ (dist_rv[0]*dist_rv[0])) ) * factor);

                if ((index >= n_AScanz*INTERP_RATIO) | (index < 0)){
                    outz[image_index] = IMAGE_SUMz[image_index]; //nix addiert
                }
                else{
                    outz[image_index] = IMAGE_SUMz[image_index] + bufferz[index];//AScanz[index];
                }
                image_index++;
            }
        }
    }
}


///////////////////////////////////////////////

void xsum_c(Addsig2vol_param* tt, Addsig2vol_param* t1, Addsig2vol_param* t2, Addsig2vol_param* t3)
{  ///TODO: COMPLEX part!!!!

float dist_sv[3] = {0,0,0};
float dist_rv[3] = {0,0,0};
float factor = 0;

unsigned int image_index = 0;
unsigned int i,j,sampl = 0;

double i_buffer=0.0;

double* sec_buffer;

//decompose variables from struct
  /* as2v_c(tt->outz, tt->AScanz, tt->n_AScanz, tt->bufferz, tt->pix_vectz,
		    tt->n_Xz, tt->rec_posz, tt->send_posz, tt->speedz, tt->resz,
		    tt->timeintz, tt->AScan_complexz,
		    tt->buffer_complexz, tt->out_complexz, tt->n_Yz, tt->n_Zz,
		    tt->IMAGE_SUMz, tt->IMAGE_SUM_complexz);*/

		double*outz = tt->outz;
        double*AScanz = tt->AScanz;
        double*out_complexz=tt->out_complexz;
        double*bufferz = tt->bufferz;
        float*pix_vectz = tt->pix_vectz;
		double*buffer_complexz=tt->buffer_complexz;
        float*rec_posz = tt->rec_posz;
        float*send_posz = tt->send_posz;
        float*speedz = tt->speedz;
        float*resz = tt->resz;
        float*timeintz = tt->timeintz;
        double*AScan_complexz = tt->AScan_complexz;
		double *IMAGE_SUMz = tt->IMAGE_SUMz;
        double *IMAGE_SUM_complexz = tt->IMAGE_SUM_complexz;
        unsigned int n_Yz = tt->n_Yz;
        unsigned int n_Zz = tt->n_Zz;
        unsigned int n_AScanz = tt->n_AScanz;
        unsigned int n_Xz = tt->n_Xz;

#ifdef addsig2vol_debug
for (i=0;i<n_AScanz*INTERP_RATIO;i++)
{	bufferz[i] = i; //set marking for NON-set or initalized values
}
#endif

//C-Version needs second buffer
sec_buffer = malloc(INTERP_RATIO * n_AScanz * sizeof(double));


//interp for first Samples
for (i=0;i<(unsigned int) floor(INTERP_RATIO/2);i++)
{	sec_buffer[i] = AScanz[0] * (unsigned int) ((i+1)/floor(INTERP_RATIO/2));
}

//almost all samples
for (i=0;i<n_AScanz-1;i++)
{    	for (j=0;j<INTERP_RATIO;j++)
	{	sec_buffer[(unsigned int) floor(INTERP_RATIO/2)+(i* INTERP_RATIO)+j] = AScanz[i+1] * j/INTERP_RATIO + AScanz[i] * (INTERP_RATIO-j)/INTERP_RATIO; //
	}
}

//interp for last Samples
for (i=0;i<(unsigned int) floor(INTERP_RATIO/2)+1;i++)
{	sec_buffer[n_AScanz*INTERP_RATIO-(unsigned int) floor(INTERP_RATIO/2)-1+i] = AScanz[n_AScanz-1] * ((floor(INTERP_RATIO/2)+1-i) / (floor(INTERP_RATIO/2)+1));
}
///end interp



/////xsum
sampl = (unsigned int)(ceil((float)1.7*(( *resz / *speedz)/ (*timeintz/INTERP_RATIO)) /2)); //halbe breite

i_buffer = 0;
for (i=0;i<sampl;i++)
{	i_buffer = i_buffer + sec_buffer[i]/(2*sampl);
}

for (i=0;i<sampl;i++)
{	if (i+sampl<n_AScanz*INTERP_RATIO){
    i_buffer = i_buffer +sec_buffer[i+sampl]/(2*sampl);
	bufferz[i] = i_buffer;}
}

for (i=sampl;i<(n_AScanz*INTERP_RATIO)-sampl;i++)
{ if (i+sampl<n_AScanz*INTERP_RATIO){
    i_buffer = i_buffer + sec_buffer[i+sampl]/(2*sampl) - sec_buffer[i-sampl]/(2*sampl);
	bufferz[i] = i_buffer;}
}

for (i=n_AScanz*INTERP_RATIO-sampl;i<n_AScanz*INTERP_RATIO;i++)
{	if (i-sampl>=0){
    i_buffer = i_buffer - sec_buffer[i-sampl]/(2*sampl);
	bufferz[i] = i_buffer / (sampl-(n_AScanz*INTERP_RATIO)-i);}
}
/////end xsum

//free sec_buffer (only buffer needed now)
free(sec_buffer);

}

///////////////////////////////////////////////


void *thread_function(void *argument)
{
       Addsig2vol_param* arg = (Addsig2vol_param*) argument;
       unsigned int id = arg->qwb0;

       float pix_vecz_buffer[3];
       float resz = *(arg->resz);
       unsigned int currentXn = threadInfo[id].x;
       unsigned int currentYn = threadInfo[id].y;
       unsigned int currentZn = threadInfo[id].z;
       unsigned int currentJob = threadInfo[id].currentJob;
       unsigned int currentTotalJob = threadInfo[id].currentTotalJob;
       //print("T%i: steps x,y,z: %i %i %i\n", id, stepX, stepY, stepZ);
       unsigned int internJob;
       unsigned int totalElementJumps;
       unsigned int nextStepX;
       unsigned int nextStepY;
       unsigned int nextStepZ;
       unsigned int oldCurrentX;
       unsigned int oldCurrentY;
       unsigned int oldCurrentZ;
       unsigned int jumps;
       unsigned int jumpsZ;
       unsigned int delta;
       while(1){

       //print("T%i: jumped %i, next work starts on x,y,z: %i %i %i\n", id, totalElementJumps,currentXn, currentYn, currentZn);


       //print("T%i: call assembler code\n", id);
       //printf("pixvektor x, y, z: %f %f %f\n",pix_vecz_buffer[0], pix_vecz_buffer[1], pix_vecz_buffer[2]);

       #ifdef C_CODE
       as2v_c(arg,arg,arg,arg); //compatible win64 & linxu64 function-call
       #else
       if (addsig2vol_mode==0) as2v_complex(arg,arg,arg,arg);
   //    if (addsig2vol_mode==2) as2v_complex_sm(arg,arg,arg,arg);

       #endif

  //decomposing for old function
  /* as2v_c(tt->outz, tt->AScanz, tt->n_AScanz, tt->bufferz, tt->pix_vectz,
           tt->n_Xz, tt->rec_posz, tt->send_posz, tt->speedz, tt->resz,
           tt->timeintz, tt->AScan_complexz,
           tt->buffer_complexz, tt->out_complexz, tt->n_Yz, tt->n_Zz,
           tt->IMAGE_SUMz, tt->IMAGE_SUM_complexz);*/


           pthread_mutex_lock(&threadMutex);
           // print("T%i, lastJob: %i, lastJumps: %i, in fullvector: %i\n", id, currentTotalJob, jumps, currentJob);
           // Mind the order
           internJob = nextJobWaiting;
             nextJobWaiting++;
           pthread_mutex_unlock(&threadMutex);
           if(id == 0){tsclock(12);}
           // Anzahl der Jobsprünge (global)
           delta = (internJob-currentTotalJob);
           //print("T%i, delta: %i\n", id, delta);
           // nächste Jobnummer dieses threads
           currentTotalJob+= delta;
           currentJob+= delta;
           jumps = floor((float)(currentJob)/(fullJobs+halfJobs));
           currentJob = currentJob % (fullJobs+halfJobs);

          // printf("T%i: delta %i, nextinternJob %i, modulo %i, jumps %i  \n", id, delta, currentTotalJob, currentJob, jumps);
          // printf("T%i, lastCurrentZN %i, lastNextStep: %i \n", id, currentZn, nextStepZ);

           switch(segmentedAxis){
               case ZAXIS:;
                    //print("ZAXIS\n");

                   currentZn = currentTotalJob*posZ;
                    //printf("T%i, currentTotalJob %i/%i\n", id, currentTotalJob, fullJobs);
                    //
                    // printf("T%i, currentZn %i, nextStepZ: %i \n", id, currentZn, nextStepZ);

                   if(currentZn >= imgZ){if(id == 0){tsclock(12);} return NULL;} // finished, last case
                   nextStepZ = posZ;
                   totalElementJumps = delta*posZ*imgX*imgY;
                   if(currentJob ==  fullJobs+halfJobs-1){
                       nextStepZ = halfStepZ;
                      // printf("FINAL RUN\n");
                   }
                   nextStepX = imgX;
                   nextStepY = imgY;
                   //wait(0.000000001);
                   break;

               case YAXIS:;
                    //print("YAXIS\n");
                   oldCurrentY = currentYn;
                   currentYn = currentJob*posY;
                   currentZn += jumps;
                   if(currentZn >= imgZ){if(id == 0){tsclock(12);} return NULL;} // finished, last case

                   nextStepY = stepY;
                   if(jumps == 0) totalElementJumps = delta*stepY*imgX;
                   else totalElementJumps = (imgY-oldCurrentY)*imgX + currentYn*imgX +  (jumps-1)*imgY*imgX;
                   if(currentJob ==  fullJobs+halfJobs-1){
                       nextStepY = halfStepY;
                   }
                   nextStepX = imgX;
                   nextStepZ = 1;
                   break;
               case XAXIS:;
                    //print("XAXIS\n");
                   oldCurrentX = currentXn;
                   currentXn = currentJob*stepX;
                   // Insgesamte Anzahl an jumps (overflow jumps)
                   jumpsZ = floor(currentYn+jumps/imgY);
                   if(jumpsZ == 0) currentYn = currentYn + jumps;
                   else currentYn = jumps%imgY;
                   currentZn += jumpsZ;
                   if(currentZn >= imgZ){if(id == 0){tsclock(12);}return NULL;} // finished, last case

                   nextStepX = stepX;
                   if(jumps == 0) totalElementJumps = nCores*stepX;
                   // TODO ausrechnen!!
                   else totalElementJumps = (imgX-oldCurrentX) + currentXn + jumps*imgX;

                   if(currentJob== fullJobs){
                       nextStepX = halfStepX;
                   }
                   nextStepY = 1;
                   nextStepZ = 1;
                   break;

           }

       pix_vecz_buffer[0]=globalPixPointer[0]+currentXn* (resz);
       pix_vecz_buffer[1]=globalPixPointer[1]+currentYn* (resz);
       pix_vecz_buffer[2]=globalPixPointer[2]+currentZn* (resz);

      // print("T%i: current x,y,z: %i %i %i\n", id, currentXn, currentYn, currentZn);
      // print("T%i: nextsteps x,y,z: %i %i %i, totalElementJumps: %i\n", id, nextStepX, nextStepY, nextStepZ, totalElementJumps);
       arg->outz+=totalElementJumps;//
       arg->pix_vectz=&(pix_vecz_buffer[0]);//
       arg->n_Xz=nextStepX;
       arg->out_complexz+=totalElementJumps;//
       arg->n_Yz=nextStepY;
       arg->n_Zz=nextStepZ;/*n_Zz*/
       arg->IMAGE_SUMz+=totalElementJumps;//
       arg->IMAGE_SUM_complexz+=totalElementJumps; //

       if(id == 0){tsclock(12);}
   }
    return NULL;
}


////////////////////////////////////////////////////////////////////

void as2v_benchLocal(){
    as2v_bench(&throughput[0], &latency[0]);
}

void as2v_bench(uint64_t throughput[], uint64_t latency[])
{

    #pragma fenv_access (on)
    uint32_t i,j,l,k;
	uint32_t n_AScan;
	uint32_t n_AScan_block;
	int n_X;
	int n_Y;
	int n_Z;
	int n_IMAGE;
	// mwSize setImageDim[3];
	// mwSize setBufferDim[2];
	// mwSize setAScanDim[2];
	float pix_vec_bench[3]  = {2,1,77};
	//float pix_vec_bench[3]  = {2,1,77};
	float send_vec_bench[3] = {1,2,2};
	float rec_vec_bench[3]  = {5,6,9};
    float float_bench = 1;

	double* pr;
   /* double*pr1;
    double* pr2;
    double*pr3;
    double*pr4;*/

    #define MAX_AVERAGE 128
    uint64_t average_buffer[MAX_AVERAGE]={0};
    uint64_t throughput_sort_buffer[NUMCORES]={0};
    uint64_t counter, counter2, stdabw, mittelw;
    uint64_t bench_ref_time=0; //ns

    uint64_t minBenchTime = 100000000; //ns

    uint32_t minAverage = 7;
    uint32_t average = 8;
    uint32_t nVoxel_throughput = 128;

    double *AScan_bench;
	double *buffer_bench;
	double *out_bench;
	double *image_sum_bench;
	double *time_bench;

    //autodetect free memory
   /*  do { nVoxel_throughput=(uint32_t) nVoxel_throughput/2;
       pr = mxMalloc(nVoxel_throughput*nVoxel_throughput*nVoxel_throughput * sizeof(double));
     if (pr!=NULL)  mxFree(pr);
       nVoxel_throughput=(uint32_t) nVoxel_throughput/2; //nocnmal halbieren f�r 2Bilder
   } while (pr==NULL);*/

  print("Benchmarking System:\nLatency with %d Byte Image\nThroughput with %d kByte Image.\n",(int)(MIN_VOXEL*NUMCORES*sizeof(double)),(int)(nVoxel_throughput*nVoxel_throughput*nVoxel_throughput*sizeof(double))/1024);
  print("Threads | Throughput in MVoxel/s                     | Latency in nsec                              | Median-Size\n");
  print("        | Median | Mean   | Min    | Max    | Std    | Median    | Min       | Max       | Std      | \n");
  print("-------------------------------------------------------------------------------------------------------------------\n");

  //check for minimum amount
  if (nVoxel_throughput<NUMCORES*MIN_VOXEL) nVoxel_throughput=NUMCORES*MIN_VOXEL;
  /////kill eventually running tic-toc!!! (not needed)
   //mexCallMATLAB(0, NULL, 0, NULL, "toc");

	////fix variables
    time_bench = malloc(sizeof(double));
	n_AScan        = 3000;        //gesamtanzahl elemente IN EINEM ASCAN!!!
	n_AScan_block  = 100;    //2 dim %number of parallel

    //buffer

    print("buffer size %i\n", (n_AScan*INTERP_RATIO));
    buffer_bench = malloc(n_AScan*INTERP_RATIO *sizeof(double));

    //pr1=pr;

	AScan_bench = malloc(n_AScan *n_AScan_block*sizeof(double));




    ///////////////throughput calc (assumption for Latency = Zero because working in saturation)

	n_X             = nVoxel_throughput;
	n_Y             = nVoxel_throughput;
    n_Z             = nVoxel_throughput;

    //number of voxel
		n_IMAGE         = n_X * n_Y * n_Z;

	out_bench = malloc(n_IMAGE*sizeof(double));
    image_sum_bench = malloc(n_IMAGE*sizeof(double));

        print("Voxels in x, y, z: [%i, %i, %i]\n", n_X, n_Y, n_Z );
        print("Voxels total: %i\n", n_IMAGE);
        print("Buffer total: %i\n", INTERP_RATIO * n_AScan);

       //first call to get REAL memory from system (WARMUP later on faster)
      //set nCore!!!
	  nCores = 1;
      do{
      counter = TimeCounter();
      as2v_MT((out_bench), (AScan_bench), n_AScan, (buffer_bench),
			     &pix_vec_bench[0], n_X, &rec_vec_bench[0], &send_vec_bench[0],
			     &float_bench,
			     &float_bench, &float_bench, NULL, NULL, NULL, n_Y, n_Z, (image_sum_bench), NULL);
       counter2 = TimeCounter();
       } while (counter2<counter); //retry on error like used wrong core

        //get througput time time for setup average
        bench_ref_time= counter2-counter;
        //print("benchreftime %llu c2:%llu c:%llu\n",bench_ref_time, counter, counter2);
        average = (uint32_t) ((minBenchTime/bench_ref_time)+1); //integer ceil

        if (average < minAverage) average = minAverage;
        if (average > MAX_AVERAGE) average = MAX_AVERAGE;


	for (i=1;i<=NUMCORES;i++)
	{
	  //set nCore!!!
		nCores = i;

	    //benchmark throughput
        //mexCallMATLAB(0, NULL, 0, NULL, "tic");

		for (j=0;j<average;j++)
         {
         do{ counter=TimeCounter();
          //no sizeof(double) needed because compilers assumes already double as datatype for pointer!!!
		  as2v_MT((out_bench), (AScan_bench), n_AScan, (buffer_bench),
			     &pix_vec_bench[0], n_X, &rec_vec_bench[0], &send_vec_bench[0],
			     &float_bench,
			     &float_bench, &float_bench, NULL, NULL, NULL, n_Y, n_Z, (image_sum_bench), NULL);
          /*as2v_MT(pr3, pr2, n_AScan, pr1,
			     &pix_vec_bench[0], n_X, &rec_vec_bench[0], &send_vec_bench[0],
			     &float_bench,
			     &float_bench, &float_bench, NULL,NULL, NULL, n_Y, n_Z, pr4,NULL);*/
		  //mexCallMATLAB(1, &time_bench, 0, NULL, "toc");
          //print("%llu ms\n",((TimeCounter()-counter)/1000000));
          counter2=TimeCounter();
         } while(counter2<counter); //retry on error like used wrong core
          average_buffer[j]= counter2-counter;
        }
        //tsprint(1,TS_MILI);
        //tsclear(1);

        //bubblesort (small time top) for median
		  for (k=average-1;k>0;k--)
          {  for (l=average-1;l>0;l--){
                 if (average_buffer[l]<average_buffer[l-1]) {counter2=average_buffer[l-1]; average_buffer[l-1] = average_buffer[l]; average_buffer[l]=counter2;}
             }
          }

         mittelw=0;
         for  (k=0;k<average;k++)  {mittelw=average_buffer[k]+mittelw;}
         mittelw=(uint64_t) mittelw/average;

         stdabw=0;
         for  (k=0;k<average;k++)
         {if (mittelw>average_buffer[k]) stdabw=stdabw+(mittelw-average_buffer[k])*(mittelw-average_buffer[k]); else stdabw=stdabw+(average_buffer[k]-mittelw)*(average_buffer[k]-mittelw);}
         stdabw = (uint64_t) sqrt( (int64_t)stdabw/(int64_t)average);

         #ifdef addsig2vol_debug
          //print("minTime: %f, timeCounter: %llu counter: %llu\n",(float)(uint64_t)bench_ref_time,counter2,counter);
          // for (k=0;k<average;k++){ print("%llu average\n",average_buffer[k]);}
         #endif

        //minimum selection
        bench_ref_time = (uint64_t) average_buffer[0];
        //median selection
        bench_ref_time = (uint64_t) average_buffer[(uint32_t) ceil(average/2)];

        //throughput[i-1] = (uint64_t) ((double)n_IMAGE/ ((double) ((uint32_t) bench_ref_time)/1000)); //gekuerzt ns und MVoxel ->Mvoxel/s // index 0 = core1; index1 = core 2 etc...(!)
        throughput[i-1] =  ((1000*(uint64_t)n_IMAGE)/ bench_ref_time); //gekuerzt ns und MVoxel ->Mvoxel/s // index 0 = core1; index1 = core 2 etc...(!)

        print("%7i |%7llu |%7llu |%7llu |%7llu |%7llu", i, (int64_t)throughput[i-1],((1000*(int64_t)n_IMAGE)/(int64_t)mittelw), (1000*(int64_t)n_IMAGE)/((int64_t)average_buffer[average-1]), (1000*(int64_t)n_IMAGE)/((int64_t)average_buffer[0]), (1000*(int64_t)n_IMAGE)/((int64_t)stdabw));


       //get throughput time time for setup average
       average = (uint32_t) ((minBenchTime/bench_ref_time)+1); //in ns; +1=integer-ceil
       if (average < minAverage) average = minAverage;
       if (average > MAX_AVERAGE) average = MAX_AVERAGE;

		 // printf("%e MVoxel/s for %i threads, %e sec through\n",(double)(4*n_IMAGE)/(*mxGetPr(time_bench)),n_IMAGE,*mxGetPr(time_bench));
      //  	  printf("%f",(float)(((4*2000000)/(*(double*)mxGetPr(time_bench)))));

//         //first call to get REAL memory form system (later on faster)
//       as2v_MT(mxGetPr(out_bench), mxGetPr(AScan_bench), n_AScan, mxGetPr(buffer_bench),
// 			     &pix_vec_bench[0], n_X, &rec_vec_bench[0], &send_vec_bench[0],
// 			     &float_bench,
// 			     &float_bench, &float_bench, mxGetPi(AScan_bench), mxGetPi(
// 				     buffer_bench), mxGetPi(
// 				     out_bench), n_Y, n_Z, mxGetPr(image_sum_bench), mxGetPi(
// 				     image_sum_bench));

	   ///////////////////////////////////////////////////////benchmark latency
	//	 mexCallMATLAB(0, NULL, 0, NULL, "tic");

		for (j=0;j<minAverage;j++)
         {
          do{
          counter=TimeCounter();
			//no sizeof(double) needed because compilers assumes already double as datatype for pointer!!!
		  as2v_MT((out_bench), (AScan_bench), n_AScan, (buffer_bench),
			     &pix_vec_bench[0],(unsigned int) MIN_VOXEL, &rec_vec_bench[0], &send_vec_bench[0],
			     &float_bench,
			     &float_bench, &float_bench, NULL, NULL, NULL, (unsigned int) 1,(unsigned int) NUMCORES, (image_sum_bench), NULL);

		//  mexCallMATLAB(1, &time_bench, 0, NULL, "toc");
        //latency[i] =(*mxGetPr(time_bench)/minAverage)/1;	 //assumption for 1 PIXEL!
		 counter2 = TimeCounter(); } while(counter2<counter); //retry on error like used wrong core
         average_buffer[j]= counter2-counter;
        }
        //bubblesort (small time top)
		 for (k=minAverage-1;k>0;k--)
         {  for (l=minAverage-1;l>0;l--){
               if (average_buffer[l]<average_buffer[l-1]) {counter2=average_buffer[l-1]; average_buffer[l-1] = average_buffer[l]; average_buffer[l]=counter2;}
            }
         }
         #ifdef addsig2vol_debug
            //for (k=0;k<minAverage;k++){ print("%llu average\n",average_buffer[k]);  }
        #endif

         mittelw=0;
         for  (k=0;k<minAverage;k++)  {mittelw=average_buffer[k]+mittelw;}
         mittelw=(uint64_t) mittelw/minAverage;

         stdabw=0;
         for  (k=0;k<minAverage;k++)
         {if (mittelw>average_buffer[k]) stdabw=stdabw+(mittelw-average_buffer[k])*(mittelw-average_buffer[k]); else stdabw=stdabw+(average_buffer[k]-mittelw)*(average_buffer[k]-mittelw);}
         stdabw = (uint64_t) sqrt((int64_t) stdabw/minAverage);

        //minimum selection
        latency[i-1] = (uint64_t) average_buffer[0];	 //assumption for 1 PIXEL! // index 0 = core1; index1 = core 2 etc...(!)
        //median selection
        latency[i-1] = (uint64_t) average_buffer[(uint32_t) ceil(minAverage/2)];
	    print("|%10llu |%10llu |%10llu |%10llu |%8i\n", latency[i-1],average_buffer[0],average_buffer[minAverage-1],stdabw,average);
  		}

		for (i=NUMCORES;i>0;i--)
        { throughput_sort_buffer[i-1]=throughput[i-1];
		}

      //bubblesort biggest on top (bottom up)
      for (i=NUMCORES-1;i>=1;i--)
      { if (throughput_sort_buffer[i]>(throughput_sort_buffer[i-1])) throughput_sort_buffer[i-1] = throughput_sort_buffer[i]; }

      //find core-number according to perf.-value
      for (i=0;i<NUMCORES;i++)
      { if (throughput[i] == throughput_sort_buffer[0]) break; } //break on first core-number which fits

      //set up used Cores
      nCores = i+1;
      //backup in nCores_bench
      nCores_bench = i+1;

      switch (i+1)
      {
      case 1:
          print("Detected Single-core System, 1 thread prefered\n");
          break;
      case 2:
          print("Detected Dual-core or Hyperthreading System, 2 threads prefered\n");
          break;
      case 3:
          print("Detected Triple-core or Hyperthreading System, 3 threads prefered\n");
          break;
      case 4:
          print("Detected Quad-Core system, 4 threads prefered\n");
          break;
      case 8:
          print("Detected Octa-Core system (or Quadcore with HT), 8 threads prefered\n");
          break;
      case 16:
          print("Detected 16 Core system, 16 threads prefered\n");
          break;
      default:
          print("Detected %i-core system, %i threads prefered (?)\n",i+1,i+1);
      }


      //benchmark perf-per size
      print("\nPerformance for various imagesize in Voxel (with potentially %i Cores)\n",nCores);
      print("     Voxel | Throughput in kVoxel/s         | Time in mikros  | Malloc time (mikro-sec)\n");
      print("           | Median   | Mean     | Std      |           | Median | mean  | Std  | min  | max  \n");
      print("------------------------------------------------------------------------------------------------\n");
      print("voxel throughput maximal: %i\n", nVoxel_throughput*nVoxel_throughput*nVoxel_throughput);
      for (i=MIN_VOXEL;i<=(nVoxel_throughput*nVoxel_throughput*nVoxel_throughput);i=i*2)
      {
          //print("i: %i, Nx: %i, Nz: %i \n", i, MIN_VOXEL, (uint32_t) floor(i/MIN_VOXEL) );


             tsclear(0);
             tsclear(1);
              tsclear(2);
              tsclear(12);
          for (j=0;j<minAverage;j++)
          {

              do {
                  counter=TimeCounter();
                  //no sizeof(double) needed because compilers assumes already double as datatype for pointer!!!
                  as2v_MT((out_bench), (AScan_bench), n_AScan, (buffer_bench),
                  &pix_vec_bench[0], (uint32_t) MIN_VOXEL, &rec_vec_bench[0], &send_vec_bench[0],
                  &float_bench,
                  &float_bench, &float_bench, NULL, NULL, NULL, (uint32_t) 1, (uint32_t) (i/MIN_VOXEL), (image_sum_bench), NULL);
                  counter2 = TimeCounter();  } while(counter2<counter); //retry on error like used wrong core
              average_buffer[j]= counter2-counter;

          }
          // tsprint(4,TS_MIKRO);
          // tsprint(5,TS_MIKRO);
          // tsprint(8,TS_MIKRO);
          // tsprint(7,TS_MIKRO);
           tsprint(0,TS_MIKRO);
           tsprint(1,TS_MIKRO);
           tsprint(2,TS_MIKRO);
           tsprint(12,TS_MIKRO);
           tsclear(0);
           tsclear(1);
            tsclear(2);
            tsclear(12);
          //bubblesort (small time top)
          for (k=minAverage-1;k>0;k--)
          {  for (l=minAverage-1;l>0;l--){
                     if (average_buffer[l]<average_buffer[l-1]) {counter2=average_buffer[l-1]; average_buffer[l-1] = average_buffer[l]; average_buffer[l]=counter2;}
                 }
          }
         mittelw=0;
         for  (k=0;k<minAverage;k++)  {mittelw=average_buffer[k]+mittelw;}
         mittelw=(uint64_t) mittelw/minAverage;

         stdabw=0;
         for  (k=0;k<minAverage;k++)
         {if (mittelw>average_buffer[k]) stdabw=stdabw+(mittelw-average_buffer[k])*(mittelw-average_buffer[k]); else stdabw=stdabw+(average_buffer[k]-mittelw)*(average_buffer[k]-mittelw);}
         stdabw = (uint64_t) sqrt((int64_t) stdabw/minAverage);

          #ifdef addsig2vol_debug
           //for (k=0;k<minAverage;k++){ print("%llu values-range \n",average_buffer[k]);}
          #endif

          //minimum selection
          counter2 = (uint64_t) average_buffer[0];	 //assumption for 1 PIXEL! // index 0 = core1; index1 = core 2 etc...(!)
          //median selection
          counter2 = (uint64_t) average_buffer[(uint32_t) ceil(minAverage/2)];
          // print("%10luu , %10llu, %10llu \n",TimeCounter, counter, TimeCounter());
          //counter=pow(2,64)-counter;

          //mexCallMATLAB(1, &time_bench, 0, NULL, "toc");
          //print("%llu ms\n",((TimeCounter()-counter)/1000000));
          print("%10i | %8llu | %8llu | %8llu |%8llu ",i,(1000000*(uint64_t)i)/(counter2),(1000000*(uint64_t)i)/(mittelw),((uint64_t)i*1000000)/(stdabw),(uint64_t)(counter2/1000) );

          //fix  (UGLY!!!!)
          nCores=nCores_bench;

          //benchmark mem-alloc
          minAverage=100;

              for (j=0;j<minAverage;j++)
              {
                  do {
                      free(image_sum_bench);

                      counter=TimeCounter();

                      image_sum_bench = malloc(n_IMAGE*sizeof(double));

                     counter2 = TimeCounter(); } while(counter2<counter); //retry on error like used wrong core
                     average_buffer[j]= counter2-counter;
              }
              //bubblesort (small time top)
              for (k=minAverage-1;k>0;k--)
              {  for (l=minAverage-1;l>0;l--){
                  if (average_buffer[l]<average_buffer[l-1]) {counter2=average_buffer[l-1]; average_buffer[l-1] = average_buffer[l]; average_buffer[l]=counter2;}
              }
              }

              mittelw=0;
             for  (k=0;k<minAverage;k++)  {mittelw=average_buffer[k]+mittelw;}
             mittelw=(uint64_t) mittelw/minAverage;

             stdabw=0;
             for  (k=0;k<minAverage;k++)
             {if (mittelw>average_buffer[k]) stdabw=stdabw+(mittelw-average_buffer[k])*(mittelw-average_buffer[k]); else stdabw=stdabw+(average_buffer[k]-mittelw)*(average_buffer[k]-mittelw);}
             stdabw = (uint64_t) sqrt((int64_t) stdabw/minAverage);

             //minimum selection
             counter2 = (uint64_t) average_buffer[0];	 //assumption for 1 PIXEL! // index 0 = core1; index1 = core 2 etc...(!)
             //median selection
             counter2 = (uint64_t) average_buffer[(uint32_t) ceil(minAverage/2)];
             print("| %8llu | %8llu | %8llu |%8llu |%8llu\n", (uint64_t)counter2/1000,(uint64_t)mittelw/1000,(uint64_t)stdabw/1000,(uint64_t) average_buffer[0]/1000,(uint64_t) average_buffer[minAverage-1]/1000);

          }

        //check status
        fpu_check();


        //free memory
		  free(out_bench);
		  free(image_sum_bench);
        //free memory
		  free(buffer_bench);
		  free(AScan_bench);
		  free(time_bench);
}


uint64_t TimeCounter(void) {
uint64_t counter;


    #ifdef __WIN32__ //fitting windows64 AND windows32
    #include <windows.h>
    //register uint64_t temp;
    uint64_t iFreq, iCount;
    QueryPerformanceFrequency((LARGE_INTEGER*)&iFreq);
    QueryPerformanceCounter((LARGE_INTEGER*)&iCount);
    //counter = (uint64_t) (1000000000*((double)iCount/(double)iFreq)); //f�r nSekunden (balancing der multiplikation)
    counter = (uint64_t) ((1000000000*iCount)/iFreq); //f�r nSekunden (balancing der multiplikation)

//#ifdef addsig2vol_debug
    //print("iFreq:%llu, iCount:%llu, Counter:%llu\n",iFreq,iCount,counter);
    //#endif
    #endif

    #ifdef __linux__

    struct timespec time_str;
    struct timespec time_res;
    clock_gettime(CLOCK_REALTIME, &time_str);
    counter = (uint64_t) time_str.tv_nsec + (uint64_t) (time_str.tv_sec*1000000000);

    /*
    clock_gettime(CLOCK_REALTIME, &time_str); clock_getres(CLOCK_REALTIME, &time_res);
    print("CLOCK_REALTIME clockRes:%f, nsec:%f, csec:%f\n",(double)time_res.tv_nsec,(double) time_str.tv_nsec,(double) time_str.tv_sec);
    clock_gettime(CLOCK_MONOTONIC, &time_str); clock_getres(CLOCK_MONOTONIC, &time_res);
    print("CLOCK_MONOTONIC clockRes:%f, nsec:%f, csec:%f\n",(double)time_res.tv_nsec,(double) time_str.tv_nsec,(double) time_str.tv_sec);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_str); clock_getres(CLOCK_PROCESS_CPUTIME_ID, &time_res);
    print("CLOCK_PROCESS_CPUTIME_ID clockRes:%f, nsec:%f, csec:%f\n",(double)time_res.tv_nsec,(double) time_str.tv_nsec,(double) time_str.tv_sec);
    */
      //int clock_getres(CLOCK_REALTIME, struct timespec *res);
    //int clock_gettime(clockid_t clk_id, struct timespec *tp);
    #endif
    return counter;
 }

void fpu_check()
{
// http://www.christian-seiler.de/projekte/fpmath/
    #ifdef __WIN32__
    //#include <float.h>
    uint32_t control_wordfp=0, status_wordfp=0;

    //read out
    control_wordfp = _controlfp(0, 0);//,&control_wordfp, 0);
    control_wordfp = getFPUStateX86();
    //http://software.intel.com/en-us/articles/x87-and-sse-floating-point-assists-in-ia-32-flush-to-zero-ftz-and-denormals-are-zero-daz/
    //DAZ and FTZ in MXCRS
    //FTZ: The FTZ bit (bit 15)& The underflow exception (bit 11)
    //DAZ: DAZ bit (bit 6)
    //control_wordfp = _controlfp( _CW_DEFAULT, 0xfffff);
    //control_wordfp =_controlfp(_DN_FLUSH|32 ,_MCW_DN|32 ); //_controlfp(control_wordfp | 34816, 4294967295);
    //control_wordfp =_controlfp(_PC_64, _MCW_PC); //_PC_24, _PC_53 _PC_64
    //control_wordfp =_controlfp(_RC_NEAR, _MCW_RC); //_RC_UP _RC_CHOP _RC_DOWN _RC_NEAR

    status_wordfp = _statusfp();
    _clearfp();
    #else
    #include <fpu_control.h>
    fpu_control_t control_wordfp=0, fpu_cw=0;
    _FPU_GETCW(control_wordfp);

    //Status WORD under linux????

    #endif


    print("FPU/SSE Control-register(0x%.4x): ", control_wordfp );
    //RC round-control
     switch ((control_wordfp & __FPU_CW_ROUND_MASK__)>>0)//& 3072)
    {case __FPU_CW_ROUND_NEAR__:
         print("nearest rounding");
         break;
     case __FPU_CW_ROUND_UP__:
          print("ceil-rounding");
         break;
     case __FPU_CW_ROUND_DOWN__:
          print("floor-rounding");
         break;
     case __FPU_CW_ROUND_CHOP__:
          print("truncation rounding");
         break;
    }
    // print("%x %x %x",control_wordfp & _MCW_PC, _MCW_PC,_MCW_RC);

    //PC Precision-control
    switch ((control_wordfp & __FPU_CW_PREC_MASK__ )>>0 )//& 768)
    {case __FPU_CW_PREC_SINGLE__:
         print(", internal precision float (32bit)\n");
         break;
     case __FPU_CW_PREC_DOUBLE__:
         print(", internal precision double (64bit)\n");
         break;
     case __FPU_CW_PREC_EXTENDED__:
         print(", internal precision extended double (80bit)\n");
         break;
     default :
         print(", internal precision invalid \n");
    }

#ifdef __WIN32__
    //FPU/SSE status
    print("FPU/SSE status (0x%.4x): ", status_wordfp);
    if ((status_wordfp & 32)>>5)  print("inexact ");
    if ((status_wordfp & 16)>>4)  print("underflow ");
    if ((status_wordfp &  8)>>3)  print("overflow ");
    if ((status_wordfp &  4)>>2)  print("division-by-zero ");
    if ((status_wordfp &  2)>>1)  print("denormal ");
    if ((status_wordfp &  1)>>0)  print("invalid operation mask ");
    if ( status_wordfp == 0 )  print("OK");
    print("\n");
    #endif
}



void as2v_overwriteBenchresultToThreadcount_n(uint32_t n){
    if ((n <= NUMCORES) & (n >= 1)){
        print("Clear benchmark results, manual set number of threads to %i\n", n);
        nCores_bench = n;
        //fill
        for (int i=NUMCORES; i>0; i--){
            throughput[i-1]=1;
            latency[i-1]=10000000;
        }
        throughput[nCores_bench-1]=299;
        latency[nCores_bench-1]=1;
    } else {
        print("Number of threads %i\n is invalid, benchmark results remain unchanged\n", n);
    }
}

as2v_results as2v_addsig2vol_3(cArrayDouble* AScan_realz, cArrayDouble* AScan_complexz,
            cArrayFloat* pix_vectz, cArrayFloat* rec_posz, cArrayFloat* send_posz, cArrayFloat* speedz, float* resz, float* timeintz,
		    cArrayDouble* IMAGE_SUM_realz, cArrayDouble* IMAGE_SUM_complexz, cArrayDouble* outputImage, cArrayDouble* buffer)
{
    as2v_results results = {NULL, NULL, NULL, NULL};
    //Check if minimal data is available...
    if (AScan_realz && IMAGE_SUM_realz && pix_vectz && rec_posz && send_posz && speedz && resz && timeintz){

        double* AScan_complexz_ptr = NULL;
        double* IMAGE_SUM_complexz_ptr = NULL;

        if ( (AScan_complexz==!NULL) & (IMAGE_SUM_complexz==!NULL) ){
            AScan_complexz_ptr = AScan_complexz->data;
            IMAGE_SUM_complexz_ptr = IMAGE_SUM_complexz->data;
        } else if( (AScan_complexz==NULL) ^ (IMAGE_SUM_complexz==NULL) ){
            print("Error: Mismatch complex/real AScan vs complex/real sum image, break\n");
            return results;
        }

        // Save pointers locally: Do NOT do pointer arithmetic in the structs themselves (used to reference/free data)
        double* AScan_realz_ptr = AScan_realz->data;
        double* IMAGE_SUM_realz_ptr = IMAGE_SUM_realz->data;
        float* pix_vectz_ptr = pix_vectz->data;
        float *rec_vec_ptr=rec_posz->data;
        float *send_vec_ptr=send_posz->data;
        float *speedz_vec_ptr=speedz->data;

        unsigned int i;
        unsigned int n_AScan = AScan_realz->x;
        unsigned int n_AScan_block = AScan_realz->y;
        unsigned int n_Z = IMAGE_SUM_realz->z;
        unsigned int n_Y = IMAGE_SUM_realz->y;
        unsigned int n_X = IMAGE_SUM_realz->x;
        unsigned int n_IMAGE = IMAGE_SUM_realz->len;
        unsigned int n_rec_vec_block = rec_posz->y;
        unsigned int n_send_vec_block= send_posz->y;
        unsigned int n_speed_vec_block = speedz->y;

        #ifdef addsig2vol_debug
        print("AScan_real:\t"); caMiniprint(AScan_realz);
        print("IMAGE_SUM_real:\t"); caMiniprint(IMAGE_SUM_realz);
        print("send_pos:\t"); caMiniprint(send_posz);
        print("rec_pos:\t"); caMiniprint(rec_posz);
        print("speed:\t\t"); caMiniprint(speedz);
        print("n_AScan: %i\n", n_AScan);
        print("n_AScan_block: %i\n", n_AScan_block);
        print("n_Z: %i\n", n_Z);
        print("n_Y: %i\n", n_Y);
        print("n_X: %i\n", n_X);
        print("n_IMAGE: %i\n", n_IMAGE);
        print("res: %f\n", *resz);
        print("timeint: %f\n", *timeintz);
        print("INTERP_RATIO: %i\n", INTERP_RATIO);
        print("NUMCORES: %i\n", NUMCORES);
        print("MIN_VOXEL: %i\n", MIN_VOXEL);
        fpu_check();
        #endif

        //check if X_Dim >= MIN_VOXEL
        if (n_X < MIN_VOXEL)
        {
            print("Error: X-Dim has to be at least %d (X is %u, Y is %u, Z %u). Use Y-dim for such small sizes.\n", MIN_VOXEL, n_X, n_Y, n_Z);
            return results;
        }

        ////benchmark of performance, selecting number of threads
        if (nCores_bench == -1) as2v_benchLocal();

        //select if use or not use multithreading
        #ifdef addsig2vol_debug
        print("selectedCore perf: %f, %f\n",((double)n_IMAGE/((double)throughput[nCores_bench-1] * 1000000))+((double)latency[nCores_bench-1]/1000000000),(((double)n_IMAGE/((double)throughput[0]*1000000))+((double)latency[0]/1000000000)));
        print("n_IMAGE: %i, throughput: %f, latency: %f, %f, %f\n",n_IMAGE, (double)throughput[nCores_bench-1],(double)latency[nCores_bench-1],(double)throughput[0],(double)latency[0]);
        #endif

        if ( ( ((double)n_IMAGE/((double)throughput[nCores_bench-1] *1000000))+((double)latency[nCores_bench-1]/1000000000)) <= (((double)n_IMAGE/((double)throughput[0] *1000000))+((double)latency[0]/1000000000)))
        {
            nCores = nCores_bench;
        }
        else
        {
            #ifdef addsig2vol_debug
            print("Overhead to big, switch to single thread.\n");
            #endif
            nCores = 1;
        }

        #ifdef addsig2vol_debug
        print("selectedNumCores: %i\n", nCores);
        print("savedNumCORE: %i\n", nCores_bench);
        print("perf_MT: %e\n", ( (throughput[nCores_bench] * n_IMAGE)+latency[nCores_bench]));
        print("perf_single: %e\n", ((throughput[1]*n_IMAGE)+latency[1]));
        #endif



        //soundmap version ?
        if (((speedz->x == n_X) & (speedz->y == n_Y) ) & ((speedz->z == n_Z) | (speedz->z == 1)))
        {
            print("Info: Soundmap version\n");
            addsig2vol_mode = 2;
        }
        else
        {
            //not soundmapversion; blocked version?
            if ( (rec_posz->x != 3) |  (send_posz->x != 3) ) //check first dimension
            {
                print("send_pos:\t"); caMiniprint(send_posz);
                print("rec_pos:\t"); caMiniprint(rec_posz);
                print("ascan_block: %d, blocksize rec_vec: %d, blocksize send_vec: %d",n_AScan_block,n_rec_vec_block,n_send_vec_block);

                print("Error: 3-d vectors needed for emitter & receiver positions or transposed blocked pos (1x3 instead of 3x1), break\n");
                return results;
            }

            if (!( (((n_AScan_block == n_rec_vec_block) & (n_AScan_block == n_send_vec_block)) | ((1 == n_rec_vec_block) & (n_AScan_block == n_send_vec_block)) | ((n_AScan_block == n_rec_vec_block) & (1 == n_send_vec_block))) & ((n_AScan_block == n_speed_vec_block) | (1 == n_speed_vec_block)) ))
            {
                print("Error: Blocked sizes parameter mismatch. Size(AScan,2) has to be size(rec_vec,2) and/or size(send_vec,2), n_rec_vec_block:%i n_send_vec_block:%i n_speed_vec_block:%i break\n",n_rec_vec_block, n_send_vec_block, n_speed_vec_block);
                return results;
            }
            if (n_AScan_block == 1)
            {
                //if here blocked mode successful initalized!
                addsig2vol_mode = 1;
            }
            else{
                //normal version
                addsig2vol_mode = 0;
            }
        }

        // malloc space for results
        if(outputImage){
            out_real = outputImage->data;
            //cAout_real = outputImage;
        }else{
            out_real = malloc(sizeof(double)*n_IMAGE);
            cAout_real = caNewArrayDoubleFromData(out_real, n_X, n_Y, n_Z);
        }

        if(buffer){
            buffer_real = buffer->data;
            //cAbuffer_real = buffer;
        }else{
            buffer_real = malloc(sizeof(double)*n_AScan*INTERP_RATIO);
            cAbuffer_real = caNewArrayDoubleFromData(buffer_real, n_AScan*INTERP_RATIO, 1, 1);
        }

        if (AScan_complexz) {
            //we know that both AScan_complexz and IMAGE_SUM_complexz exist
            // TODO same as in REAL case
            // out_complex = malloc(sizeof(double)*n_IMAGE);
            // buffer_complex = malloc(sizeof(double)*n_AScan*INTERP_RATIO);
            // cAout_complex = caNewArrayDoubleFromData(out_complex, n_X, n_Y, n_Z);
            // cAbuffer_complex = caNewArrayDoubleFromData(buffer_complex, n_AScan*INTERP_RATIO, 1, 1);
        }

        #ifdef addsig2vol_debug
        print("Set addsig2vol_mode to %i\n", addsig2vol_mode);
        print("Elements in out_real: %i\n", n_IMAGE);
        print("Elements in out_complex: %i\n", n_IMAGE);
        print("Elements in buffer_real: %i\n", n_AScan*INTERP_RATIO);
        print("Elements in buffer_complex: %i\n", n_AScan*INTERP_RATIO);
        #endif

        #ifdef SAVEDATA
        mkdir("data", 0777);
        saveDoubleArrayUnstruct(AScan_realz_ptr, n_AScan, n_AScan_block, 1, "data/AScan");
        saveFloatArrayUnstruct(pix_vectz_ptr, 3, 1, 1, "data/pix");
        saveFloatArrayUnstruct(rec_vec_ptr, 3, n_AScan_block, 1, "data/rec");
        saveFloatArrayUnstruct(send_vec_ptr, 3, n_AScan_block, 1, "data/send");
        saveFloatArrayUnstruct(speedz_vec_ptr, 1, 1, 1, "data/speed");
        saveDoubleArrayUnstruct(IMAGE_SUM_realz_ptr, n_X, n_Y, n_Z, "data/IMAGE_SUM");
        #endif

        ////first Ascan
        // combined REAL & COMPLEX VERSION
        as2v_MT(out_real, AScan_realz_ptr, n_AScan, buffer_real,
            pix_vectz_ptr, n_X, rec_vec_ptr, send_vec_ptr,
            speedz_vec_ptr, resz, timeintz, AScan_complexz_ptr, buffer_complex, out_complex, n_Y, n_Z, IMAGE_SUM_realz_ptr, IMAGE_SUM_complexz_ptr);


        //loop over ascans > 1
        for (i = 2; i <= n_AScan_block; i++) {
            #ifdef addsig2vol_debug
            print("as2v_MT call %i:\n", i);
            #endif

            //check for complex ascan only increase if available because NULL-Pointer +something -> not anymore a nullpointer!
            if (AScan_complexz != NULL)	AScan_complexz = AScan_complexz + (n_AScan * (i - 1)); //set to next value
            if (1<n_rec_vec_block)  rec_vec_ptr  = rec_posz->data  + (3 * (i - 1)); else rec_vec_ptr = rec_posz->data;
            if (1<n_send_vec_block) send_vec_ptr = send_posz->data + (3 * (i - 1)); else send_vec_ptr = send_posz->data;
            if (1<n_speed_vec_block) speedz_vec_ptr = speedz->data + (1  * (i - 1)); else speedz_vec_ptr = speedz->data;


            // NOTE: pass out_real/out_complex as IMAGE_SUM to sum up the single scans
            as2v_MT(out_real, AScan_realz_ptr + (n_AScan * (i - 1)), n_AScan, buffer_real,
            pix_vectz_ptr, n_X, rec_vec_ptr, send_vec_ptr,
            speedz_vec_ptr, resz, timeintz, AScan_complexz_ptr, buffer_complex, out_complex, n_Y, n_Z, out_real, out_complex);

            //             mxSetPr(ascan_output_buffer,mxGetPr(AScan)+(n_AScan*(i-1)));
            //             mxSetPr(recpos_output_buffer,(char*)mxGetPr(recpos_org)+(3*sizeof(float)*(i-1)));
            //
            //             mexCallMATLAB(0, NULL, 0, NULL, "figure");
            //             mexCallMATLAB(0, NULL, 1, &ascan_output_buffer, "plot");
            //             mexCallMATLAB(0, NULL, 1, &recpos_output_buffer, "disp");

        }
        //
        if (segmentedAxis == ZAXIS){
            printf("ZAXIS\n");
        }
        if (segmentedAxis == YAXIS){
            printf("YAXIS\n");
        }
        if (segmentedAxis == XAXIS){
            printf("XAXIS\n");
        }
        // printf("img size x,y,z: %i %i %i\n", imgX, imgY, imgZ);
        // printf("pos x,y,z: %i %i %i\n", posX, posY, posZ);
        // printf("halfstep x,y,z: %i %i %i\n", halfStepX, halfStepY, halfStepZ);

        // store resulting image
        results.out_real = &cAout_real;
        results.buffer_real = &cAbuffer_real;
        results.buffer_complex = &cAbuffer_complex;
        results.out_complex = &cAout_complex;
    }
    //Daten fehlen
    return results;
}

void printIntro(void){
    print("\naddSig2Vol_2 SSE1 Assembler Optimized 64bit LINUX&Windows v3.1 (Multiple Rec-AScan Vers.)\n\n Calculate the ellip. backprojection.\nUses SSE. Features: Win&Linux, 32&64bit version, SSE1-Assembler&C-Implementation, Multithreaded by PosixThreads (under windows pthreadsVC2.dll might be needed)\n\t %s. M.Zapf KIT-IPE\n\n",__DATE__);
    // TODO print som flags for debugging
    #ifdef addsig2vol_debug
        print("addsig2vol_debug: 1\n");
        #ifdef C_CODE
            print("C_CODE: 1, use c implementation\n");
        #else
            print("C_CODE: 0, use asm implementation\n");
        #endif

    #endif
}
