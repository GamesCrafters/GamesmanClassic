import pickle

# Don't want someone requesting "/etc/passwd" as a game name.
def replacebadchars(fname):
	fname = fname.replace('/','-').replace('\\','-').replace('\x00','').replace("\"",'-')
	return fname.replace('_','-').replace('=','-').replace(' ','+')

def getFileName(gamename, options):
	fname = 'db_'+replacebadchars(gamename)
	mykeys= options.keys()
	mykeys.sort()
	for key in mykeys:
		val = str(options[key])
		fname += '_' + key + '=' + replacebadchars(val)
	return fname

def makeBitClass(fields):
	totalbits = sum([f[1] for f in fields])
	totalbytes = (totalbits+7) / 8
	
	ctypefields = [(f[0], "ctypes.c_uint", f[1]) for f in fields]
	
	class Database_Bitfield: #(ctypes.BigEndianStructure):
		_pack_ = 1
		_fields_ = ctypefields
		
		bytesize = totalbytes
		
		def __init__(self, remoteness=0):
			self.remoteness = remoteness
		@staticmethod
		def create(mystr):
			val = 0
			print mystr
			while mystr:
				val <<= 8
				val |= ord(mystr[0])
				mystr = mystr[1:]
			print " = > " +str(val)
			return Database_Bitfield(val)
		@staticmethod
		def read_from_file(myfile):
			bytes = myfile.read(totalbytes)
			return Database_Bitfield.create(bytes)
		
		def __str__(self):
			x = self.remoteness 
			print x
			s = ''
			for i in range(totalbytes):
				s = chr(x&0xff) + s
				x >>= 8
			print " <= " + repr(s)
			return s
		def to_dictionary(self, defaults=None):
			d = defaults or {}
			for f in ctypefields:
				d[f[0]] = getattr(self,f[0])
			return d
	
	return Database_Bitfield

class QueryPuzzle:
	def __init__(self,pname):
		self.puzzleclass = getattr(__import__(pname), pname)
		#self.puzzle = self.puzzleclass(**self.header['options'])
	
	def unserializePuzzle(self, options):
		myopts = {}
		myopts.update(self.puzzleclass.default_options)
		myopts.update(options)
		boardstr = myopts['board']
		del myopts['board']
		return self.puzzleclass.unserialize(myopts, boardstr)
		
	def getPuzzle(self, *args, **kwargs):
		options = {}
		options.update(self.puzzleclass.default_options)
		options.update(kwargs)
		return self.puzzleclass.unserialize(options, *args)
	
	def read(self, mypuzzle, mydict = {}):
		d = {}
		d.update(mydict)
		d['remoteness'] = -1
		d['board'] = mypuzzle.serialize()
		return d
	
class OpenDB(QueryPuzzle):
	def __init__(self,fname):
		self.fp = open(fname, 'rb')
		self.header = pickle.load(self.fp) # Yes, possibly insecure
		self.startpos = self.fp.tell()
		if 'chunkbits' in self.header:
			cb = self.header['chunkbits']
			chunkmax = (1<<cb)-1
			self.startpos = (self.startpos + chunkmax) & (~chunkmax)
		
		self.bitClass = makeBitClass(self.header['fields'])
		
		pname = replacebadchars(self.header['puzzle'])
		QueryPuzzle.__init__(self, pname)
		print "Opened database "+fname

	def unserializePuzzle(self, options):
		for o in options.iterkeys():
			if o in self.header['options']:
				if options[o] != self.header['options'][o]:
					raise ValueError("invalid options passed to database")
		return self.puzzleclass.unserialize(self.header['options'], options['board'])
	
	def read(self, mypuzzle, mydict = None):
		if mypuzzle.is_illegal() and not mypuzzle.is_a_solution:
			print 'ILLEGAL: '+str(mypuzzle)
			return None # Illegal position.
		
		h = hash(mypuzzle)
		self.fp.seek(self.startpos + (self.bitClass.bytesize * h))
		info = self.bitClass.read_from_file(self.fp)
		d = info.to_dictionary(mydict)

		if d['remoteness'] == 0 and not mypuzzle.is_a_solution():
			d['remoteness'] = -1 # Not a reachable position!
		
		d['board'] = mypuzzle.serialize()
		return d
	

