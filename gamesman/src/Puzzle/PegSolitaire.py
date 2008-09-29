class PegSolitaire:
    """This is the Peg Solitaire Class (Triangular 15)"""

    def __init__(self, board = [[True], [True, True], [True, False, True], [True, True, True, True], [True, True, True, True, True]]):
	self.board = board
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
		    moves.append([row, col, row, col + 1, row, col + 2])
		# check north east
		if (col < len(self.board[row]) - 2) and \
		   (row > 1) and \
		   (self.board[row][col]) and \
		   (self.board[row - 1][col]) and not \
		   (self.board[row - 2][col]):
		    moves.append([row, col, row - 1, col, row - 2, col])
		# check south east
		if (col < len(self.board[row]) - 2) and \
		   (row < len(self.board) - 2) and \
		   (self.board[row][col]) and \
		   (self.board[row][col + 1]) and not \
		   (self.board[row][col + 2]):
		    moves.append([row, col, row, col + 1, row, col + 2])
		# check west
		if (col > 1) and \
		   (self.board[row][col]) and \
		   (self.board[row][col - 1]) and not \
		   (self.board[row][col - 2]):
		    moves.append([row, col, row, col - 1, row, col - 2])
		# check north west
		if (col > 1) and \
		   (row > 1) and \
		   (self.board[row][col]) and \
		   (self.board[row - 1][col - 1]) and not\
		   (self.board[row - 2][col - 2]):
		    moves.append([row, col, row - 1, col - 1, row - 2, col - 2])
		# check south west
		if (col > 1) and\
		   (row < len(self.board) - 2) and\
		   (self.board[row][col]) and\
		   (self.board[row + 1][col - 1]) and not\
		   (self.board[row + 2][col - 2]):
		    moves.append([row, col, row + 1, col - 1, row + 2, col - 2])
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
	return self.reverse_move(move)
    
    def is_a_solution(self):
        return self in self.generate_solutions()

    def is_illegal(self):
        return False

    def reverse_move(self, move):
	raise 'Cannot reverse move'

    def get_permutations(self):
	return 0

    def __cmp__(self, other):
        return cmp(hash(self), hash(other))

    def __hash__(self):
	index = 15
	hash = 0
        for row in range(len(self.board)):
	    for col in range(len(self.board[row])):
		index -= 1
		if self.board[row][col]:
		    hash |= (1 << index)
	return hash

    def __str__(self):
	str = ''
	for row in range(len(self.board)):
	    str += ' ' * (len(self.board) - row - 1)
	    for col in range(len(self.board[row])):
		if self.board[row][col]:
		    str += 'o '
		else:
		    str += '. '
	    str += '\n'
        return str
