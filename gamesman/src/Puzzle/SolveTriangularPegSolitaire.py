from Puzzle import Puzzle
from TriangularPegSolitaire import TriangularPegSolitaire
from UnreverseSolver import *
from Solver import *

solv = UnreverseSolver()
tps = TriangularPegSolitaire()
tps = tps.generate_start(3, 0)
solv.solve(tps, True)