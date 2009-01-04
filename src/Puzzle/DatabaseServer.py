#!/usr/bin/env python

#  DatabaseServer.py
#  Copyright (c) 2008, Patrick Horn
#________________________________________________ 
# This source code is dual-licensed as BSD/GPL
# The text of these licenses may be found here:
# http://opensource.org/licenses/bsd-license.php
# http://opensource.org/licenses/gpl-2.0.php
#------------------------------------------------ 

import sys
import random

# TCP Server
from DatabaseHelper import *
from JsonServer import JsonServer, jsonify, get_params

import FCG

LOUDNESS = 1

class Executor:
	def __init__(self, game, db=None):
		self.databases = {}
		self.db = QueryPuzzle(game)
		self.game = game
		if db:
			self.db = db
			self.puzz = db.getPuzzle()
	
	def getDatabase(self, options):
		myopts = self.db.puzzleclass.default_options
		for k in myopts.iterkeys():
			if k in options:
				myopts[k] = options[k]
		dbname = getFileName(self.game, myopts)
		if dbname in self.databases:
			return self.databases[dbname]
		try:
			self.databases[dbname] = OpenDB(dbname)
		except:
			self.databases[dbname] = self.db # default
		return self.databases[dbname]

	def do_shuffle(self, count):
		p = self.puzz
		for i in range(int(count)):
			newp = p.do_move(random.choice(p.generate_moves()))
			if newp.is_a_solution():
				p = newp
				break
			if newp.is_illegal():
				continue
			p = newp
		
		ret = self.db.read(p)
		return ret

	def do_getMoveValue(self, req):
		#p = parse_FCG_board(req['board'])
		db = self.getDatabase(req)
		p = db.unserializePuzzle(req)
		return db.read(p)
	
	def do_getNextMoveValues(self, req):
		#mypuzz = parse_FCG_board(req['board'])
		db = self.getDatabase(req)

		curval = self.do_getMoveValue(req)

		mypuzz = db.unserializePuzzle(req)
		nextmoves = [(mypuzz.do_move(m), m) for m in mypuzz.generate_moves()]
		nextvalues = [db.read(p, {'move': m}) for p, m in nextmoves if not p.is_illegal()]
		#minremoteness = min(*[p['remoteness'] for p in nextvalues])
		minremoteness = curval['remoteness']-1
		if minremoteness < -1:
			return nextvalues
		for val in nextvalues:
			if val.get('value',-1) == -1:
				if val['remoteness']<=minremoteness:
					val['value'] = 3
				elif val['remoteness']==minremoteness+1:
					val['value'] = 2
				else:
					val['value'] = 1
		return nextvalues
	def __call__(self, req):
		# if debug: print req
		if LOUDNESS >= 1:
			print req
		result = None
		method = req['method']
		if method == 'random':
			result = self.do_shuffle(req['count'])
		elif method == 'getMoveValue':
			result = self.do_getMoveValue(req)
		elif method == 'getNextMoveValues':
			result = self.do_getNextMoveValues(req)
		else:
			raise Exception, "Invalid command %s"%req['method']
		if LOUDNESS >= 2:
			print "  --> RETURNED "+str(result)
		return result


if __name__=='__main__':
	### Parse arguments
	temp = None
	port = None
	db = None
	game = None
	query = []
	cmd = None
	for arg in sys.argv:
		if arg=='--help' or arg=='-h':
			print \
"""Usage: %s  {-q QUERY | -p PORT} -d DB
	-q, --query \tQuery the database and return the result
	-p, --port \tHost a server on this port
	-g, --game \tUse this game
	-d, --db, --database \tDatabase from DatabaseWriter.py
	-v, --verbose \tTurn verbose to level 0, 1 (default) or 2.

Available Queries (URL encoded)
	method=random&count=NUM \n\t\tDo NUM random moves and query that position
	method=getNextMoveValues&board=BOARDSTRING \n\t\tQuery BOARDSTRING and also query all of the children
	method=getMoveValue&board=BOARDSTRING \n\t\tQuery BOARDSTRING

""" % sys.argv[0]
			sys.exit(0)
		
		if arg[0:2]=='--':
			temp = arg[2:]
		elif arg[0]=='-':
			temp = arg[1:]
		else:
			if temp == 'port' or temp == 'p':
				port = int(arg)
			elif temp == 'database' or temp == 'db' or temp == 'd':
				dbfile = arg
				### Open database
				db = OpenDB(dbfile)
				game = db.header['puzzle']
			elif temp == 'game' or temp == 'g':
				game = arg
				db = QueryPuzzle(arg)
			elif temp == "query" or temp == 'q':
				query.append(arg)
			elif temp == "shuffle":
				cmd(arg)
			elif temp == "verbose" or temp == 'v':
				LOUDNESS=int(arg)
			temp = None
	
	if db == None:
		print "No database specified.  Use --help for options"
		sys.exit(-1)

	print LOUDNESS

	handle_gcweb = Executor(game, db)

	if query:
		ret = 0
		for q in query:
			try:
				print jsonify(handle_gcweb(get_params(q)))
			except Exception, e:
				import traceback
				print "Error in handling query:"
				ret += 1
				print e
				traceback.print_exc()
		sys.exit(ret)

	if not port:
		print "No port or query specified.  Use --help for options"
		sys.exit(-1)
	
	server = JsonServer(port, handle_gcweb)

	JsonServer.run()


