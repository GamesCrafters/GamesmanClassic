from PegSolitaire import *

# make a default board
puz = PegSolitaire()
print puz

# get some moves
foo = puz.generate_moves()
print foo

# make the first one
puz += foo[0]
print puz
#print [x.count(True) for x in puz.board]

# testing hash
print puz.__hash__()

# testing reverse move
puz -= foo[0]
print puz

# test unhash
bar = 32631
puz = puz.unhash(bar)
print puz
bar = 1
puz = puz.unhash(bar)
print puz