from Puzzle import *
import string

UDLR_TO_DELTA = {'U':(0,1), 'D':(0,-1), 'L':(-1,0), 'R':(1,0)}
EMPTY_DELTA = {"BU":(0,-2), "BD":(0,2), "BL":(2,0), "BR":(-2,0), \
               "HU":(0,-1), "HD":(0,1), "VL":(1,0), "VR":(-1,0), \
               "VU":(0,-1), "VD":(0,1), "HL":(1,0), "HR":(-1,0), \
               "SU":(0,-1), "SD":(0,1), "SL":(1,0), "SR":(-1,0)}
NEIGHBOR_DELTA = {"VD":(0,-1), "VU":(0,2), "HL":(-1,0), "HR":(2,0), \
                  "SD":(0,-1), "SU":(0,1), "SL":(-1,0), "SR":(1,0)}
FLIP_MOVE = {'D':'U', 'U':'D', 'L':'R', 'R':'L'}

# Checks for equality of pieces
def can_move_both(pieceA, pieceB, emptys, delta):
    return ((pieceA[0]+delta[0][0], pieceA[1]+delta[0][1]) == emptys[0] and \
            (pieceB[0]+delta[1][0], pieceB[1]+delta[1][1]) == emptys[1]) or \
           ((pieceA[0]+delta[0][0], pieceA[1]+delta[0][1]) == emptys[1] and \
            (pieceB[0]+delta[1][0], pieceB[1]+delta[1][1]) == emptys[0])

# Checks for equality of pieces
def can_move(piece, emptys, delta):
    return can_move_this_empty(piece, emptys[0], delta) or \
           can_move_this_empty(piece, emptys[1], delta) 

def can_move_this_empty(piece, empty, delta):
        return (piece[0]+delta[0], piece[1]+delta[1]) == empty

# Take the (x,y) location of a piece and return the 0-19 index
def linear(piece):
    return piece[1]*5 + piece[0] # (0-3)*5+(0-4) = 0-19

def shift(piece, udlr):
    global UDLR_TO_DELTA
    return (piece[0]+UDLR_TO_DELTA[udlr][0], piece[1]+UDLR_TO_DELTA[udlr][1])

def double_shift_empty(emptys, move2):
    global EMPTY_DELTA
    return ((emptys[0][0] + EMPTY_DELTA[move2][0], \
             emptys[0][1] + EMPTY_DELTA[move2][1]), \
            (emptys[1][0] + EMPTY_DELTA[move2][0], \
             emptys[1][1] + EMPTY_DELTA[move2][1]))

def single_shift_empty(piece, emptys, move2, scale, puzzle):
    # at this point we know the piece, but NOT what empty (which do we update?)
    # because the move is something like "SD0" which says to move the 0th small
    # one down, but what empty does it replace? We have to figure out which one
    # The arg is 'move2' because it's only the first two chars of the move
    global EMPTY_DELTA
    global NEIGHBOR_DELTA
    #print str(piece), str(emptys), str(move2), str(scale)

    if can_move_this_empty(piece, emptys[0], NEIGHBOR_DELTA[move2]):
        return ((emptys[0][0] + scale * EMPTY_DELTA[move2][0], \
                 emptys[0][1] + scale * EMPTY_DELTA[move2][1]), \
                emptys[1])
    elif can_move_this_empty(piece, emptys[1], NEIGHBOR_DELTA[move2]):
        return (emptys[0], \
                (emptys[1][0] + scale * EMPTY_DELTA[move2][0], \
                 emptys[1][1] + scale * EMPTY_DELTA[move2][1]))
    else:
        raise "single_shift_empty_BadElse_error", "Move should match ONE of emptys" + str(piece) +  str(emptys) +  str(move2) +  str(scale) + "\n" + str(puzzle)

class Donkey(Puzzle):
    """This is the Donkey puzzle class"""

    def __init__(self, B = (3,2), V = ((0,0),(1,0),(3,0),(4,0)), \
                 H = ((0,2),(0,3)), S = ((2,2),(2,3)), E = ((2,0),(2,1)) ):
        if linear(H[0]) > linear(H[1]):
            H = (H[1], H[0])
        if linear(S[0]) > linear(S[1]):
            S = (S[1], S[0])
        if linear(E[0]) > linear(E[1]):
            E = (E[1], E[0])

        # sort V 
        tmp = map(lambda elt: (linear(elt), elt), V) # cons linear num to front
        tmp.sort()                                   # sort by it (linear num)
        V = tuple(map(lambda elt:elt[1], tmp))       # reassign to sorted list

        self.B, self.V, self.H, self.S, self.E = B, V, H, S, E

        Puzzle(False) # Multiple solutions!

    # 4#######
    # 3#--oBB#  56789  Reference pieces by lower-left corner
    # 2#--oBB#  01234  AND reference similar pieces by row-major
    # 1#|| ||#  56789
    # 0#|| ||#  01234
    #-1#######
    # -1012345
    def generate_start(self):
        return Donkey() # default _is_ start

#    def generate_solutions(self):
#        return [Donkey(B=(0,2),V=((0,0),(1,0),(3,0),(4,0)),H=((3,2),(3,3)),S=((2,0),(2,1)),E=((2,2),(2,3)))]

    def is_a_solution(self):
        return self.B == (0,2) # Upper-left corner for B
    
    def generate_moves(self):
        moves = []          # Can always take nothing

        if can_move_both(self.B, self.B, self.E, ((-1,0), (-1,1)) ):
            moves.append('BL')
        if can_move_both(self.B, self.B, self.E, ((2,0), (2,1)) ):
            moves.append('BR')
        if can_move_both(self.B, self.B, self.E, ((0,-1), (1,-1)) ):
            moves.append('BD')
        if can_move_both(self.B, self.B, self.E, ((0,2), (1,2)) ):
            moves.append('BU')
        for i in range(4):
            if can_move_both(self.V[i], self.V[i], self.E, ((-1,0), (-1,1)) ):
                moves.append('VL' + str(i))
            if can_move_both(self.V[i], self.V[i], self.E, ((1,0), (1,1)) ):
                moves.append('VR' + str(i))
        for i in range(2):
            if can_move_both(self.H[i], self.H[i], self.E, ((0,-1), (1,-1)) ):
                moves.append('HD' + str(i))
            if can_move_both(self.H[i], self.H[i], self.E, ((0,1), (1,1)) ):
                moves.append('HU' + str(i))
        for i in range(4):
            if can_move(self.V[i], self.E, (0,-1)):
                moves.append('VD' + str(i))
            if can_move(self.V[i], self.E, (0,2)):
                moves.append('VU' + str(i))
        for i in range(2):
            if can_move(self.H[i], self.E, (-1,0)):
                moves.append('HL' + str(i))
            if can_move(self.H[i], self.E, (2,0)):
                moves.append('HR' + str(i))
        for i in range(2):
            if can_move(self.S[i], self.E, (-1,0)):
                moves.append('SL' + str(i))
            if can_move(self.S[i], self.E, (1,0)):
                moves.append('SR' + str(i))
            if can_move(self.S[i], self.E, (0,-1)):
                moves.append('SD' + str(i))
            if can_move(self.S[i], self.E, (0,1)):
                moves.append('SU' + str(i))

        return moves

    def do_move(self, move):
        B, V, H, S, E = list(self.B), list(self.V), list(self.H), list(self.S), list(self.E)

        if move[0] == 'B':
            B = shift(B, move[1])
            E = double_shift_empty(E, move)
        elif (move[0] == 'V' and (move[1] == 'L' or move[1] == 'R')):
            V[int(move[2])] = shift(V[int(move[2])], move[1])
            E = double_shift_empty(E, move[0:2])
        elif (move[0] == 'H' and (move[1] == 'U' or move[1] == 'D')):
            H[int(move[2])] = shift(H[int(move[2])], move[1])
            E = double_shift_empty(E, move[0:2])
        elif move[0] == 'V':
            piece = V[int(move[2])]
            V[int(move[2])] = shift(V[int(move[2])], move[1])
            E = single_shift_empty(piece, E, move[0:2], 2, self)
        elif move[0] == 'H':
            piece = H[int(move[2])]
            H[int(move[2])] = shift(H[int(move[2])], move[1])
            E = single_shift_empty(piece, E, move[0:2], 2, self)
        elif move[0] == 'S':
            piece = S[int(move[2])]
            S[int(move[2])] = shift(S[int(move[2])], move[1])
            E = single_shift_empty(piece, E, move[0:2], 1, self)
        else:
            raise "do_move_BadElse_error", "Move[0] should be one of [BVHS] but was" + move[0]

        return Donkey(tuple(B),tuple(V),tuple(H),tuple(S),tuple(E)) # Make a new position and return it
            
    def reverse_move_broken(self, move):
        #This doesn't work because the numbers can get reordered after a move
        global FLIP_MOVE
        return move[0] + FLIP_MOVE[move[1]] + move[2:]

    def reverse_move(self, move):
        parent = self + move
        for new_move in parent.generate_moves():
            if parent + new_move == self:
                return new_move

    def __str__(self):

        # Make a Grid with all the pieces there
        G = {}

        for x in range(-1,6):
            G[(x,-1)] = '#'
            G[(x,4)]  = '#'
        for y in range(4):
            G[(-1,y)] = '#'
            G[(5,y)]  = '#'

        G[self.B] = '#'
        G[(self.B[0],self.B[1]+1)] = '#'
        G[(self.B[0]+1,self.B[1])] = '#'
        G[(self.B[0]+1,self.B[1]+1)] = '#'
        for i in range(4):
            G[self.V[i]] = '|'
            G[(self.V[i][0],self.V[i][1]+1)] = '|'
        for i in range(2):
            G[self.H[i]] = '-'
            G[(self.H[i][0]+1,self.H[i][1])] = '-'
        for i in range(2):
            G[self.S[i]] = 'o'
        for i in range(2):
            G[self.E[i]] = ' '

        str = ""
        for y in [3,2,1,0]:
            for x in range(5):
                str += G[(x,y)]
            if y:
                str += '\\n'

        return str

    def __hash__(self):
        B  = self.B[1] * 4 + self.B[0]       # 0-11
        V0 = self.V[0][1] * 5 + self.V[0][0] # 0-14
        V1 = self.V[1][1] * 5 + self.V[1][0] # 0-14
        V2 = self.V[2][1] * 5 + self.V[2][0] # 0-14
        V3 = self.V[3][1] * 5 + self.V[3][0] # 0-14
        H0 = self.H[0][1] * 4 + self.H[0][0] # 0-15
        H1 = self.H[1][1] * 4 + self.H[1][0] # 0-15
        E0 = linear(self.E[0])
        E1 = linear(self.E[1])
        S0 = linear(self.S[0])
        S1 = linear(self.S[1])
        if   E1 > E0 > S1 > S0: spaces = 0
        elif E1 > S1 > E0 > S0: spaces = 1
        elif E1 > S1 > S0 > E0: spaces = 2
        elif S1 > E1 > E0 > S0: spaces = 3
        elif S1 > E1 > S0 > E0: spaces = 4
        elif S1 > S0 > E1 > E0: spaces = 5
        else:
            raise 'BadOrderingError', "The order of spaces and emptys not expected"

        # spc|-H1-|-H0-|-V3-|-V2-|-V1-|-V0-|--B-|
        return (spaces << 28) + \
               (H1 << 24) + \
               (H0 << 20) + \
               (V3 << 16) + \
               (V2 << 12) + \
               (V1 << 8) + \
               (V0 << 4) + \
               B 
