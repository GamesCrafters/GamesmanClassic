import Solver
import Wheel

print
for seqSize in range(4,9):
	for rotSize in range(2, seqSize):
		s = Solver.Solver()
		w = Wheel.Wheel(seqSize, rotSize)
		s.solve(w)
		print "seqSize = ", seqSize, "; rotSize = ", rotSize
		print "max level   = ", s.get_max_level()
		print "reachables  = ", s.get_reachables()
		print "permuations = ", s.get_permutations(w)
		print "ratio       = ", s.get_ratio(w)
		print