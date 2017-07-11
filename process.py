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
	np.savetxt('data', data)
	fftfreq = np.fft.fftfreq(BUFSIZE, d=TIMESTEP)
	end = time.time()
	fftarray = np.array([[fftdata[i], fftfreq[i]] for i in range(BUFSIZE)])
	print(fftarray)
	np.savetxt("fft",fftarray)
