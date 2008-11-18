# import cjson
#import gobject
import sys
import random
from DatabaseHelper import *
#
#StandardDB = makeBitClass([('value',16),('remoteness',16)])
#a = StandardDB.create('\x00\x02\x01\x55')
#print hex(a.value)
#print hex(a.remoteness)
#
#print repr(str(StandardDB(**{'value': 5, 'remoteness': 10})))
#

if __name__=='__main__':
	if len(sys.argv)<3:
		print >>sys.stderr, "Arguments: databaseFile, numberOfMovesToScramble"
		sys.exit(0)
	db = OpenDB(sys.argv[1])
	puzz = db.getPuzzle()

	for i in range(int(sys.argv[2])):
		puzz = puzz.do_move(random.choice(puzz.generate_moves()))
	
	print eval("'"+str(puzz)+"'")
	print db.read(puzz)

