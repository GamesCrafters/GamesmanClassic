import Wheel
import Solver

w = Wheel.Wheel(8,7)
s = Solver.Solver()
s.solve(w)
s.graph()
