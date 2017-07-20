import os 
from time import sleep
if __name__ == '__main__':
	try:
		while True:
			sleep(1)
			os.system("./acquire_and_process")
			os.system("scp input_data hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp input_data2 hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp fitted_data hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp fitted_data2 hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp fft_data hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp fft_data2 hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp spectraldata hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
	except KeyboardInterrupt:
			pass
