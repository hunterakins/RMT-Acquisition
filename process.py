# data comes in as a stream
# I write stream into an array
# I eliminate DC offset by subtracting the mean
# I fit the data to a line and subtract away the line
# I window the data
# Then I FFT

import numpy as np
import sys
import time 

BUFSIZE = 16384
SAMPLE_FREQ = 1953000

def sub_lin_offset(data):
	num_vals = len(data)
	a, b = np.polyfit(np.arange(0, num_vals), data, 1)
	linfit = lambda x: a*x + b
	linvals = linfit(num_vals)
	return data - linvals

def hamming_window(data):
	num_vals = len(data)
	return np.multiply(np.hamming(num_vals),data[:,0])

def comp_rfft(data):
	num_vals = len(data)
	fftvals = np.fft.rfft(data)
	fftfreq = np.fft.rfftfreq(num_vals, d=1/SAMPLE_FREQ)
	return fftfreq, fftvals

if __name__ == '__main__':
	i = 0
	data = np.zeros((BUFSIZE,1))
	start = time.time()
	for line in sys.stdin:
		data[i] = float(line)
		i += 1
	np.savetxt("ogdata", data)
	data = sub_lin_offset(data)	
	data = hamming_window(data)
	fftfreq, fftvals = comp_rfft(data)
	np.savetxt("fftdata", np.append(fftfreq, fftvals))
	end = time.time()
	print("time elapsed = " + str(end-start))
