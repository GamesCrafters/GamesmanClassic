import ctypes # PYTHON IS AWESOME!
import pickle

# Don't want someone requesting "/etc/passwd" as a game name.
def replacebadchars(fname):
	fname = fname.replace('/','-').replace('\\','-').replace('\x00','').replace("\"",'-')
	return fname.replace('_','-').replace('=','-').replace(' ','+')

def getFileName(gamename, options):
	fname = 'db_'+replacebadchars(gamename)
	for key in options.iterkeys():
		val = str(options[key])
		fname += '_' + key + '=' + replacebadchars(val)
	return fname

def makeBitClass(fields):
	totalbits = sum([f[1] for f in fields])
	totalbytes = (totalbits+7) / 8
	
	ctypefields = [(f[0], ctypes.c_uint, f[1]) for f in fields]
	
	class Database_Bitfield(ctypes.BigEndianStructure):
		_pack_ = 1
		_fields_ = ctypefields
		
		bytesize = totalbytes
		
		@staticmethod
		def create(mystr):
			return ctypes.cast(mystr, ctypes.POINTER(Database_Bitfield)).contents
		@staticmethod
		def read_from_file(myfile):
			bytes = myfile.read(totalbytes)
			return Database_Bitfield.create(bytes)
		
		def __str__(self):
			return ctypes.string_at(ctypes.addressof(self), totalbytes)
		def to_dictionary(self, defaults=None):
			d = defaults or {}
			for f in ctypefields:
				d[f[0]] = getattr(self,f[0])
			return d
	
	return Database_Bitfield

class OpenDB:
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
		self.puzzleclass = getattr(__import__(pname), pname)
		#self.puzzle = self.puzzleclass(**self.header['options'])
	
	def getPuzzle(self, *args, **kwargs):
		return self.puzzleclass(**self.header['options'])
	
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
		
		d['board'] = str(mypuzzle)
		return d
	

