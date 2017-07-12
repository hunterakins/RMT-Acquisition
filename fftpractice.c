/* 
Hunter's Note: copied this script from paulbourke.net/miscellaneous/dft, 

This computes an in-place complex-to-complex FFT 
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform 
*/
#include<stdio.h>
#include<stdlib.h>
#include<math.h>


double sin(double x);
double sqrt(double x);
void makesample(double *sp, int bufsize, float freq, float spacing);
void FFT(short int dir,long m,double *x,double *y);

void main(int argc, char * argv[]) {
	int i = 0;
	int bufsize = 16384;
	float freq = 100; /* 100 hertz sine wave */
	float spacing =  .001; /* 1kHz sample rate has a spacing of .001 seconds */
	double *dp = (double *)malloc(bufsize*sizeof(double));
	double *idp = (double *)malloc(bufsize*sizeof(double));
	short int dir = 1;
	long m = 14;	

	/* generate a buffer of points for my FFT test */
	makesample(dp, bufsize, freq, spacing);
	/* generate buffer of zeros for imaginary data */
	i = 0;
	makesample(idp, bufsize, 0.0, 0.0);
	printf("%u", dir);
	FFT(dir,m , dp, idp); 
	for (i = 0; i < bufsize/2; i ++) {
		printf("%f\n", dp[i]);
	} 	
}

void makesample(double *sp, int bufsize, float freq, float spacing) {
	double timevalue;
	int i = 0;
	double arg;

	for (i = 0; i < bufsize; i += 1) {
		timevalue = i * spacing;
		*sp = sin(i * spacing * freq * 2 * M_PI);
		sp++;
	}
}

void FFT(short int dir,long m,double *x,double *y) {
   long n,i,i1,j,k,i2,l,l1,l2;
   double c1,c2,tx,ty,t1,t2,u1,u2,z;
   /* Calculate the number of points */
   n = 1;
   for (i=0;i<m;i++) { 
      n *= 2;
	}
   /* Do the bit reversal */
   i2 = n >> 1;
   j = 0;
   for (i=0;i<n-1;i++) {
      if (i < j) {
         tx = x[i];
         ty = y[i];
         x[i] = x[j];
         y[i] = y[j];
         x[j] = tx;
         y[j] = ty;
      }
      k = i2;
      while (k <= j) {
         j -= k;
         k >>= 1;
      }
      j += k;
   }

   /* Compute the FFT */
   c1 = -1.0; 
   c2 = 0.0;
   l2 = 1;
   for (l=0;l<m;l++) {
      l1 = l2;
      l2 <<= 1;
      u1 = 1.0; 
      u2 = 0.0;
      for (j=0;j<l1;j++) {
         for (i=j;i<n;i+=l2) {
            i1 = i + l1;
            t1 = u1 * x[i1] - u2 * y[i1];
            t2 = u1 * y[i1] + u2 * x[i1];
            x[i1] = x[i] - t1; 
            y[i1] = y[i] - t2;
            x[i] += t1;
            y[i] += t2;
         }
         z =  u1 * c1 - u2 * c2;
         u2 = u1 * c2 + u2 * c1;
         u1 = z;
      }
      c2 = sqrt((1.0 - c1) / 2.0);
      if (dir == 1) 
         c2 = -c2;
      c1 = sqrt((1.0 + c1) / 2.0);
   }

   /* Scaling for forward transform */
   if (dir == 1) {
      for (i=0;i<n;i++) {
         x[i] /= n;
         y[i] /= n;
      }
   }
}
