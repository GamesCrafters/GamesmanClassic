
#  JsonServer.py
#  Copyright (c) 2008, Patrick Horn
#________________________________________________ 
# This source code is dual-licensed as BSD/GPL
# The text of these licenses may be found here:
# http://opensource.org/licenses/bsd-license.php
# http://opensource.org/licenses/gpl-2.0.php
#------------------------------------------------ 


# Beware the Jabberwock, my son! The jaws that bite, the claws that catch!
# Beware the Jubjub bug, and read vazor.com/cjson.html#!
from cjson import decode as json_dec, encode as json_enc

# Or, if you like simplejson better:
# from simplejson import loads as json_dec, dumps as json_enc

import urllib
import traceback
import socket
import asyncore
from asynchat import async_chat

def get_params(querystring):
	params = {}
	querystring=querystring.strip()
	split_args = querystring.replace('&',';').split(';')
	for kv in split_args:
		if '=' not in kv and 'board' not in params:
			# convenience
			key = 'board'
			value = kv
		else:
			key, value = kv.split('=', 1)
		params[key] = urllib.unquote_plus(value)
	return params

def jsonify(dictionary):
	return json_enc(dictionary)

class JsonConnection(async_chat):
	def __init__(self, conn, handler):
		async_chat.__init__(self, conn=conn)
		self.ibuffer = []
		self.handler = handler
		self.set_terminator('\n')
	
	def collect_incoming_data(self,data):
		self.ibuffer.append(data)
	
	def found_terminator(self):
		msg = ''.join(self.ibuffer)
		self.ibuffer = []

		request = {}

		try:
			request = get_params(msg)
			response = self.handler(request)
			msg = {"status": "ok", "response": response}
			
		except Exception, e:
			print "Error when handling JSON query."
			print e
			traceback.print_exc()
			print "---------------"
			msg = {"status": "error", "msg": str(e)}
			
		if "id" in request:
			msg["id"] = request["id"]

		try:
			self.push(jsonify(msg)+"\n")
		except:
			print "FAILURE in encoding message"
			print repr(msg)
			traceback.print_exc()

class JsonServer(asyncore.dispatcher):
	def __init__(self, port, handler):
		asyncore.dispatcher.__init__(self)
		self.port = port
		self.handler = handler

		self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
		self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # bloody timeout rules
		self.bind(("", port))
		self.listen(5)
		print "Awaiting a connection on port %d"%port

	def handle_accept(self):
		chan, addr = self.accept()
		JsonConnection(chan, self.handler)

	@staticmethod
	def run():
		asyncore.loop()

