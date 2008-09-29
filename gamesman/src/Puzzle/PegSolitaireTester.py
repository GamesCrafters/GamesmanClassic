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

# testing hash
print puz.__hash__()

# testing reverse move
puz.reverse_move(foo[0])