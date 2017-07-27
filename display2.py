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
	imf = channel[:,3]
	autopower = channel[:,]
	return inputdata, window, realf, imf, autopower
title = ['input_data', 'windowed', 'realfft', 'imfft', 'autopower']

fs = 1953000
nyquist = fs/2
filenames = ['channel1', 'channel2', 'crosspower', 'coherency']
freq = np.linspace(20, nyquist, 8172)
plt.ion()
fig = plt.figure(figsize=(18, 20))
plt.plot(np.loadtxt('input_data'))
numplots = 10
try:
	while True:
		plt.pause(2)
		plt.clf()
		channel1 = getData('channel1')
		channel2 = getData('channel2')
		for i in range(5):
			plt.subplot(12, 2, 2*i+1)
			plt.title(title[i])
			plt.plot(channel1[i])
			plt.subplot(12,2,2*i+2)
			plt.plot(channel2[i])
			plt.title(title[i])
		plt.draw()
except KeyboardInterrupt:
	pass
