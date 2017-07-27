''' display program for client computer to create a "real time" plot of the two channels time series data
fs is the sampling time
'''
import numpy as np

from matplotlib import pyplot as plt
import time
import sys


def getData(channel):
	channel = np.loadtxt(channel);
	inputdata = channel[:,0]
	window = channel[:,1]
	realf = channel[:,2]
	imf = channel[:,1]
	

fs = 1953000
nyquist = fs/2
filenames = ['/Data/channel1', 'Data/channel2', 'Data/crosspower', 'Data/coherency']
freq = np.linspace(20, nyquist, 8172)
plt.ion()
fig = plt.figure(figsize=(18, 20))
plt.plot(np.loadtxt('input_data'))
numplots = 10
try:
	while True:
		plt.pause(2)
		plt.clf()
		channel1 = np.loadtxt
		for i in range():
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
