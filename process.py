from scipy.signal import butter, lfilter
import numpy as np
import sys
import time

def butter_bandpass(lowcut, highcut, fs, order=5):
    nyq = 0.5 * fs
    low = lowcut / nyq
    high = highcut / nyq
    b, a = butter(order, [low, high], btype='band')
    return b, a


def butter_bandpass_filter(data, lowcut, highcut, fs, order=5):
    b, a = butter_bandpass(lowcut, highcut, fs, order=order)
    y = lfilter(b, a, data)
    return y

BUFSIZE = 16384
LOWCUT = 8000
HIGHCUT = 10000000
fs = 125000000

if __name__ == '__main__':
	i = 0
	data = np.zeros((BUFSIZE, 1))
	start = time.time()
	for line in sys.stdin:
		data[i] = float(line)
		i += 1
	filtered = butter_bandpass_filter(data, LOWCUT, HIGHCUT, fs)
	end = time.time()
	print(filtered)
	np.savetxt("filtered", filtered)
	print("time elapsed = " + str(end - start))
