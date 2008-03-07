from Puzzle import *
import string
import random
import copy

class Wheel(Puzzle):
	seqSize = 0
	rotSize = 0
	sequence = ()
	def __init__(self, seqSize = 20, rotSize = 4):
		if (rotSize > 1) and (seqSize > rotSize):	
			self.seqSize = seqSize
			self.rotSize = rotSize
			self.sequence = tuple(range(1, self.seqSize + 1))
			Puzzle(True) # 1, 2, 3, ... 20 are in order
	
	def generate_solutions(self):
		return [Wheel(self.seqSize, self.rotSize)] # init is the solution
    
	def generate_start(self, sequence = None):
		if sequence and (len(sequence) == self.seqSize):
			newWheel = Wheel(self.seqSize, self.rotSize)
			newWheel.sequence = tuple(sequence)
			return newWheel
		else:
			return None
			
	def shuffle(self, times = 100):
		newWheel = copy.copy(self)
		for i in range(times):
			move = random.randint(1, newWheel.seqSize)
			newWheel = newWheel.do_move(move)
			newWheel = newWheel.do_move(0)
		return newWheel
    
	def generate_moves(self):
		moves = []
		for i in range(self.seqSize):
			moves += [i]
		return moves

	def do_move(self, move):
		old_seq = list(self.sequence)
		new_seq = []
		if move == 0:
			new_seq += old_seq[0:self.rotSize]
			new_seq.reverse() 
			new_seq += old_seq[self.rotSize:]
			return self.generate_start(new_seq)
		else:
			new_seq += old_seq[move:]
			new_seq += old_seq[0:move]
			return self.generate_start(new_seq)
	
	def reverse_move(self, move):
		if move == 0:
			return move
		else:
			return self.seqSize - move
            
	def is_illegal(self):
		return False
		
	def get_permutations(self):
		prod = 1
		for i in range(self.seqSize):
			prod *= (1 + i)
		return prod
	
	def __str__(self):
		s = ''
		for i in self.sequence:
			s += str(i)
		return s
	
	def __hash__(self):
		h = 0
		fact = 1
		seq = list(self.sequence)
		indices = []
		for num in range(1, self.seqSize + 1):
			indices.append(seq.index(num))
		for i in range(1, self.seqSize):
			fact *= i
			sum = 0
			for j in range(0, i):
				sum += int(indices[j] > indices[i])
			h += fact * sum
		return h
