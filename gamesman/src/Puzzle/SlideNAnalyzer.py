import Solver
import SlideN

print
for size in range(2, 4):
	s = Solver.Solver()
	w = SlideN.SlideN(size)
	s.solve(w)
	print "size =", size
	print "max level   =", s.get_max_level()
	print "reachables  =", s.get_reachables()
	print "permuations =", s.get_permutations(w)
	print "ratio       =", s.get_ratio(w)
	print