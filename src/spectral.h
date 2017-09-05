
// computes autopower of spectrum and writes it into power
int AutoPower(complex *spectrum, double *power, int bufferSize);

// computes crosspower between two input spectra and stores in the complex valued array crosspower 
int CrossPower(gsl_complex *spectrum, gsl_complex *spectrum2, gsl_complex *crosspower, int bufferSize);

// computes coherency of two input channels given the crosspower and respective autopower
int Coherency(int bufferSize, gsl_complex *crosspower, double *autopower, double *autopower2, double *coherency);
