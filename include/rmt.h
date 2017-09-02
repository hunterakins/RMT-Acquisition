#include <stdio.h>
#include <stdlib.h>
#include <gsl/gls_fit.h>
#include <gsl/gls_complex.h>


void GenWindow(int bufsize, double *hp); /*generate hamming window vals */


void Hadamard(int bufsize, double *hp, double *dp); /* dot product the two arrays hp and dp */ 

void FFT(short int dir,long m,double *x,double *y); /* calculate the FFT */

int gsl_fit_linear(const double * x, const size_t xstride, const double * y, const size_t ystride, size_t n,

double * c0, double * c1, double * cov00, double * cov01, double * cov11, double

* sumsq); /* lin fit the data */

void MakeImagBuffer(int bufsize, double *ip); /* make buffer for imaginary component data since time series is totally real */

void MakeDomain(int bufsize, double *ip); // creates an empty buffer against which to compute the linear fit 

void FirstOrderCorrect(int bufsize, double c0, double c1, double *dp); // takes in linear fit coefficients c0, c1 and subtracts away the linear fit to dp

int GenWave(rp_channel_t channel, rp_waveform_t waveform, float amp, float freq, float offset); // will replace this with a macro

int PrintTime(struct timespec start); //prints the time elapsed since start

int WriteFFTData(FILE *fd, double *dp, double *idp, int numvals); // dp and idp to a file fd

int WriteData(FILE *fd, double *dp, int numvals);

int FloatToDouble(double *dp, float *tmpdp, int length);

int DebugDecimation(void);

int PrintVals(double *dp, int numvals);

int AutoPower(gsl_complex *spectrum, double *power, int bufferSize);

int MakeComplexArray(double *dp, double *idp, gsl_complex *spectrum, int bufsize);

double gsl_complex_abs(gsl_complex z);

int CrossPower(gsl_complex *spectrum, gsl_complex *spectrum2, gsl_complex *crosspower, int bufferSize);

gsl_complex gsl_complex_mul(gsl_complex a, gsl_complex b);

gsl_complex gsl_complex_conjugate(gsl_complex z);	 

int Coherency(int bufferSize, gsl_complex *crosspower, double *autopower, double *autopower2, double *coherency);

int GetRealArray(gsl_complex *array, double * real, int bufferSize);

int WriteSpectralData(FILE *fd, double *autopower, double *autopower2, gsl_complex * crosspower, double *coherency, int numvals);

