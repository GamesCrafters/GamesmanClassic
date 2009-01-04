from Puzzle import Puzzle
from TriangularPegSolitaire import TriangularPegSolitaire
from UnreverseSolver import *
from Solver import *

print 'Solving size 2 start 0'
solv = UnreverseSolver()
tps = TriangularPegSolitaire()
tps = tps.generate_start(2, 0)
solv.solve(tps, True)
solv.save('tps_2_0.txt')

print 'Solving size 3 start 0'
solv = UnreverseSolver()
tps = TriangularPegSolitaire()
tps = tps.generate_start(3, 0)
solv.solve(tps, True)
solv.save('tps_3_0.txt')

print 'Solving size 4 start 0'
solv = UnreverseSolver()
tps = TriangularPegSolitaire()
tps = tps.generate_start(4, 0)
solv.solve(tps, True)
solv.save('tps_4_0.txt')

'''
print 'Solving size 5 start 0'
solv = UnreverseSolver()
tps = TriangularPegSolitaire()
tps = tps.generate_start(5, 0)
solv.solve(tps, True)
solv.save('tps_5_0.txt')
'''