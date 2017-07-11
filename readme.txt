Acquisition programs for RMT application.

The tentative structure is as follows:
Acquisition is triggered via serial.
Samples are piped to a python script which uses the MTpy library available at http://github.com/geophysics/mtpy.
Acquisition program is coded in C and either written to a file.
Samples of the frequency range 10kHz to 10Mhz are considered.

acquire_and_out will fill up a buffer of 16*1024 samples. 
At 125 Msps, this only spans 131 microseconds. Thus, only a fraction of certain waveforms of interest will be detected.
16,384 samples is enough to to make measurements, but the sampling frequency limits the bandwidth somewhat. 
At high sampling rates, the time window is too small. 
At low sampling rates, I am Nyquist-limited. 
Let's limit our frequency at 1MHz. That lets me sample at either 1.953 MHz or at 15.625 MHz.
Then my buffer of 16384 samples will span either 8.4 milliseconds or 1.04 milliseconds. 

We care about 30 meters, so our minimum frequency is, say 10 kHz to be on the safe side. 
My time uncertainty is either 8.4 milliseconds or 1.04 milliseconds, so my uncertainty in frequency will be either around 1 kHz or rou 10kHz.
Obviously 10kHz uncertainty is unacceptable if I want to sample as low as 10 Khz.
Thus, I propose having a switch in the acquire_and_out, which will set the sampling rate to be 1.953Msps or 15.625Msps, to be determined by the depth of the survey.

FFT Notes:
Eliminate ramp by fit to degree 1 poly and subtract as well as subtract the mean
Then window it (multiply by hanning, hamming, pers, ...)
 
