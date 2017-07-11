import numpy as np
import sys
import time


SAMP_FREQ = 1953000
BUFSIZE = 16384
TIMESTEP = 1/SAMP_FREQ

if __name__ == '__main__':
	i = 0
	data = np.zeros((BUFSIZE, 1))
	start = time.time()
	for line in sys.stdin:
		data[i] = float(line)
		i += 1
	fftdata = np.fft.fft(data).real
	fftfreq = np.fft.fftfreq(BUFSIZE, d=TIMESTEP)
	end = time.time()
	fftarray = np.append(fftdata, fftfreq)
	np.savetxt("fft",fftdata)
