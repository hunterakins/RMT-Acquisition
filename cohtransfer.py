import os 
from time import sleep
import sys


if __name__ == '__main__':
	scp_path = sys.argv[1]
	try:
		while True:
			sleep(3)
			os.system("./avg_coherency")
			os.system("scp avg_coh "+ scp_path)
	except KeyboardInterrupt:
			pass
