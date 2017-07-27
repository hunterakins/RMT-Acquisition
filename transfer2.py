import os 
from time import sleep, time
import sys


if __name__ == '__main__':
	scppath = sys.argv[1] 
	try:
		while True:
			sleep(1)
			start = time()
			os.system("./par_acq_and_process")
			os.system("scp Data/channel1 " + scp_path)
			os.system("scp Data/channel2 " + scp_path)
			end = time()
			print("transfer time = " + str(end - start))
	except KeyboardInterrupt:
			pass
