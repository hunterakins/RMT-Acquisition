import os 
from time import sleep
if __name__ == '__main__':
	try:
		while True:
			sleep(3)
			os.system("./avg_coherency")
			os.system("scp avg_coh hunterakins@131.243.186.72:/home/hunterakins/Documents/Data")
	except KeyboardInterrupt:
			pass
