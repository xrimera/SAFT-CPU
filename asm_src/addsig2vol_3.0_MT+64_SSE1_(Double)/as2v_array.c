#include "as2v_array.h"

as2v_doubleArray as2v_boxDoubleArray(double* data, unsigned int x, unsigned int y, unsigned int z){
    as2v_doubleArray array;
    array.data = data;
    if(data) {array.x = x; array.y = y; array.z = z;}
    else {array.x = 0; array.y = 0; array.z = 0;}
    array.len = x*y*z;
    return array;
}

as2v_floatArray as2v_boxFloatArray(float* data, unsigned int x, unsigned int y, unsigned int z){
    as2v_floatArray array;
    array.data = data;
    if(data) {array.x = x; array.y = y; array.z = z;}
    else {array.x = 0; array.y = 0; array.z = 0;}
    array.len = x*y*z;
    return array;
}

as2v_doubleArray as2v_mallocDoubleArray(unsigned int x, unsigned int y, unsigned int z){
    as2v_doubleArray array;
    array.x = x; array.y = y; array.z = z;
    array.len = x*y*z;
    array.data = malloc(array.len*sizeof(double));
    return array;
}

as2v_floatArray as2v_mallocFloatArray(unsigned int x, unsigned int y, unsigned int z){
    as2v_floatArray array;
    array.x = x; array.y = y; array.z = z;
    array.len = x*y*z;
    array.data = malloc(array.len*sizeof(float));
    return array;
}

void briefDoubleArray(as2v_doubleArray* array){
    if (array){
        PRINT("DoubleArray | Data at %p | #Elements=%i x=%i y=%i z=%i\n", array->data, array->len, array->x, array->y, array->z);
    }
}

void briefFloatArray(as2v_doubleArray* array){
    if (array){
        PRINT("FloatArray | Data at %p | #Elements=%i x=%i y=%i z=%i\n", array->data, array->len, array->x, array->y, array->z);
    }
}

void upstreamDoubleArray(as2v_doubleArray* array){
    if (upstreamDoubleArray_callback){
        upstreamDoubleArray_callback(array);
    }
}
void upstreamFloatArray(as2v_floatArray* array){
if (upstreamFloatArray_callback){
    upstreamFloatArray_callback(array);
}
}

void setUpstreamDoubleArray_callback(void (*callback)(as2v_doubleArray*))
{
    upstreamDoubleArray_callback = callback;
}
void setUpstreamFloatArray_callback(void (*callback)(as2v_floatArray*))
{
    upstreamFloatArray_callback = callback;
}

void saveDoubleArray(as2v_doubleArray* array, char* filename){
    if(array){
        FILE *filePtr = fopen(filename,"w");
        if( filePtr == NULL )
        {
            PRINT("Error while opening file: %c.\n", filename);
            exit(EXIT_FAILURE);
        }
        fprintf(filePtr, "elements: %i\n", array->len);
        fprintf(filePtr, "x:%i\n", array->x);
        fprintf(filePtr, "y:%i\n", array->y);
        fprintf(filePtr, "z:%i\n", array->z);
        for(unsigned int i=0; i<array->len; i++) fprintf(filePtr, "%.*e\n", OP_DBL_Digs-1, array->data[i]);
        fclose(filePtr);
    }
    return;
}

void saveFloatArray(as2v_floatArray* array, char* filename){
    if(array){
        FILE *filePtr = fopen(filename,"w");
        if( filePtr == NULL )
        {
            PRINT("Error while opening file: %c.\n", filename);
            exit(EXIT_FAILURE);
        }
        fprintf(filePtr, "elements: %i\n", array->len);
        fprintf(filePtr, "x:%i\n", array->x);
        fprintf(filePtr, "y:%i\n", array->y);
        fprintf(filePtr, "z:%i\n", array->z);
        //for(unsigned int i=0; i<array->len; i++) fprintf(filePtr, "%.*e\n", OP_DBL_Digs-1, array->data[i]);
        for(unsigned int i=0; i<array->len; i++) fprintf(filePtr, "%.*e\n", OP_FLT_Digs-1, array->data[i]);
        fclose(filePtr);
    }
    return;
}



void loadDoubleArray(as2v_doubleArray* targetArray, char* filename)
{
    if(targetArray){
        unsigned int linesize = 512;
        FILE *filePtr;
        filePtr = fopen(filename, "r");
        if( filePtr == NULL )
        {
            PRINT("Error while opening file: %c.\n", filename);
            exit(EXIT_FAILURE);
        }
        char line[linesize];
        unsigned int len, x, y, z;
        fgets(line, linesize, filePtr); sscanf(line, " elements: %i", &(targetArray->len));
        fgets(line, linesize, filePtr); sscanf(line, " x: %i", &(targetArray->x));
        fgets(line, linesize, filePtr); sscanf(line, " y: %i", &(targetArray->y));
        fgets(line, linesize, filePtr); sscanf(line, " z: %i", &(targetArray->z));
        free(targetArray->data);
        targetArray->data = malloc((targetArray->len)*sizeof(double));
        unsigned int i = 0;
        while((i < targetArray->len ) & !(feof(filePtr))) {
            fgets(line, linesize, filePtr);
            //see https://stackoverflow.com/questions/4264127/correct-format-specifier-for-double-in-printf
            sscanf(line, " %le", &(targetArray->data[i]));
            i++;
        } /*End while not EOF*/
        fclose(filePtr);
    }
}

void loadFloatArray(as2v_floatArray* targetArray, char* filename)
{
    if(targetArray){
        unsigned int linesize = 512;
    FILE *filePtr;
    filePtr = fopen(filename, "r");
    if( filePtr == NULL )
    {
        PRINT("Error while opening file: %c.\n", filename);
        exit(EXIT_FAILURE);
    }
    char line[linesize];
    unsigned int len, x, y, z;
    fgets(line, linesize, filePtr); sscanf(line, " elements: %i", &(targetArray->len));
    fgets(line, linesize, filePtr); sscanf(line, " x: %i", &(targetArray->x));
    fgets(line, linesize, filePtr); sscanf(line, " y: %i", &(targetArray->y));
    fgets(line, linesize, filePtr); sscanf(line, " z: %i", &(targetArray->z));
    free(targetArray->data);
    targetArray->data = malloc((targetArray->len)*sizeof(float));
    unsigned int i = 0;
    while((i < targetArray->len ) & !(feof(filePtr))) {
        fgets(line, linesize, filePtr);
        sscanf(line, " %e", &(targetArray->data[i]));
        i++;
    } /*End while not EOF*/
    fclose(filePtr);
}
}


#ifdef BUILD_WITH_MEX

//careful: data doesnt get copied, so if mxa is mxfree'd, doublArray.data will be a dangling pointer
as2v_doubleArray as2v_mxArray2doubleArray(mxArray* mxa, mxComplexity flag){
    as2v_doubleArray array;
    if (mxa){
        if (flag == mxCOMPLEX) {array.data = (double*) mxGetPi(mxa);}
        else {array.data = (double*) mxGetPr(mxa);}
        array.len = (unsigned int) mxGetNumberOfElements(mxa);
        mwSize* dim = (mwSize*) mxGetDimensions(mxa);
        array.x = dim[0];
        array.y = dim[1];
        //% workaround for matlab behaviour for size(IMAGE(1 x1x1))->reduced to 1 x1 (some kind of squeeze)
        if (array.x * array.y < array.len){array.z = dim[2];}
        else {array.z = 1;}
    } else {
        array.data = NULL;
        array.x = 0; array.y = 0; array.z = 0; array.len = 0;
    }
    return array;
}

as2v_floatArray as2v_mxArray2floatArray(mxArray* mxa, mxComplexity flag){
    as2v_floatArray array;
    if (mxa){
        if (flag == mxCOMPLEX)  {array.data = (float*) mxGetPi(mxa);}
        else                    {array.data = (float*) mxGetPr(mxa);}
        array.len = (unsigned int) mxGetNumberOfElements(mxa);
        mwSize* dim = (mwSize*) mxGetDimensions(mxa);
        array.x = dim[0];
        array.y = dim[1];
        //% workaround for matlab behaviour for size(IMAGE(1 x1x1))->reduced to 1 x1 (some kind of squeeze)
        if (array.x * array.y < array.len) {array.z = dim[2];}
        else {array.z = 1;}
    } else {
        array.data = NULL;
        array.x = 0; array.y = 0; array.z = 0; array.len = 0;
    }
    return array;
}

//data gets copied, so you can free C array without losing mxArray
mxArray* as2v_doubleArray2mxArray(as2v_doubleArray* array_real, as2v_doubleArray* array_complex){
    mxArray* mxarray = NULL;
    if(array_real){
        mwSize numberOfDimensions = 3;
        if(array_real->z == 1) numberOfDimensions = 2; //squeeze z away
        mwSize setImageDim[3]={array_real->x, array_real->y, array_real->z};
        mxarray = mxCreateNumericArray(numberOfDimensions, setImageDim, mxDOUBLE_CLASS, mxCOMPLEX); //Sum buffer laenge ascan //bsp. 3000x1  -> (3000,1) ,2
        // don't inject malloc array directly into mxArray, it won't be recognized by the MATLAB memory manager
        memcpy(mxGetPr(mxarray), array_real->data, sizeof(array_real->data));
        if(array_complex) memcpy(mxGetPi(mxarray), array_complex->data, sizeof(array_complex->data));
    }
    return mxarray;
}

mxArray* as2v_floatArray2mxArray(as2v_floatArray* array_real, as2v_floatArray* array_complex){
    mxArray* mxarray = NULL;
    if(array_real){
        mwSize numberOfDimensions = 3;
        if(array_real->z == 1) numberOfDimensions = 2; //squeeze z away
        mwSize setImageDim[3]={array_real->x, array_real->y, array_real->z};
        mxarray = mxCreateNumericArray(numberOfDimensions, setImageDim, mxDOUBLE_CLASS, mxCOMPLEX); //Sum buffer laenge ascan //bsp. 3000x1  -> (3000,1) ,2
        // don't inject malloc array directly into mxArray, it won't be recognized by the MATLAB memory manager
        memcpy(mxGetPr(mxarray), array_real->data, sizeof(array_real->data));
        if(array_complex) memcpy(mxGetPi(mxarray), array_complex->data, sizeof(array_complex->data));
    }
    return mxarray;
}

#endif //BUILD_WITH_MEX
