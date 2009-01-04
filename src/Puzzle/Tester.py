### python2.5 -i Tester.py

from Solver import *
from Rubik import *
from FCG import *
from Cannibals import *
from Donkey import *
from Wheel import *

s = Solver()
#puzzle = FCG()
#puzzle = Cannibals()
#puzzle = Rubik()
#puzzle = Donkey()
puzzle = Wheel(6)
#print puzzle
p = puzzle

s.solve(puzzle)
#s.solve(puzzle, max_level = 10)
#s.solve(puzzle,True)

s.graph() 
#s.path(puzzle)

'''
p = Donkey(B=(1,2), V=((0,1),(1,0),(3,0),(4,0)), H = ((3,2),(3,3)), S = ((2,0),(2,1)), E = ((0,0),(0,3)))
print p

print "ORIGINAL POSITION\n" + str(p)

#moves = [2,1,2,2,0,1,1,0,1,3,0,1,1]
#moves = [2,1,2,2,0,1,1,0,1,3]
moves = []

for move in moves:
    p += p.generate_moves()[move]
    print "CHILD POSITION\n" + str(p)

ms = p.generate_moves()
print "AVAILABLE MOVES: " + str(ms)
for m in ms:
    print "MAKING MOVE: " + m + " (reversed is " + p.reverse_move(m) + ")"
    print p+m
'''


'''
--BB|
--BB|
||o |
|| o|
SD0
'''
