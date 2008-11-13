# GamesCrafters Fall 2008
# Puzzle Team
# @author: Roger Tu

from TriangularPegSolitaire import *

print 'Triangular Peg Solitaire Tester v0.1'

# make a default board
puz = TriangularPegSolitaire()
print puz

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
puz = TriangularPegSolitaire(16)
print puz
puz = TriangularPegSolitaire(14)
print puz
puz = TriangularPegSolitaire(45)
print puz
print puz.__hash__()