#!/usr/bin/python

# Run the server with "python httpserver.py"

# This file is not executable for safety if these files are placed in a real cgi-bin directory.

import os
import sys
if __name__=='__main__' and 'REQUEST_METHOD' in os.environ:
	print "HTTP/1.1 500 Internal Server Error"
	print "Content-Type: text/html\n\nThis is not a CGI script."
	sys.exit(1)

from BaseHTTPServer import HTTPServer, BaseHTTPRequestHandler

import traceback

import DatabaseServer
import DatabaseHelper
import JsonServer

#### ENVIRONMENT SETTINGS.  CHANGE THESE FOR YOUR COMPUTER.
PORTNUM=8080
GAMESMANWEB=os.environ["HOME"]+"/workspace/GamesmanWeb2/deploy/ui"

#### DEFAULT PUZZLE.
PUZZLENAME="fcg"
PYTHONFILE="FCG"
# Also accepts command line args "internalpuzzlename PythonClass"
if len(sys.argv) >= 3:
	PUZZLENAME = sys.argv[1]
	PYTHONFILE = sys.argv[2]

PUZZLEJSP = "/puzzle.jsp"
ALLOWED_PATHS = ["", "/images", "/js", "/styles", "/images/fcg", "/images/pegsol", "/applets"]
executor = DatabaseServer.Executor(PYTHONFILE)
serviceroot="/gcweb/service/gamesman/puzzles/"+PUZZLENAME

def stripjsp(header):
	while True:
		begin = header.find("<%")
		if begin == -1:
			break
		end = header.find("%>", begin)
		if end == -1:
			break
		header = header[:begin] + header[end+2:]
	return header

def parsepuzzlejsp(fname, puzzlename):
	f = open(GAMESMANWEB + fname)
	text = f.read()
	f.close()
	text = text.replace("<%= canonicalName %>", puzzlename)
	text = text.replace("<%= uifile %>", puzzlename)
	header, footer = text.split("<% dynamicInclude(out, uifile); %>", 1)
	header = stripjsp(header)
	footer = stripjsp(footer)
	return  header, footer

class GamesmanHTTPHandler(BaseHTTPRequestHandler):
	def handle_my_error(self,type):
		err = traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_traceback)
		textformat = ''.join(err)
		htmlformat = textformat.replace('\n','<br>\n').replace(' ','&nbsp;')
		print textformat
		self.send_response(500, "Internal Server Error")
		self.send_header('Content-Type', 'text/plain')
		self.end_headers()
		self.wfile.write('\n<h1>Python error occurred in '+type+' request.</h1>'+htmlformat)
	
	def do_request(self, postargs=''):
		path = self.path
		if path[:len(serviceroot)]==serviceroot:
			try:
				query = path[len(serviceroot)+1:]
				query = "method="+query
				resp = {"status": "ok", "response": executor(JsonServer.get_params(query))}
				output = JsonServer.jsonify(resp)
				self.send_response(200, "OK")
				self.send_header('Content-Type', "text/javascript")
				self.send_header('Content-Length', str(len(output)))
				self.end_headers()
				print "OUTPUT   IS:"
				print output
				self.wfile.write(output)
			except:
				traceback.print_exc()
				self.send_error(404, "Not Found")
			return
		else:
			dir, file = path.rsplit('/', 1)
			if file.find('.')==-1:
				file = PUZZLENAME+".html"
			if dir in ALLOWED_PATHS:
				try:
					file, ext = file.rsplit(".", 1)
					file = file.replace("/","_").replace("\\","_")
					ext = ext.replace("/","_").replace("\\","_")
					fullpath = dir + "/" + file + "." + ext
					
					print GAMESMANWEB + fullpath
					myfp = open(GAMESMANWEB + fullpath,"r")
					head, foot = parsepuzzlejsp(PUZZLEJSP, file)
				except:
					traceback.print_exc()
					self.send_error(404, "Not Found")
					return

				if ext == "html" or ext == "css":
					ctype = "text/"+ext
				elif ext == "js":
					ctype = "text/javascript"
				elif ext == "jar":
					ctype = "application/java-archive"
				else:
					ctype = "image/"+ext
				self.send_response(200, "OK")
				self.send_header('Content-Type', ctype)
				self.end_headers()
				if ext == "html":
					self.wfile.write(head)
				while True:
					data = myfp.read(1024*1024)
					print "reading"+str(len(data))
					if data:
						self.wfile.write(data)
					else:
						break
				myfp.close()
				if ext == "html":
					self.wfile.write(foot)
				return
			else:
				self.send_error(403, "Forbidden directory "+str(dir))
	
	def do_GET(self):
		try:
			self.do_request()
		except:
			print "ERROR"
			self.handle_my_error("GET")
	def do_POST(self):
		try:
			len = int(self.headers.get('Content-Length', '0'))
			contents = self.rfile.read(len)
			self.do_request(contents)
		except:
			self.handle_my_error("POST")
		
		
	

def httpServer(port=PORTNUM, host=''):
	print "Starting HTTP server on port %d..." % (port,)
	server_address = (host, port)
	httpd = HTTPServer(server_address, GamesmanHTTPHandler)
	httpd.serve_forever()

if __name__=='__main__':
	httpServer()

