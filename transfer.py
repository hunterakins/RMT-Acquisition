# call acq and process and send its results to scp_path
# I store my scp path in a text file and just cat it, pipe it to this script
import os 
from time import sleep
import sys


if __name__ == '__main__':
	print(len(sys.argv))
	scp_path = sys.argv[1]	
	os.system("scp display.py " + scp_path)
	try:

		while True:
			sleep(1)
			os.system("./acquire_and_process")
			os.system("scp Data/input_data i " + scp_path)
			os.system("scp Data/input_data2 " + scp_path)
			os.system("scp Data/fitted_data " + scp_path)
			os.system("scp Data/fitted_data2 " + scp_path)
			os.system("scp Data/fft_data " +  scp_path)
			os.system("scp Data/fft_data2 " + scp_path)
			os.system("scp Data/spectraldata " + scp_path)
	except KeyboardInterrupt:
			pass
