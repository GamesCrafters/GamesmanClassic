class PegSolitaire:
    """This is the Peg Solitaire Class (Triangular 15)"""

    def __init__(self, board = [[False], [True, True], [True, True, True], [True, True, True, True], [True, True, True, True, True]]):
	if isinstance(board, list):
	    self.board = board
	elif isinstance(board, int):
	    if board > 2:
		self.board = [[False]]
		row = 2
		board -= 1
		while board > 0:
		    foo = row
		    bar = []
		    while foo > 0 and board > 0:
			bar.append(True)
			foo -= 1
			board -= 1
		    self.board.append(bar)
		    row += 1
	    else:
		self.board = [[False]]
        #self.size = 15

    def generate_start(self):
        return PegSolitaire()

    def generate_solutions(self):
        solutions = []
        for power in range(self.size):
            tmp = PegSolitaire()
            tmp.board = 2 ** power
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
        return PegSolitaire(tmpBoard)

    def __add__(self, move):
        return self.do_move(move)

    def __sub__(self, move):
	return self.undo_move(move)
    
    def is_a_solution(self):
        return self in self.generate_solutions()

    def is_illegal(self):
	foo += (bar for bar in [baz.count(True) for baz in self.board])
	if foo == 15 or foo == 0:
	    return True
	elif foo == 14 and self.board[2][1]:
	    return True
        return False

    def reverse_move(self, move):
	raise '''Cannot reverse move'''

    def undo_move(self, move):
	tmpBoard = self.board[:]
	tmpBoard[move[0]][move[1]] = True
	tmpBoard[move[2]][move[3]] = True
	tmpBoard[move[4]][move[5]] = False
        return PegSolitaire(tmpBoard)
    
    def get_permutations(self):
	return 0

    def get_score(self):
	return self.__str__().count('.')
    
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
		#index -= 1
		if ((hash >> index) & 1) == 1:
		    tmpBoard[row][col] = True
		else:
		    tmpBoard[row][col] = False
		index += 1
	return PegSolitaire(tmpBoard)

    def __str__(self):
	str = '\n'
	for row in range(len(self.board)):
	    str += ' ' * (len(self.board) - row - 1)
	    for col in range(len(self.board[row])):
		if self.board[row][col]:
		    str += 'o '
		else:
		    str += '. '
	    str += '\n'
        return str
