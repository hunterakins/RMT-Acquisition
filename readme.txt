Acquisition and processing programs for RMT application.

The tentative structure is as follows:

Acquisition is triggered via i2c or some other hardware line to the pixhawk.

Samples of the frequency range 1kHz to 1Mhz are considered.



Acquisition and processing will be comprised of a single script. I currently have a few working drafts: acquire_and_process works, write the FFT of the two channels to files in the Data folder.
par_acq_and_process processes the two channels in parallel, writes two files channel1 and channel2 in the data folder. 

The python scripts transfer scp the files over to the client computer, they accept as an argument the scp argument: hostname@hostipaddress:/directory

The display.py scripts should be called on the client computer in the same folder as the data files, they will produce plots that update periodically. 

The avgcoherency function is neat: it acquires 10 samples, computes the average crosspower of these and returns the coherency of the two channels. 




Processing consists:
1. Subtracting away linear and constant trends (fit to degree 1 poly and subtract the poly from the data)
2. Windowing. We use a hanning window, easy.
3. FFT. Using a function from the web. 
3 and 1/2. Possibly hone in on values of interest.
4. Compute the Cross Power Spectrum. 
5. Compute the Power Spectrum.


 
