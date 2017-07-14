Acquisition and processing programs for RMT application.

The tentative structure is as follows:

Acquisition is triggered via serial or some other hardware line to the pixhawk.

Samples of the frequency range 1kHz to 1Mhz are considered.

Acquisition and processing will be comprised of a single script, acquire_and_process written in C. 

Acquisitions occur by filling up a buffer of 16*1024 samples, using the built-in rp_getAcq functions from the rp header
Processing consists:
1. Subtracting away linear and constant trends (fit to degree 1 poly and subtract the poly from the data)
2. Windowing. We use a hanning window, easy.
3. FFT. Using a function from the web. 
3 and 1/2. Possibly hone in on values of interest.
4. Compute the Cross Power Spectrum. 
5. Compute the Power Spectrum.
6. Write these to a file. The coherence and frequency response etc. can be computed offboard. 



Ultimately I think it's easiest to have a shell script which calls the data acquisition program upon powerup

The program simply operates indefinitely, acquiring data and processing it.

We would like to save as data the FFT of each buffer, or perhaps simply the power in certain spectra of interest, if memory limitations are a concern.
I think writing this data to a usb is easiest, easy enough to do that, simply mount the usb in the shell script to a folder /mnt/usb and then write to it in my acquire_and_process script. 


FFT Notes:

Eliminate ramp by fit to degree 1 poly and subtract as well as subtract the mean
Then window it (multiply by hanning, hamming, pers, ...)
 
