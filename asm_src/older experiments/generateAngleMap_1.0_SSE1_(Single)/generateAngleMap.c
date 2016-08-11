  #include "mex.h"
		#define out	  plhs[0]

		#define Map	  prhs[0]
		#define rec_pos   prhs[1]
		#define rec_norm  prhs[2]
		#define send_pos  prhs[3]
		#define send_norm prhs[4]
		#define pix_vect  prhs[5]
		#define IMAGE_XYZ prhs[6]
		#define res	  prhs[7]

		void gam(double* a, double* b, int c, double* d, double* e,
		double* g, double* h, double* i, int o, int p, int q, double *r);

		void mexFunction(int nlhs, mxArray* plhs[],
				 int nrhs, const mxArray* prhs[])
		{
		    int n_Map;
		    int n_X;
		    int n_Y;
		    int n_Z;
		    int n_IMAGE;
		    int* IMAGE_ptr;
		    int setImageDim[2];

		    double* pr;

		    if (nlhs > 2) mexErrMsgTxt("Too many output arguments.");
		    switch(nrhs)
		    {
		     default:
			mexErrMsgTxt("Incorrect number of arguments, sorry.");
			mexPrintf("\ngenerateAngleMap (SSE1 Assembler Optimized)\n\n#define out       plhs[0] (SINGLE[1:END]) \n#define Map       prhs[0] (SINGLE[1:end])\n#define rec_pos   prhs[1] (SINGLE[1:3])\n#define rec_norm  prhs[2] (SINGLE[1:3])\n#define send_pos  prhs[3] (SINGLE[1:3])\n#define send_norm prhs[4] (SINGLE[1:3])\n#define pix_vect  prhs[5] (SINGLE[1:3])\n#define IMAGE_XYZ prhs[6] (UINT32[1:3])\n#define res       prhs[7] (SINGLE)\n\n\t© 2006.04.08. M.Zapf FZK-IPE\n\n");
			break;
		     case 0:
			mexPrintf("\ngenerateAngleMap (SSE1 Assembler Optimized)\n\n#define out       plhs[0] (SINGLE[1:END]) \n#define Map       prhs[0] (SINGLE[1:end])\n#define rec_pos   prhs[1] (SINGLE[1:3])\n#define rec_norm  prhs[2] (SINGLE[1:3])\n#define send_pos  prhs[3] (SINGLE[1:3])\n#define send_norm prhs[4] (SINGLE[1:3])\n#define pix_vect  prhs[5] (SINGLE[1:3])\n#define IMAGE_XYZ prhs[6] (UINT32[1:3])\n#define res       prhs[7] (SINGLE)\n\n\t© 2006.04.08. M.Zapf FZK-IPE\n\n");
			break;
		     case 8:

			n_Map		= mxGetNumberOfElements(Map);    //gesamtanzahl elemente
			IMAGE_ptr	= mxGetPr(IMAGE_XYZ);
			n_X		= *IMAGE_ptr;
			n_Y		= *(IMAGE_ptr+1);
			n_Z		= *(IMAGE_ptr+2);
			n_IMAGE 	= n_X*n_Y*n_Z;
			setImageDim[0]	= n_IMAGE;
			setImageDim[1]	= 1;        //z.b: 400000x1

			//if (n_AScan<(2*wid+2)) mexErrMsgTxt("1. Array not 2 times +2 greater value 3 ");
			//mexPrintf("n_Z:  %i\n\n", n_Z);
			//mexPrintf("n_Y :  %i\n\n", n_Y);
			//mexPrintf("n_X :  %i\n\n", n_X);
			//mexPrintf("n_IMAGE :  %i\n\n", n_IMAGE);

			out =  mxCreateNumericArray(0,0, mxSINGLE_CLASS, mxREAL);
			mxSetDimensions(out,setImageDim,2); //bsp. 3000x1  -> (3000,1) ,2
			pr = mxMalloc(n_IMAGE*sizeof(int));
			mxSetPr(out,pr);
			//mexPrintf("ptr_bild :  %i\n\n",pr);

			//// generate angle Map
			gam(mxGetPr(out),mxGetPr(Map),n_Map,mxGetPr(rec_pos),mxGetPr(rec_norm),mxGetPr(send_pos),mxGetPr(send_norm),mxGetPr(pix_vect),n_X,n_Y,n_Z,mxGetPr(res));
		     }

		return;
		}

