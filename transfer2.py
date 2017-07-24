import os 
from time import sleep, time


if __name__ == '__main__':
	try:
		while True:
			sleep(1)
			start = time()
			os.system("./par_acq_and_process")
			os.system("scp channel1 hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp channel2 hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			end = time()
			print("transfer time = " + str(end - start))
	except KeyboardInterrupt:
			pass
