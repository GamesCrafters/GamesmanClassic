from Puzzle import *
import string
import random

class SlideN(Puzzle):
	def __init__(self, size = 4):
		self.size = size
		temp = range(1, size**2)
		temp.append(0)
		self.board = tuple(temp)
		Puzzle(True)
	
	def generate_solutions(self):
		return [SlideN(self.size)]
    
	def generate_start(self, board = None):
		if board and (len(board) == (self.size)**2):
			newSlideN = SlideN(self.size)
			newSlideN.board = tuple(board)
			return newSlideN
		else:
			return None
		
	def shuffle(self, times = 100):
		shuffledSlideN = SlideN(self.size)
		shuffledSlideN.board = self.board
		for i in range(times):
			moves = shuffledSlideN.generate_moves()
			m = random.randint(0, len(moves)-1)
			shuffledSlideN = shuffledSlideN.do_move(moves[m])
		return shuffledSlideN
    
	def generate_moves(self):
		tempboard = list(self.board)
		index = tempboard.index(0)
		moves = ['u', 'd', 'l', 'r']
		(row, col) = self.indextorc(index)
		if row == 0:
			moves.remove('u')
		if col == 0:
			moves.remove('l')
		if row == self.size - 1:
			moves.remove('d')
		if col == self.size - 1:
			moves.remove('r')
		return moves
	
	def indextorc(self, i):
		row = i/self.size
		col = i%self.size
		return (row, col)
	
	def rctoindex(self, (row, col)):
		index = row*self.size + col
		return index
		
	def do_move(self, move):
		tempboard = list(self.board)
		index0 = tempboard.index(0)
		(row0, col0) = self.indextorc(index0)
		if move == 'u':
			(rowS, colS) = (row0-1, col0)
		if move == 'd':
			(rowS, colS) = (row0+1, col0)
		if move == 'l':
			(rowS, colS) = (row0, col0-1)
		if move == 'r':
			(rowS, colS) = (row0, col0+1)
		indexS = self.rctoindex((rowS, colS))
		self.swap(tempboard, index0, indexS)
		movedSlideN = SlideN(self.size)
		movedSlideN.board = tuple(tempboard)
		return movedSlideN
	
	def swap(self, alist, i, j):
		temp = alist[i]
		alist[i] = alist[j]
		alist[j] = temp
	
	def reverse_move(self, move):
		if move == 'u':
			return 'd'
		if move == 'd':
			return 'u'
		if move == 'l':
			return 'r'
		if move == 'r':
			return 'l'
			
	def is_illegal(self):
		return False
	
	def get_permutations(self):
		prod = 1
		for i in range(self.size**2):
			prod *= (1 + i)
		return prod
		
	def __str__(self):
		nl = '\\n'
		s =''
		for i in range(self.size):
			s += (str(self.board[i*self.size:(i+1)*self.size]) + nl)
		return s
	
	def __hash__(self):
		h = 0
		fact = 1
		seq = list(self.board)
		indices = []
		for num in range(0, (self.size)**2):
			indices.append(seq.index(num))
		for i in range(1, (self.size)**2):
			fact *= i
			sum = 0
			for j in range(0, i):
				sum += int(indices[j] > indices[i])
			h += fact * sum
		return h
