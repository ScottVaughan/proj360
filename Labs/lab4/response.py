
import time
import os

class Response:

	def __init__(self, poller, clients, media):
		self.poller = poller
		self.clients = clients
		self.media = media
		
	def getTime(self, t):
		gmt = time.gmtime(t)
		format = '%a, %d %b %Y %H:%M:%S GMT'
		timeFormat = time.strftime(format,gmt)
		return timeFormat
			
	def handleResponse(self, statusCode, status, mediatype, filename, body, fd):

			httpResponse = "HTTP/1.1 %s %s\r\n" % (statusCode, status)

			t = time.time()
			date = self.getTime(t)
			httpResponse += "Date: %s\r\n" % date

			httpResponse += "Server: localhost\r\n"
			
			if statusCode == 200:
				cl = os.stat(filename).st_size
			else:
				cl = len(body)
			httpResponse += "Content-Length: %s\r\n" % cl
			
			httpResponse += "Content-Type: %s\r\n" % mediatype
			
			if statusCode == 200:
				mt = os.stat(filename).st_mtime
				lastModified = self.getTime(mt)
				httpResponse += "Last-Modified: %s\r\n\r\n" % lastModified
			
			print httpResponse

			self.clients[fd].send(httpResponse)
			try:
				self.clients[fd].send(body)
			except: 
				print "error in response"
			
