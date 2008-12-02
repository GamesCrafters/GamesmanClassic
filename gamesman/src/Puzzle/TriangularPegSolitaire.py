# GamesCrafters Fall 2008
# Puzzle Team
# @author: Roger Tu

from UnreversePuzzle import UnreversePuzzle
import copy

class TriangularPegSolitaire(UnreversePuzzle):
    """This is the Triangular Peg Solitaire Class"""
    
    pegCharacter = 'o'
    blankCharacter = '.'
    
    default_options = {'size': 5, 'start': 0}
    
    @staticmethod
    def unserialize(options = None, str = None):
	tmpBoard = TriangularPegSolitaire()
	tmpBoard.board = []
	row_length = 1
	row = []
	
	if options == None:
	    options = {'size': 5, 'start': 0}
	size = int(options['size'])
	
	if str==None:
		start = int(options['start'])
		str = TriangularPegSolitaire(size=size, start=start).generate_start(size=size,start=start).serialize()
	
	strindex = 0
	print str
	while strindex < len(str):
	    if str[strindex] == TriangularPegSolitaire.pegCharacter:
		row.append(True)
	    elif str[strindex] == TriangularPegSolitaire.blankCharacter:
		row.append(False)
	    elif str[strindex] == ';':
		tmpBoard.board.append(row)
		row = []
	    strindex += 1
	    
	tmpBoard.size = len(tmpBoard.board)
	tmpBoard.start = options['start']
	return tmpBoard

    def __init__(self, size = 5, board = [[False], [True, True], [True, True, True], [True, True, True, True], [True, True, True, True, True]], start = 0):
	self.size = size
	self.board = copy.deepcopy(board)
	self.start = start

    def serialize(self):
	string = ''
	for r in range(len(self.board)):
	    for c in range(len(self.board[r])):
		if self.board[r][c]:
		    string += TriangularPegSolitaire.pegCharacter# + ','
		else:
		    string += TriangularPegSolitaire.blankCharacter# + ','
	    #string = string[:-1]
	    string += ';'
	#string += ':{size: ' + str(self.size) + ', start: ' + str(self.start) + '}'
	return string
    
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
	tmpBoard = copy.deepcopy(self.board)
	for row in range(len(self.board)):
	    for col in range(len(self.board[row])):
		tmpBoard[row][col] = not tmpBoard[row][col]
	return TriangularPegSolitaire(self.size, tmpBoard, self.start)
    
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
		# check south
		if (row < len(self.board) - 2) and \
		   (self.board[row][col]) and \
		   (self.board[row + 1][col]) and not \
		   (self.board[row + 2][col]):
		    moves.append((row, col, row + 1, col, row + 2, col))
		# check west
		if (col > 1) and \
                   (row > 1) and \
		   (self.board[row][col]) and \
		   (self.board[row][col - 1]) and not \
		   (self.board[row][col - 2]):
		    moves.append((row, col, row, col - 1, row, col - 2))
		# check north
		if (row > 1) and \
                   (col < len(self.board[row]) - 2) and \
		   (self.board[row][col]) and \
		   (self.board[row - 1][col]) and not \
		   (self.board[row - 2][col]):
		    moves.append((row, col, row - 1, col, row - 2, col))
		# check north west
		if (col > 1) and \
		   (row > 1) and \
		   (self.board[row][col]) and \
		   (self.board[row - 1][col - 1]) and not \
		   (self.board[row - 2][col - 2]):
		    moves.append((row, col, row - 1, col - 1, row - 2, col - 2))
		# check south east
		if (row < len(self.board) - 2) and \
		   (self.board[row][col]) and \
		   (self.board[row + 1][col + 1]) and not \
		   (self.board[row + 2][col + 2]):
		    moves.append((row, col, row + 1, col + 1, row + 2, col + 2))
        return moves
    
    def do_move(self, move):
        tmpBoard = copy.deepcopy(self.board)
	tmpBoard[move[0]][move[1]] = False
	tmpBoard[move[2]][move[3]] = False
	tmpBoard[move[4]][move[5]] = True
        return TriangularPegSolitaire(self.size, tmpBoard, self.start)
    
    def undo_move(self, move):
	tmpBoard = copy.deepcopy(self.board)
	tmpBoard[move[0]][move[1]] = True
	tmpBoard[move[2]][move[3]] = True
	tmpBoard[move[4]][move[5]] = False
        return TriangularPegSolitaire(self.size, tmpBoard, self.start)

    def __add__(self, move):
        return self.do_move(move)

    def __sub__(self, move):
	return self.undo_move(move)
    
    def is_a_solution(self):
        return self in self.generate_solutions()
    '''
    def is_a_solution(self):
        s = 0
        for row in range(len(self.board)):
            for col in range(len(self.board[row])):
                s += self.board[row][col]
        return (s == 1)
    '''
    def is_illegal(self):
        return False
    
    def is_deadend(self):
        return (not self.is_a_solution()) and (self.is_leaf())

    def is_leaf(self):
        return (len(self.generate_moves()) == 0)

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
	tmpBoard = copy.deepcopy(self.board)
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
