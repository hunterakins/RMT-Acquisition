from scipy.signal import butter, lfilter
import numpy as np
from matplotlib import pyplot as plt
import sys


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

if __name__ == '__main__':
	low = 540000
	high = 550000
	dats = np.loadtxt(sys.argv[1])
	filtereddats = butter_bandpass_filter(dats, low, high, 125000000)
	plt.plot(dats[0:100000])
	plt.plot(filtereddats[0:100000])
	np.savetxt(sys.argv[1] + 'bw' + str(low) + 'to' + str(high), filtereddats)
	plt.show()
