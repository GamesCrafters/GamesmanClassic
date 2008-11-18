import sys
import DatabaseHelper
import Solver
import Puzzle
import pickle

def setList(lis, element, value, default):
	length = len(lis)
	if element < length:
		lis[element] = value
	else:
		for i in xrange(length,element):
			lis.append(default)
		lis.append(value)
	

def numBits(x):
	a = 0
	while x:
		x >>= 1
		a += 1
	return a

def writeDatabase(puzzname, options):
	if puzzname.find('/') != -1 or puzzname.find('\\') != -1:
		raise ArgumentException, "invalid puzzle"

	module = __import__(puzzname)
	puzzleclass = getattr(module, puzzname)
	
	print "Solving "+puzzleclass.__name__+" with options "+repr(options)+"..."
	puzzle = puzzleclass(**options) # instantiates a new puzzle object
	solver = Solver.Solver()
	solver.solve(puzzle,verbose=True)
	
	print "Solved!  Generating array of values"
	fields =[('remoteness',numBits(solver.get_max_level()))]
	bitClass = DatabaseHelper.makeBitClass(fields)
	
	CHUNKBITS=8
	CHUNK = 1<<CHUNKBITS
	
	alldata = {}
	default = str(bitClass(remoteness=0)) # remoteness==0 but not solution means not seen.
	for i in range(solver.get_max_level()+1): # +1 includes starting position
		print 'Level %d'%i
		for x in solver.levels[i]:
			position = hash(x)
			chunknum = position>>CHUNKBITS
			chunkoff = position&(CHUNK-1)
			
			arr = alldata.get(chunknum, [])
			val = bitClass(remoteness=i)
			setList(arr, chunkoff, str(val), default)
			alldata[chunknum] = arr
	
	f = open(DatabaseHelper.getFileName(puzzname, options),"wb")

	print "writing to file "+str(f)
	pickle.dump({'puzzle': puzzname,
		'options': options,
		'fields': fields,
		'chunkbits': CHUNKBITS}, f)
	offsetbase = (f.tell() + (CHUNK-1)) >> CHUNKBITS
	
	#for i in xrange((len(l)+CHUNK)/CHUNK):
	#	bytechunk = ''.join(l[i*CHUNK : (i+1)*CHUNK])
	#	f.write(bytechunk)
	for pos in alldata:
		f.seek((offsetbase + pos)<<CHUNKBITS)
		f.write(''.join(alldata[pos]))
	
	f.close()
	print "Done!"

if __name__=='__main__':
	#writeDatabase('Wheel',
	#	{'seqSize': 6})
	if len(sys.argv)<2:
		print >>sys.stderr, "Arguments: Python file to import (without .py)"
		sys.exit(0)
	writeDatabase(sys.argv[1], {})

