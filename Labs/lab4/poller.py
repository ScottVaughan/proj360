import errno
import select
import socket
import sys
import traceback
import time

from response import Response

class Poller:
    """ Polling server """
    def __init__(self, port, debug):
        self.host = ""
        self.port = port
        self.debug = debug
        self.open_socket()
        
        self.clients = {}
        self.media = {}
        self.cache = {}
        self.ClientTimeout = {}
        self.timeout = 1
        self.host = {}
        self.size = 1024

    def open_socket(self):
        """ Setup the socket for incoming clients """
        try:
            self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)
            self.server.bind((self.host,self.port))
            self.server.listen(5)
            self.server.setblocking(0)
        except socket.error, (value,message):
            if self.server:
                self.server.close()
            print "Could not open socket: " + message
            sys.exit(1)
            
    def configure(self):
        """Configure the server settings"""
        with open("web.conf") as webFile:
            for line in webFile.readlines():
                word = line.split()
                #skip blank lines
                if not line.strip():
                    continue
                elif word[0] == "host":
                    name = word[1]
                    path = word[2]
                    self.host[name] = path
                elif word[0] == "media":
                    ext = word[1]
                    mediaType = word[2]
                    if ext == "":
                        ext = "txt"
                        mediaType = "text/plain"
                    self.media[ext] = mediaType

                elif word[0] == "parameter":
                    self.timeout = int(word[2])
                else:
                    print "Error in web.conf"

                if self.debug:
                    print word[0]+" "+word[1]+" "+word[2]+"\n"
            webFile.close()
            
    def checkTime(self, startTime):
		endTime = time.time()
		clientsToClose = []
		if(endTime - startTime > 0.5):
			startTime = endTime
			for fd, value in self.clients.iteritems():
				if endTime - self.ClientTimeout[fd] >= self.timeout:
					clientsToClose.append(fd)
			self.closeClient(clientsToClose)
            
    def closeClient(self, clientsToClose):
        for client in clientsToClose:
			self.poller.unregister(client)
			self.clients[client].close()
			print "Closing client: %d"%client
			del self.clients[client]
			del self.ClientTimeout[client]
			del self.cache[client]
        
    def run(self):
        """ Use poll() to handle each incoming client."""
        self.poller = select.epoll()
        self.pollmask = select.EPOLLIN | select.EPOLLHUP | select.EPOLLERR
        self.poller.register(self.server,self.pollmask)
        self.configure()
        while True:
            # poll sockets
            try:
                startTime = time.time()
                fds = self.poller.poll(timeout=1)
                self.checkTime(startTime)
            except:
                print traceback.format_exc()
                return
            for (fd,event) in fds:
                self.ClientTimeout[fd] = time.time()
                # handle errors
                if event & (select.POLLHUP | select.POLLERR):
                    self.handleError(fd)
                    continue
                # handle the server socket
                if fd == self.server.fileno():
                    self.handleServer()
                    continue
                # handle client socket
                result = self.handleClient(fd)
                self.ClientTimeout[fd] = time.time()
                
    def handleError(self,fd):
        self.poller.unregister(fd)
        if fd == self.server.fileno():
            # recreate server socket
            self.server.close()
            self.open_socket()
            self.poller.register(self.server,self.pollmask)
        else:
            # close the socket
            self.clients[fd].close()
            del self.clients[fd]

    def handleServer(self):
        # accept as many clients as possible
        while True:
            try:
                (client,address) = self.server.accept()
            except socket.error, (value,message):
                # if socket blocks because no clients are available,
                # then return
                if value == errno.EAGAIN or errno.EWOULDBLOCK:
                    return
                print traceback.format_exc()
                sys.exit()
            # set client socket to be non blocking
            client.setblocking(0)
            self.clients[client.fileno()] = client
            self.cache[client.fileno()] = ""
            self.poller.register(client.fileno(),self.pollmask)
            self.ClientTimeout[client.fileno()] = time.time()
    
    def defineParseMedia(self, filepath):
        word = filepath.split("/")
        if word:
            fileExt = word[-1].split(".")[1]
            mediaType = self.media[fileExt]
            return mediaType

    def request(self,fd, data):

		response = Response(self.poller, self.clients, self.media)
		#get the line: GET / HTTP/1.1 ...
		line = data.splitlines()[0]
		word = line.split()
		filepath = ""
		if word[0] == "GET":
			body = ""
			if word[1] == "/":
				word[1] = "/index.html"
			if word[1].startswith("/index.html")or word[1].startswith("/static"):
				filepath = word[1][1:]

			try:
				with open(filepath, 'rb') as f:
					body = f.read()
					mediatype = self.defineParseMedia(filepath)
					statusCode = 200
					status = "OK"
			except IOError as (errno, strerror):
				if errno == 13:
					statusCode = 403
					status = "Forbidden"
				elif errno == 2:
					print "404 Error"
					statusCode = 404
					status = "Not Found"
				else:
					statusCode = 500
					status = "Internal Server Error"					  
		elif word[0] == "DELETE":
			statusCode = 501
			status = "Not Implemented"
		else:
			statusCode = 400
			status = "Bad Request"
		if statusCode == 200:
			response.handleResponse(statusCode, status, mediatype, filepath, body, fd)
		else:
			body = '<html><body><h1>%s</h1><br><h2>%s</h2></body></html>'  % (statusCode, status)
			contentLength = body
			response.handleResponse(statusCode, status, "text/plain", contentLength, body, fd)
			      
    def handleClient(self,fd):
		try:
			data = self.clients[fd].recv(self.size)
		except socket.error, (value,message):
			# if no data is available, move on to another client
			if value == errno.EAGAIN or errno.EWOULDBLOCK:
				return
				print traceback.format_exc()
				sys.exit() 

		if data:
			if not data in self.cache[fd]:
				#append cache data
				self.cache[fd] += data
			#check for end of data
			if self.cache[fd].endswith("\r\n\r\n"):
				print self.cache[fd]
				self.request(fd, self.cache[fd])
				self.cache[fd] = ""
		else:
			self.poller.unregister(fd)
			self.clients[fd].close()
			del self.clients[fd]
			del self.ClientTimeout[fd]
			del self.cache[fd]
		

                
    
    
