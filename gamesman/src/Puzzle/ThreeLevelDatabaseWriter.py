import sys
import DatabaseHelper
import ThreeLevelSolver
import OldSolver
import UnreverseSolver
import Puzzle
import pickle
import time

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

def writeDatabase(puzzname, passedoptions,isunreverse,solverclass):
	if puzzname.find('/') != -1 or puzzname.find('\\') != -1:
		raise ArgumentException, "invalid puzzle"

	module = __import__(puzzname)
	puzzleclass = getattr(module, puzzname)
	
	options = {}
	options.update(puzzleclass.default_options)
	options.update(passedoptions)
	print "Solving "+puzzleclass.__name__+" with options "+repr(options)[1:-1]+"..."
	puzzle = puzzleclass.unserialize(options) #.generate_start(**options) # instantiates a new puzzle object
	solver = solverclass()
	solveriter = solver.solve(puzzle,verbose=True)
	# Generator function
	
        maxlevel = 255 # WARNING: BIG ASSUMPTION, NEED A 2-PASS SOLVER!
	
	options = {}
	options.update(puzzleclass.default_options)
	options.update(passedoptions)
	print options
	
	fields =[('remoteness',numBits(maxlevel))]
	if isunreverse:
		fields.append(('score', numBits(maxlevel*2)));
	print fields
	
	bitClass = DatabaseHelper.makeBitClass(fields)
	
	CHUNKBITS=8
	CHUNK = 1<<CHUNKBITS
	
	filename = DatabaseHelper.getFileName(puzzname, options)
	default = str(bitClass(remoteness=0)) # remoteness==0 but not solution means not seen.
	print "writing to file %s"%filename
	maxchunkwritten = -1
        countitem = 0
	pct = 0.0

	data = {} # indexed by chunk

	# Not as innocent as it looks: solveriter.next() may take a long time.
	for levelnum, curlevel in solveriter:
		try:
			x = solver.maxHash
		except:
			solver.maxHash = 2**32
                len_curlevel = len(curlevel)
                countitem += len_curlevel
		if solver.maxHash:
			pct = float(100*countitem)/solver.maxHash
		print "[% 2.4f%%]  Writing level %d, numitems=%d, curitem=%d, maxhash=%d."% \
			(pct,levelnum,len(curlevel),countitem,solver.maxHash)
		maxchunks = 1 + solver.maxHash/CHUNK
		#print "Maximum hash is %d, number of %d-byte chunks to write is %d"%(solver.maxHash, CHUNK, maxchunks)

		mykeys = curlevel.keys()
		mykeys.sort()
		mykeys.append(None) # write the last chunk to disk
		
		lastchunknum = -1
		arr = None
		for position in mykeys:
			if position is None:
				chunknum = -1
			else:
				chunknum = position>>CHUNKBITS
				chunkoff = position & (CHUNK-1)
			if chunknum != lastchunknum:
				if arr:
					maxchunkwritten = max(maxchunkwritten, lastchunknum)
					data[lastchunknum] = (''.join(arr))
				arr = None
				lastchunknum = chunknum
			if position is not None:
				#position = (chunknum<<CHUNKBITS)+chunkoff
				#if position not in curlevel:
				#	continue
				myval = levelnum
				if arr == None:
					if chunknum not in data:
						arr = []
					else:
						arr = [c for c in data[chunknum]]
				if isunreverse:
					#print dict(remoteness=(maxlevel - myval[0]), score=myval[1])
					val = bitClass(remoteness=(maxlevel - myval[0])) #, score=myval[1])
					#print repr(str(val))
				else:
					val = bitClass(remoteness=myval)
				setList(arr, chunkoff, str(val), default)
			
		
		#print "Done with level %d!"%levelnum
	
	print "Solved!  Thank you, writing to disk."
	f = open(filename, "wb")
	pickle.dump({'puzzle': puzzname,
		'options': options,
		'fields': fields,
		'version': 1,
		'maxlevel': solver.get_max_level(),
		'chunkbits': CHUNKBITS}, f)
	chunkbase = (f.tell() + (CHUNK-1)) >> CHUNKBITS
	
	f.close()
	f = open(filename, "r+b") # read/write, no truncate.
	for chunknum in xrange(1 + (solver.maxHash >> CHUNKBITS)):
		if chunknum in data:
			f.seek((chunkbase + (chunknum*bitClass.bytesize))<<CHUNKBITS)
			f.write(data[chunknum])
	print "DONE."
	f.close()
	solveriter.close()
	return solver

solver = None

solve_args = ()

# run with -i to interact with solver.
def run(name, options, unreverse, solverclass):
	global solver
	solver = writeDatabase(name, options, unreverse, solverclass)
	

# run with -i to use the prof variable.
def profile():
	global prof, stats, solve_args
	
	import cProfile, pstats
	prof = cProfile.Profile()
	# *solveargs is to allow passing options in... can't convert everything to repr.
	prof = prof.runctx("run(*solve_args)", globals(), locals())
	print "Done profiling!"
	stats = pstats.Stats(prof)
	stats.sort_stats("time")
	stats.print_stats(80)

if __name__=='__main__':
	#writeDatabase('Wheel',
	#	{'seqSize': 6})
	if len(sys.argv)<2:
		print >>sys.stderr, "Arguments: [-u] [-o] [-p] PythonClass option1 value1 option2 value2 ..."
		print >>sys.stderr, "\t-u: Unreverse solver"
		print >>sys.stderr, "\t-o: Old and slow solver for 'legacy' puzzles (including FCG)"
		print >>sys.stderr, "\t-p: Enable solver profiling"
		print >>sys.stderr, "\tto determine allowed options, look at default_options in PythonClass.py"
		sys.exit(0)
	args = sys.argv[1:]
	unreverse=False
	profiling=False
	solverclass = ThreeLevelSolver.Solver
	if args[0]=="-u":
		solverclass = UnreverseSolver.UnreverseSolver
		unreverse=True
		args=args[1:]
	if args[0]=="-o":
		solverclass = OldSolver.Solver
		args=args[1:]
	if args[0]=="-p":
		profiling = True
		args=args[1:]
	
	options = {}
	for i in xrange(1,len(args)-1,2):
		options[args[i]] = args[i+1]
	
	if profiling:
		solve_args = (args[0], options, unreverse, solverclass)
		profile()
	else:
		run(args[0], options, unreverse, solverclass)

