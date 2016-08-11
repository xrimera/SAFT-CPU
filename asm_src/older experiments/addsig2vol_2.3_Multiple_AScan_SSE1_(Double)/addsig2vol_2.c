#include "mex.h"
#define out       plhs[0]
#define out2      plhs[1]

#define AScan     prhs[0]
#define pix_vect  prhs[1]
#define rec_pos   prhs[2]
#define send_pos  prhs[3]
#define speed     prhs[4]
#define res       prhs[5]
#define timeint   prhs[6]
#define IMAGE_XYZ prhs[7]
#define IMAGE_SUM prhs[8]


#define interp_ratio 5    // resizes ascan from 3000x1 -> 15000x1 (FIXED SIZE lin. interp.)


void __as2v_complex(double*outz, double*AScanz, int n_AScanz, double*bufferz, float*pix_vectz,
		    int n_Xz, float*rec_posz, float*send_posz, float*speedz, float*resz,
		    float*timeintz, double*AScan_complexz,
		    double*buffer_complexz, double*out_complexz, int n_Yz, int n_Zz,
		    double *IMAGE_SUMz, double *IMAGE_SUM_complexz,
		    double qwordbufferz);

void mexFunction (int nlhs, mxArray*plhs[],
		  int nrhs, const mxArray*prhs[]) {



	//int width_test;
	unsigned int i;
	unsigned int n_AScan;
	unsigned int n_AScan_block;
	int n_X;
	int n_Y;
	int n_Z;
	int n_IMAGE;
	int*IMAGE_ptr;
	int*BUFFER_ptr;
	int setImageDim[2];
	int setBufferDim[2];
	double *double_ptr;
	double qwordbuffer;

	double*pr;
	double*pi;

	mxArray*buffer;

//     mxArray* ascan_output_buffer;
//     mxArray* recpos_output_buffer;
//     mxArray* recpos_org = rec_pos;
	if (nlhs > 2) mexErrMsgTxt("Too many output arguments.");
	switch (nrhs) {
	default:
		mexErrMsgTxt("Incorrect number of arguments.");
		break;
	case 0:
		mexPrintf(
			"\naddSig2Vol_2 SSE1 Assembler Optimized v2.4 (Multiple Rec-AScan Vers.)\n\n Calculate the ellip. backprojection.\nUses SSE.\n\n#define out       plhs[0] (Double(1:end))\n#define out2      plhs[1] DEBUG (Double(1:end))\n#define AScan     prhs[0] (Double(NxM))\n#define pix_vect  prhs[1] (Single(1:3))\n#define rec_pos   prhs[2] (Single(1:3xM))\n#define send_pos  prhs[3] (Single(1:3))\n#define speed     prhs[4] (Single)\n#define res       prhs[5] (Single)\n#define timeint   prhs[6] (Single)\n#define IMAGE_XYZ prhs[7] (UINT32(1:3))\n#define IMAGE_SUM prhs[8] (Double(1:end))\n\n\t© 2007.07.11. M.Zapf FZK-IPE\n\n");
		break;
	case 9:


		IMAGE_ptr       = mxGetPr(IMAGE_XYZ);
		n_X             = *IMAGE_ptr;
		n_Y             = *(IMAGE_ptr + 1);

		// mh: translate this matlab code
		/*
		   % workaround for matlab behaviour for size(IMAGE(1 x1x1))->reduced to 1 x1
		   if (mxGetNumberOfDimensions(IMAGE_XYZ) < 3)
		   	n_Z = 1;else n_Z              = *(IMAGE_ptr + 2);end
		 */
		//% workaround for matlab behaviour for size(IMAGE(1 x1x1))->reduced to 1 x1
		if (mxGetNumberOfDimensions(IMAGE_XYZ) < 3)
			n_Z = 1;
		else
			n_Z              = *(IMAGE_ptr + 2);
		 // mh: translate done

		n_IMAGE         = n_X * n_Y * n_Z;
		setImageDim[0]  = n_IMAGE;
		setImageDim[1]  = 1;        //z.b: 400000x1

		BUFFER_ptr      = mxGetDimensions(AScan);
		n_AScan         = (*BUFFER_ptr);        //gesamtanzahl elemente IN EINEM ASCAN!!!
		n_AScan_block   = *(BUFFER_ptr + 1);    //2 dim %number of parallel

		setBufferDim[0] = (*BUFFER_ptr) * interp_ratio;
		setBufferDim[1] = 1;   //z.b: 400000x1

		//if (n_AScan<(2*wid+2)) mexErrMsgTxt("1. Array not 2 times +2 greater value 3 ");
		//mexPrintf("n_AScan :  %i\n\n", n_AScan );
		//mexPrintf("n_AScan_block :  %i\n\n", n_AScan_block );
		//mexPrintf("n_Z:  %i\n\n", n_Z);
		//mexPrintf("n_Y :  %i\n\n", n_Y);
		//mexPrintf("n_X :  %i\n\n", n_X);
		//mexPrintf("n_IMAGE :  %i\n\n", n_IMAGE);


		//Check if complex
		double_ptr = mxGetPi(AScan);
//         ascan_output_buffer = mxCreateDoubleMatrix(3000,1,mxREAL);
//         recpos_output_buffer = mxCreateNumericMatrix(3,1,mxSINGLE_CLASS,mxREAL);
		//anlegen puffer fuer xsum
		if (double_ptr != NULL) {
			buffer = mxCreateDoubleMatrix(0, 0, mxCOMPLEX);                         //Sum buffer laenge ascan
			mxSetDimensions(buffer, setBufferDim, mxGetNumberOfDimensions(AScan));  //bsp. 3000x1  -> (3000,1) ,2
			pr = mxMalloc(interp_ratio * n_AScan * sizeof(double));
			pi = mxMalloc(interp_ratio * n_AScan * sizeof(double));
			mxSetPr(buffer, pr);
			mxSetPi(buffer, pi);

			out        = mxCreateDoubleMatrix(0, 0, mxCOMPLEX);             //out     = mxCreateDoubleMatrix(n_Index,1,mxCOMPLEX);
			mxSetDimensions(out, setImageDim, mxGetNumberOfDimensions(IMAGE_SUM));                           //bsp. 3000x1  -> (3000,1) ,2
			pr = mxMalloc(n_IMAGE * sizeof(double));
			pi = mxMalloc(n_IMAGE * sizeof(double));
			mxSetPr(out, pr);
			mxSetPi(out, pi);
		} else {                                                                        //real
			buffer = mxCreateDoubleMatrix(0, 0, mxREAL);                            //Sum buffer laenge ascan
			mxSetDimensions(buffer, setBufferDim, mxGetNumberOfDimensions(AScan));  //bsp. 3000x1  -> (3000,1) ,2
			pr = mxMalloc(interp_ratio * n_AScan * sizeof(double));
			mxSetPr(buffer, pr);
			mxSetPi(buffer, NULL);

			out        = mxCreateDoubleMatrix(0, 0, mxREAL);        //out     = mxCreateDoubleMatrix(n_Index,1,mxCOMPLEX);
			mxSetDimensions(out, setImageDim, mxGetNumberOfDimensions(IMAGE_SUM));                   //bsp. 3000x1  -> (3000,1) ,2
			pr = mxMalloc(n_IMAGE * sizeof(double));
			mxSetPr(out, pr);
			mxSetPi(out, NULL);
		}

		////first Ascan
		// combined REAL & COMPLEX VERSION

		//no sizeof(double) needed because compilers assumes already double as dattype for pointer!!!
		as2v_complex(mxGetPr(out), mxGetPr(AScan), n_AScan, mxGetPr(buffer),
			     mxGetPr(pix_vect), n_X, mxGetPr(rec_pos), mxGetPr(send_pos),
			     mxGetPr(speed),
			     mxGetPr(res), mxGetPr(timeint), mxGetPi(AScan), mxGetPi(
				     buffer), mxGetPi(
				     out), n_Y, n_Z, mxGetPr(IMAGE_SUM), mxGetPi(
				     IMAGE_SUM), qwordbuffer);
		//     as2v_complex(mxGetPr(out),mxGetPr(AScan),n_AScan,mxGetPr(buffer),mxGetPr(pix_vect),n_X,mxGetPr(rec_pos),mxGetPr(send_pos),mxGetPr(speed),mxGetPr(res),mxGetPr(timeint),mxGetPi(AScan),mxGetPi(buffer),mxGetPi(out),n_Y,n_Z,mxGetPr(IMAGE_SUM),mxGetPi(IMAGE_SUM),qwordbuffer);

//         mexCallMATLAB(0, NULL, 1, &recpos_org, "disp");

		//loop over ascans > 1
		for (i = 2; i <= n_AScan_block; i++) {
			//check for complex ascan only increase if available because NULL-Pointer +something -> not anymore a nullpointer!
			if (double_ptr != NULL)
				//set to next value
				double_ptr = mxGetPi(AScan) + (n_AScan * (i - 1));


// 			mexPrintf("delta_address :  %i\n\n", n_AScan*(i-1));
// 			mexPrintf("n_AScan_address :  %p\n\n", mxGetPr(AScan)+(n_AScan)*(i-1));
// 			mexPrintf("mxGetPr(out):  %p\n\n", mxGetPr(out));
// 			mexPrintf("mxGetPi(out):  %p\n\n", mxGetPi(out));
// 			mexPrintf("mxGetPr(Ascan):  %p\n\n", mxGetPr(AScan));
// 			mexPrintf("mxGetPi(Ascan):  %p\n\n", mxGetPi(AScan));
// 			mexPrintf("i:  %i\n\n", i);

			// combined REAL & COMPLEX VERSION
			//no sizeof(double) needed because compilers assumes already double as datatype for pointer!!!
			//matlab seems to do some nasty errors on ptr to float...adding 8bytes instead of 4!!!! workaround implemented
			as2v_complex(mxGetPr(out), mxGetPr(
					     AScan) + (n_AScan * (i - 1)), n_AScan, mxGetPr(
					     buffer), mxGetPr(
					     pix_vect), n_X, (char*)mxGetPr(
					     rec_pos) + (3 * sizeof(float) * (i - 1)),
				     mxGetPr(send_pos), mxGetPr(
					     speed), mxGetPr(res), mxGetPr(
					     timeint), double_ptr, mxGetPi(buffer),
				     mxGetPi(out), n_Y, n_Z, mxGetPr(out), mxGetPi(
					     out), qwordbuffer);
			//  test
			//as2v_complex(mxGetPr(out),mxGetPr(AScan)+(n_AScan*(i-1)),n_AScan,mxGetPr(buffer),mxGetPr(pix_vect),n_X,(char*)mxGetPr(rec_pos)+(3*sizeof(float)*(i-1)),mxGetPr(send_pos),mxGetPr(speed),mxGetPr(res),mxGetPr(timeint),double_ptr,NULL,NULL,n_Y,n_Z,mxGetPr(out),NULL,qwordbuffer);

//             mxSetPr(ascan_output_buffer,mxGetPr(AScan)+(n_AScan*(i-1)));
//             mxSetPr(recpos_output_buffer,(char*)mxGetPr(recpos_org)+(3*sizeof(float)*(i-1)));
//
//             mexCallMATLAB(0, NULL, 0, NULL, "figure");
//             mexCallMATLAB(0, NULL, 1, &ascan_output_buffer, "plot");
//             mexCallMATLAB(0, NULL, 1, &recpos_output_buffer, "disp");
		}

		//out2 = buffer;
		mxDestroyArray(buffer);
	}

	return;
}

