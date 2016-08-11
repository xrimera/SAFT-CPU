
		#include "mex.h"
		#define out	plhs[0]
		#define out2	plhs[1]
		#define AScan	prhs[0]
		#define Index	prhs[1]
		#define rec_pos prhs[2]
		#define dist_ep prhs[3]
		#define speed	prhs[4]
		#define res	prhs[5]
		#define timeint prhs[6]

	       //; void xsum(double* ,double* ,double* ,double* ,int ,int ,int );
	       //; void double2int32(int* , double* ,int , char );
	       // void as2v(double* bild, double* _AScan, int _n_AScan,double* _buffer, double* _index_vol, int _n_index, double* _rec_pos, double* _em2pix, double* _speed, double* _resolution,double* _timeintervall)
		void as2v(double* a, double* b, int c, double* d, double* e,
		int f, double* g, double* h, double* i, double* j, double* k) ;
		void as2v_complex(double* a, double* b, int c, double* d, double* e,
		int f, double* g, double* h, double* i, double* j, double* k, double* l,
		double* m, double*n) ;

		void mexFunction(int nlhs, mxArray* plhs[],
				 int nrhs, const mxArray* prhs[])
		{
		    //int width_test;
		    int n_AScan;
		    int n_Index;
		    double* pr;
		    double* pi;
		    mxArray* buffer;

		    if (nlhs > 2) mexErrMsgTxt("Too many output arguments.");
		    switch(nrhs)
		    {
		     default:
			mexErrMsgTxt("Incorrect number of arguments.");
		     case 0:
			mexPrintf("\naddSig2Vol \n\n\t© 2006.01.27. M.Zapf FZK-IPE\n\n");
			break;
		     case 7:

			n_AScan = mxGetNumberOfElements(AScan);    //gesamtanzahl elemente
			n_Index = mxGetNumberOfElements(dist_ep);
			//if (n_AScan<(2*wid+2)) mexErrMsgTxt("1. Array not 2 times +2 greater value 3 ");

			//mexPrintf("width:  %i\n", wid);
			//mexPrintf("n_AScan:  %i\n", n_AScan);

			//anlegen puffer fuer xsum


			if (mxIsComplex(AScan))
			   {
			   buffer = mxCreateDoubleMatrix(0,0,mxCOMPLEX); //Sum buffer laenge ascan
			   mxSetDimensions(buffer,mxGetDimensions(AScan),mxGetNumberOfDimensions(AScan)); //bsp. 3000x1  -> (3000,1) ,2
			   pr=mxMalloc(n_AScan*sizeof(double));
			   pi=mxMalloc(n_AScan*sizeof(double));
			   mxSetPr(buffer,pr);
			   mxSetPi(buffer,pi);

			   out	   = mxCreateDoubleMatrix(0,0,mxCOMPLEX);      //out     = mxCreateDoubleMatrix(n_Index,1,mxCOMPLEX);
			   mxSetDimensions(out,mxGetDimensions(dist_ep),mxGetNumberOfDimensions(dist_ep)); //bsp. 3000x1  -> (3000,1) ,2
			   pr=mxMalloc(n_Index*sizeof(double));
			   pi=mxMalloc(n_Index*sizeof(double));
			   mxSetPr(out,pr);
			   mxSetPi(out,pi);
			   }
			 else  //real
			   {
			   buffer = mxCreateDoubleMatrix(0,0,mxREAL); //Sum buffer laenge ascan
			   mxSetDimensions(buffer,mxGetDimensions(AScan),mxGetNumberOfDimensions(AScan)); //bsp. 3000x1  -> (3000,1) ,2
			   pr=mxMalloc(n_AScan*sizeof(double));
			   mxSetPr(buffer,pr);

			   out	   = mxCreateDoubleMatrix(0,0,mxREAL);      //out     = mxCreateDoubleMatrix(n_Index,1,mxCOMPLEX);
			   mxSetDimensions(out,mxGetDimensions(dist_ep),mxGetNumberOfDimensions(dist_ep)); //bsp. 3000x1  -> (3000,1) ,2
			   pr=mxMalloc(n_Index*sizeof(double));
			   mxSetPr(out,pr);
			   }

			   // combined REAL & COMPLEX VERSION
			   as2v_complex(mxGetPr(out),mxGetPr(AScan),n_AScan,mxGetPr(buffer),mxGetPr(Index),n_Index,mxGetPr(rec_pos),mxGetPr(dist_ep),mxGetPr(speed),mxGetPr(res),mxGetPr(timeint),mxGetPi(AScan),mxGetPi(buffer),mxGetPi(out));


			out2 = buffer;
			//mxDestroyArray(buffer);
			}

		return;
		}

