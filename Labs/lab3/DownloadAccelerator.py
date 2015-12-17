import argparse
import threading
import requests
import os
import time
import timeit

class DownloadAccelerator:
	def __init__(self,args):
		self.args = None
		self.url = args.url
		self.numberOfThreads = args.threads
		self.download()

	def download(self):
		request = requests.head(self.url)
		#check to see if status code is not a 400 or 500 error
		if(request.status_code == requests.codes.ok):
			length = int(request.headers['content-length'])
			print "content length: ",length
			bytes = length/self.numberOfThreads
			print "bytes length: ",bytes
	
			self.filename = "downloads"+'/'+self.url.split('/')[-1].strip()
	
			if(self.url.endswith("/")):
				self.filename = "index.html"

			threads = []
			j = 1
			#100 bytes is 0-99
			for i in range(0,self.numberOfThreads):
				#byte range for each thread
				startRange = i * bytes
				endRange = (bytes * j)-1

				if(i == self.numberOfThreads - 1):
					endRange = length - 1
				j+=1
				tc = threadContent(self.url, startRange, endRange, self.filename);
				threads.append(tc)

			start = time.time()
			#start() creates thread and invokes run() method
			for thread in threads:
				thread.start()
			#join() wait for threads to finish
			for thread in threads:
				thread.join()
			end = time.time()
			seconds = end-start
			
			print "%s %d %d %s"%(self.url, self.numberOfThreads,length,seconds)
		else:
			request.raise_for_status()
			return

class threadContent(threading.Thread):
	def __init__(self, url, startRange, endRange,filename):
		threading.Thread.__init__(self)
		self.url = url
		self.startRange = startRange
		self.endRange = endRange
		self.filename = filename

	def run(self):
		byteRange = self.startRange - self.endRange
		#Accept-encoding doesn't return a compressed response
		headers = {'Range': 'bytes=byteRange','Accept-Encoding': 'identity'}
		r = requests.get(self.url, headers=headers)
		with open(self.filename, 'wb') as f:
			f.write(r.content)
			
def parse():
		#parse command line arguments
		parser = argparse.ArgumentParser(prog = 'DownloadAccelerator', description='parses command-line', add_help=True)
		parser.add_argument('-n', '--threads',  type=int, default='1')
		parser.add_argument('url', default= "http://cs360.byu.edu/fall-2015/labs/download-accelerator")
		
		#create a directory to store downloaded content
		if not os.path.exists("downloads"): 
			os.makedirs("downloads")
			
		args = parser.parse_args()
		return args
				 
if __name__ == '__main__':
	args = parse()
	DownloadAccelerator(args)











