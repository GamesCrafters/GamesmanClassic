# GamesCrafters Fall 2008
# Puzzle Team
# @author: Roger Tu

from Puzzle import Puzzle

class TriangularPegSolitaire(Puzzle):
    """This is the Triangular Peg Solitaire Class"""
    
    pegCharacter = 'o'
    blankCharacter = '.'

    def __init__(self, size = 5, board = [[False], [True, True], [True, True, True], [True, True, True, True], [True, True, True, True, True]], start = 0):
	self.size = size
	self.board = board[:]
	self.start = start
    '''
    def copy(self, board = self.board):
	tmp = TriangularPegSolitaire(self.size, board, self.start)
	return tmp
	'''
    def generate_start(self, size = 5, start = 0):
	tmp = TriangularPegSolitaire()
	if size < 1:
	    tmp.board = [[False], [True, True], [True, True, True], [True, True, True, True], [True, True, True, True, True]]
	    tmp.size = 5
	    tmp.start = 0
	else:
	    tmp.size = size
	    board = []
	    row_size = 1
	    if self.__triangular__() <= start or start < 0:
		start = 0
	    while row_size <= size:
		row = []
		for r in range(row_size):
		    row.append(True if (start != 0) else False)
		    start -= 1
		board.append(row)
		row_size += 1
	    tmp.board = board
        return tmp

    def __flip__(self):
	tmp = self.board[:]
	for row in range(len(self.board)):
	    for col in range(len(self.board[row])):
		tmp[row][col] = not tmp[row][col]
	return TriangularPegSolitaire(self.size, tmp, self.start)
    
    def __triangular__(self):
	return (self.size * (self.size + 1) / 2)
    
    def generate_solutions(self):
        solutions = []
	for position in range(self.__triangular__()):
	    tmp = self.generate_start(self.size, position)
	    tmp = tmp.__flip__()
	    solutions.append(tmp)
        return solutions
    
    def generate_moves(self):
        moves = []
        for row in range(len(self.board)):
            for col in range(len(self.board[row])):
		# check east
		if (col < len(self.board[row]) - 2) and \
		   (self.board[row][col]) and \
		   (self.board[row][col + 1]) and not \
		   (self.board[row][col + 2]):
		    moves.append((row, col, row, col + 1, row, col + 2))
		# check north east
		if (col < len(self.board[row]) - 2) and \
		   (row > 1) and \
		   (self.board[row][col]) and \
		   (self.board[row - 1][col]) and not \
		   (self.board[row - 2][col]):
		    moves.append((row, col, row - 1, col, row - 2, col))
		# check south east
		if (col < len(self.board[row]) - 2) and \
		   (row < len(self.board) - 2) and \
		   (self.board[row][col]) and \
		   (self.board[row][col + 1]) and not \
		   (self.board[row][col + 2]):
		    moves.append((row, col, row, col + 1, row, col + 2))
		# check west
		if (col > 1) and \
		   (self.board[row][col]) and \
		   (self.board[row][col - 1]) and not \
		   (self.board[row][col - 2]):
		    moves.append((row, col, row, col - 1, row, col - 2))
		# check north west
		if (col > 1) and \
		   (row > 1) and \
		   (self.board[row][col]) and \
		   (self.board[row - 1][col - 1]) and not\
		   (self.board[row - 2][col - 2]):
		    moves.append((row, col, row - 1, col - 1, row - 2, col - 2))
		# check south west
		if (col > 1) and\
		   (row < len(self.board) - 2) and\
		   (self.board[row][col]) and\
		   (self.board[row + 1][col - 1]) and not\
		   (self.board[row + 2][col - 2]):
		    moves.append((row, col, row + 1, col - 1, row + 2, col - 2))
        return moves

    def do_move(self, move):
	tmpBoard = self.board[:]
	tmpBoard[move[0]][move[1]] = False
	tmpBoard[move[2]][move[3]] = False
	tmpBoard[move[4]][move[5]] = True
	#copy(tmpBoard)
        return TriangularPegSolitaire(self.size, tmpBoard, self.start)
    
    def undo_move(self, move):
	tmpBoard = self.board[:]
	tmpBoard[move[0]][move[1]] = True
	tmpBoard[move[2]][move[3]] = True
	tmpBoard[move[4]][move[5]] = False
	#copy(tmpBoard)
        return TriangularPegSolitaire(self.size, tmpBoard, self.start)

    def __add__(self, move):
        return self.do_move(move)

    def __sub__(self, move):
	return self.undo_move(move)
    
    def is_a_solution(self):
        return self in self.generate_solutions()

    def is_illegal(self):
	foo += (bar for bar in [baz.count(True) for baz in self.board])
	triangular = (self.size) * (self.size + 1) / 2
	if foo == triangular or foo == 0:
	    return True
	elif foo == (triangular - 1) and self.board[0][0]:
	    return True
        return False
    
    def is_deadend(self):
        return (not self.is_a_solution()) and (self.is_leaf())

    def is_leaf(self):
        return (len(self.generate_moves) == 0)

    def reverse_move(self, move):
	raise '''Cannot reverse move'''
    
    def get_permutations(self):
	return None

    def value(self):
	return self.__str__().count(TriangularPegSolitaire.blankCharacter)
    
    def __cmp__(self, other):
        return cmp(hash(self), hash(other))

    def __hash__(self):
	index = 0
	hash = 0
        for row in range(len(self.board)):
	    for col in range(len(self.board[row])):
		if self.board[row][col]:
		    hash |= (1 << index)
		index += 1
	return hash
    
    def unhash(self, hash):
	tmpBoard = self.board[:]
	index = 0
	for row in range(len(self.board)):
	    for col in range(len(self.board[row])):
		if ((hash >> index) & 1) == 1:
		    tmpBoard[row][col] = True
		else:
		    tmpBoard[row][col] = False
		index += 1
	tmp = TriangularPegSolitaire(self.size, tmpBoard, self.start)
        return tmp

    def __str__(self):
	str = '\n'
	for row in range(len(self.board)):
	    str += ' ' * (len(self.board) - row - 1)
	    for col in range(len(self.board[row])):
		if self.board[row][col]:
		    str += TriangularPegSolitaire.pegCharacter + ' '
		else:
		    str += TriangularPegSolitaire.blankCharacter + ' '
	    str += '\n'
        return str
