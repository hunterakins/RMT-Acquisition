''' display program for client computer to create a "real time" plot of the two channels time series data
fs is the sampling time
'''
import numpy as np

from matplotlib import pyplot as plt
import time
import sys

fs = 1953000
nyquist = fs/2
filenames = ['input_data', 'input_data2', 'fitted_data', 'fitted_data2', 'fft_data', 'fft_data2', 'spectraldata']
freq = np.linspace(20, nyquist, 8172)
plt.ion()
fig = plt.figure(figsize=(18, 20))
plt.plot(np.loadtxt('input_data'))
numplots = 10
try:
	while True:
		plt.pause(2)
		plt.clf()
		dats = [np.loadtxt(x) for x in filenames]
		for i in range(7):
			plt.subplot(numplots//2+1, 2, i+1)
			plt.title(filenames[i])
			if i == 6:
				for j in range(5):
					plt.subplot(numplots//2+1, 2, i + 1 + j)
					plt.plot(freq, dats[i][20:, j])
			else:
				plt.plot(dats[i][:])	
		plt.draw()
except KeyboardInterrupt:
	pass
