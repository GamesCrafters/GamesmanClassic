# GamesCrafters Fall 2008
# Puzzle Team
# @author: Roger Tu

from TriangularPegSolitaire import TriangularPegSolitaire
from UnreverseSolver import *
from Solver import *

print 'Triangular Peg Solitaire Tester v0.2'

# make a default board
puz = TriangularPegSolitaire()
print puz
print 'testing serialization & unserialization'
print '\tserialization of starting board'
print puz.serialize()
print '\tunserialization of a random board'
print TriangularPegSolitaire.unserialize(None, '.;.o;o.o;o.o.;o.ooo;')

# get some moves
foo = puz.generate_moves()
print foo

# make the first one
puz += foo[0]
print puz
#print [x.count(True) for x in puz.board]

# testing hash
print 'testing hash'
print puz.__hash__()
'''
print 'unhash'
print puz.unhash(32757)
puz += foo[0]
foo = puz.generate_moves()
puz += foo[1]
print puz
print puz.__hash__()
print puz.unhash(32487)
'''
# testing reverse move
print 'testing reverse move'
puz -= foo[0]
print puz

# test unhash
print 'testing unhash'
bar = 32757
puz = puz.unhash(bar)
print puz
bar = 1
puz = puz.unhash(bar)
print puz
print puz.value()

# test n size board
print 'testing n-size board'
foo = TriangularPegSolitaire()
puz = foo.generate_start(16, 0)
print puz
puz = foo.generate_start(14, 0)
print puz
puz = foo.generate_start(45)
print puz
print puz.__hash__()
