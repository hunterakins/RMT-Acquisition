import os 
from time import sleep
if __name__ == '__main__':
	try:
		while True:
			sleep(1)
			os.system("./acquire_and_process")
			os.system("scp Data/input_data hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp Data/input_data2 hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp Data/fitted_data hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp Data/fitted_data2 hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp Data/fft_data hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp Data/fft_data2 hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
			os.system("scp Data/spectraldata hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
	except KeyboardInterrupt:
			pass
