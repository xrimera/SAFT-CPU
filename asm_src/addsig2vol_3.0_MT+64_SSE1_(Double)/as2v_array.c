#include "as2v_array.h"

#include <string.h>
#include <stdarg.h>

static void (*print)(char*, ...) = &printf;

#define LINESIZE 1024

// Use customized printing
void caSetPrintCallback(void (*callback)(char*, ...)) {
    if (callback) print = callback;
}


// Empty objects
#define cArrayDoubleEmpty (cArrayDouble) {CARRAY_DOUBLE, 0, 0, 0, 0, NULL}
#define cArrayFloatEmpty (cArrayFloat) {CARRAY_FLOAT, 0, 0, 0, 0, NULL}

//Create new arrays
cArrayDouble caNewArrayDouble(){ return cArrayDoubleEmpty; }
cArrayFloat caNewArrayFloat(){ return cArrayFloatEmpty; }

cArrayDouble caNewArrayDoubleFromData(double* data, unsigned int x, unsigned int y, unsigned int z){
    cArrayDouble array = caNewArrayDouble();
    caUpdateData((cArray*)&array, data, x, y, z);
    return array;
}
cArrayFloat caNewArrayFloatFromData(float* data, unsigned int x, unsigned int y, unsigned int z){
    cArrayFloat array = caNewArrayFloat();
    caUpdateData((cArray*)&array, data, x, y, z);
    return array;
}

cArrayDouble caNewArrayDoubleFromFile(char* filename){
    cArrayDouble array = caNewArrayDouble();
    caLoad((cArray*)&array, filename);
    return array;
}
cArrayFloat caNewArrayFloatFromFile(char* filename){
    cArrayFloat array = caNewArrayFloat();
    caLoad((cArray*)&array, filename);
    return array;
}


//Operate on existing arrays
void caUpdateData(cArray* array, void* data, unsigned int x, unsigned int y, unsigned int z){
    if(data) {array->x = x; array->y = y; array->z = z;}
    else {array->x = 0; array->y = 0; array->z = 0;}
    array->len = x*y*z;
    switch (array->type) {
        case CARRAY_DOUBLE: ;
            cArrayDouble* cad = (cArrayDouble*) array;
            cad->data = (double*) data;
            break;
        case CARRAY_FLOAT: ;
            cArrayFloat* caf = (cArrayFloat*) array;
            caf->data = (float*) data;
            break;
        default: ;
            break;
    }
}

void caSave(cArray* array, char* filename) {
    if(array){
        FILE *filePtr = 0;
        filePtr = fopen(filename,"w");
        if( filePtr == NULL )
        {
            print("Error while opening file: %c.\n", filename);
            exit(EXIT_FAILURE);
        }
        fprintf(filePtr, "elements: %i\n", array->len);
        fprintf(filePtr, "x:%i\n", array->x);
        fprintf(filePtr, "y:%i\n", array->y);
        fprintf(filePtr, "z:%i\n", array->z);
        switch (array->type) {
            case CARRAY_DOUBLE: ;
                cArrayDouble* cad = (cArrayDouble*) array;
                if(cad->data) for(unsigned int i=0; i<array->len; i++) fprintf(filePtr, "%.*le\n", OP_DBL_Digs-1, cad->data[i]);
                break;
            case CARRAY_FLOAT: ;
                cArrayFloat* caf = (cArrayFloat*) array;
                if(caf->data) for(unsigned int i=0; i<array->len; i++) fprintf(filePtr, "%.*e\n", OP_FLT_Digs-1, caf->data[i]);
                break;
            default: ;
                break;
        }
        fclose(filePtr);
    }
}

void caMiniprint(cArray* array){
    if (array){
        switch (array->type) {
            case CARRAY_DOUBLE: ;
                cArrayDouble* cad = (cArrayDouble*) array;
                print("cArrayDouble \t| data at %p \t| elements=%i \t| x=%i y=%i z=%i\n", cad->data, array->len, array->x, array->y, array->z);
                break;
            case CARRAY_FLOAT: ;
                cArrayFloat* caf = (cArrayFloat*) array;
                print("cArrayFloat \t| data at %p \t| elements=%i \t| x=%i y=%i z=%i\n", caf->data, array->len, array->x, array->y, array->z);
                break;
            default: ;
                break;
        }
    }
}

void caLoad(cArray* array, char* filename){
    if(array){
        unsigned int linesize = 512;
        FILE *filePtr = 0;
        filePtr = fopen(filename, "r");
        if( filePtr == NULL )
        {
            print("Error while opening file: %c.\n", filename);
            exit(EXIT_FAILURE);
        }
        char line[linesize];
        fgets(line, linesize, filePtr); sscanf(line, " elements: %i", &(array->len));
        fgets(line, linesize, filePtr); sscanf(line, " x: %i", &(array->x));
        fgets(line, linesize, filePtr); sscanf(line, " y: %i", &(array->y));
        fgets(line, linesize, filePtr); sscanf(line, " z: %i", &(array->z));
        switch (array->type) {
            case CARRAY_DOUBLE: ;
                cArrayDouble* cad = (cArrayDouble*) array;
                free(cad->data);
                if (cad->len == 0){ cad->data = NULL; return; }
                cad->data = malloc(cad->len*sizeof(double));
                unsigned int i = 0;
                char* lined[LINESIZE];
                while((i < cad->len ) & !(feof(filePtr))) {
                    fgets(lined, LINESIZE, filePtr);
                    //see https://stackoverflow.com/questions/4264127/correct-format-specifier-for-double-in-printf
                    sscanf(lined, " %le", &(cad->data[i]));
                    i++;
                }
                break;
            case CARRAY_FLOAT: ;
                cArrayFloat* caf = (cArrayFloat*) array;
                free(caf->data);
                if (caf->len == 0){ caf->data = NULL; return; }
                caf->data = malloc(caf->len*sizeof(float));
                unsigned int j = 0;
                char* linef[LINESIZE];
                while((j < caf->len ) & !(feof(filePtr))) {
                    fgets(linef, LINESIZE, filePtr);
                    //see https://stackoverflow.com/questions/4264127/correct-format-specifier-for-double-in-printf
                    sscanf(linef, " %e", &(caf->data[j]));
                    j++;
                }
                break;
            default: ;
                break;
        }
        fclose(filePtr);
    }
}


char* concatPath(char* filename){
    // if(strlen(filedir)>0)
    // {
    //     char path[2000];
    //     strcpy(path, filedir);
    //     strcpy(path+strlen(filedir), "/");
    //     strcat(path+strlen(filedir)+1, filename);
    //     printf(path);
    //
    //     return path;
    // } else
    // {
    //     return filename;
    // }

}



#ifdef BUILD_WITH_MEX

//data gets copied, so you can free C array without losing mxArray
// TODO change flag system
cArrayDouble caNewDoubleArrayFromMxarray(mxArray* mxa, mxComplexity flag){
    cArrayDouble array = caNewArrayDouble();
    cArray2Mxarray((cArray*)&array, mxa);
    return array;
}

cArrayFloat caNewFloatArrayFromMxarray(mxArray* mxa, mxComplexity flag){
    cArrayFloat array = caNewArrayFloat();
    cArray2Mxarray((cArray*)&array, mxa, flag);
    return array;
}


void cArray2Mxarray(cArray* ca, mxArray* mxa, mxComplexity flag){
    //as2v_doubleArray array;
    if (mxa){
        ca->len = (unsigned int) mxGetNumberOfElements(mxa);
        mwSize* dim = (mwSize*) mxGetDimensions(mxa);
        ca->x = dim[0];
        ca->y = dim[1];
        //% workaround for matlab behaviour for size(IMAGE(1 x1x1))->reduced to 1 x1 (some kind of squeeze)
        if (ca->x * ca->y < ca->len){ca->z = dim[2];}
        else {ca->z = 1;}
        switch (ca->type) {
            case CARRAY_DOUBLE: ;
                cArrayDouble* cad = (cArrayDouble*) ca;
                double* datad = malloc(cad->len*sizeof(double));
                if (flag == mxCOMPLEX) memcpy(datad, (double*) mxGetPi(mxa), cad->len*sizeof(double));
                else memcpy(datad, (double*) mxGetPr(mxa), cad->len*sizeof(double));
                cad->data = datad;
                break;
            case CARRAY_FLOAT: ;
                cArrayFloat* caf = (cArrayFloat*) ca;
                float* dataf = malloc(caf->len*sizeof(float));
                if (flag == mxCOMPLEX) memcpy(dataf, (float*) mxGetPi(mxa), caf->len*sizeof(float));
                else memcpy(dataf, (float*) mxGetPr(mxa), caf->len*sizeof(float));
                caf->data = dataf;
                break;
            default: ;
                break;
        }
    }
}

// TODO
//data gets copied, so you can free C array without losing mxArray
mxArray* caNewMxarrayFromCArray(cArray* real, cArray* complex){
    mxArray* mxarray = NULL;
    if(real){
        mwSize numberOfDimensions = 3;
        mwSize setImageDim[3] = {real->x, real->y, real->z};
        if(real->z == 1) numberOfDimensions = 2; //squeeze z away
        switch (real->type) {
            case CARRAY_DOUBLE: ;
                mxarray = mxCreateNumericArray(numberOfDimensions, setImageDim, mxDOUBLE_CLASS, mxCOMPLEX);
                if(real) memcpy(mxGetPr(mxarray), (double*)real->data, real->len*sizeof(double));
                if(complex) memcpy(mxGetPi(mxarray), (double*)complex->data, complex->len*sizeof(double));
                break;
            case CARRAY_FLOAT: ;
                mxarray = mxCreateNumericArray(numberOfDimensions, setImageDim, mxSINGLE_CLASS, mxCOMPLEX);
                if(real) memcpy(mxGetPr(mxarray), (float*)real->data, real->len*sizeof(float));
                if(complex) memcpy(mxGetPi(mxarray), (float*)complex->data, complex->len*sizeof(float));
                break;
            default: ;
                break;
        }
    }
    return mxarray;
}



#endif //BUILD_WITH_MEX
