from Puzzle import *
import string

# 11022480
#Level 0 : 1       | 
#Level 1 : 9       | 9
#Level 2 : 54      | 6
#Level 3 : 321     | 5.94
#Level 4 : 1847    | 5.75
#Level 5 : 9992    | 5.41
#Level 6 : 50135   | 5.01
#Level 7 : 227510  | 4.53
#Level 8 : 869715  | 3.82
#Level 9 : 1885639 | 2.16

class Rubik(Puzzle):
    """This is the 2x2x2 Rubiks Cube puzzle class"""
    
    #We want to be color scheme independent, so we're going to identify colors
    #from their face. Standard notation is F for front, U=up, R=right L=left, B=back, D=down
    #Corners are numbered as in the following diagram
    
    #         +-----+
    #         |3 U 2|
    #         |1   0|
    #         +-----+
    # +-----+ +-----+ +-----+ +-----+
    # |3 L 1| |1 F 0| |0 R 2| |2 B 3|
    # |7   5| |5   4| |4   6| |6   7|
    # +-----+ +-----+ +-----+ +-----+
    #         +-----+
    #         |5 D 4|
    #         |7   6|
    #         +-----+
    
    #The cube will be represented as an array of 8 numbers, where each number is N*3 + T,
    #where N is the number of piece (as in the above diagram), and T is the
    #number of turns clockwise necessary to orient the piece. A piece is oriented
    #if it has either the U or D color on U or D.
    def is_oriented(self, c):
        return c == "U" or c == "D"
    
    #Each face's stickers are defined in the following order:
    # +---+---+
    # |_3_|_2_|
    # | 1 | 0 |
    # +---+---+
    values = { "F" : 0, "U" : 0, "R" : 0, "L" : 1, "B" : 2, "D" : 4}
    """TODO - deal with corner 7 not being correct"""
    #This method exists because python does not provide a way to have multiple constructors
    def initializeCubeWithStickers(self, F="FFFF", U="UUUU", R="RRRR", L="LLLL", B="BBBB", D="DDDD"):
        self.pieces = []
        orientation = 0
        for c in range(8):
            up = c < 4
            s0 = eval("DU"[up])[c % 4]                  # this sticker is facing up or down
            s2 = eval("RFBLFLRB"[c])[3 * up]            # this sticker is clockwise of s0
            s1 = eval("RFBLFLRB"[(c + 4) % 8])[1 + up]  # this sticker is counter clockwise of s0
            corner = [  ]
            #TODO - check if is valid and unvisited corner
            n = self.values[s0] + self.values[s1] + self.values[s2]
            t = 1*self.is_oriented(s1) + 2*self.is_oriented(s2)
            orientation += t
            self.pieces.append(3*n + t)
        
        if orientation % 3 != 0:
            raise "Invalid corner orientation"
        
    def __init__(self, pieces=[3*n for n in range(8)]):
        self.pieces = pieces
        Puzzle(True)

    def generate_solutions(self):
        return [Rubik()]   # Default _is_ solution; what else?
    
    #We're anchoring the BLD corner, so legal moves are F, U, R (and their multiples)
    #F means rotate front face clockwise 90 degrees, F' means 90 degrees counterclockwise, F2 means 180 degrees
    def generate_moves(self):
        return ["F", "F2", "F'", "U", "U'", "U2", "R", "R'", "R2"]

    #this returns the value of the corner after being twisted turnsCW
    def twist_cw(self, corner, turnsCW):
        return 3 * (corner / 3) + (corner - turnsCW) % 3
    
    def do_move(self, move):
        times = self.turns.index(move[1:])
        pieces = list(self.pieces)
        for i in range(times):
            if move[0] == 'F':
                pieces[0], pieces[4], pieces[5], pieces[1] = pieces[1], pieces[0], pieces[4], pieces[5]
                pieces[0] = self.twist_cw(pieces[0], 1)
                pieces[1] = self.twist_cw(pieces[1], 2)
                pieces[4] = self.twist_cw(pieces[4], 2)
                pieces[5] = self.twist_cw(pieces[5], 1)
            elif move[0] == 'U':
                pieces[0], pieces[1], pieces[3], pieces[2] = pieces[2], pieces[0], pieces[1], pieces[3]
            elif move[0] == 'R':
                pieces[0], pieces[2], pieces[6], pieces[4] = pieces[4], pieces[0], pieces[2], pieces[6]
                pieces[0] = self.twist_cw(pieces[0], 2)
                pieces[2] = self.twist_cw(pieces[2], 1)
                pieces[4] = self.twist_cw(pieces[4], 1)
                pieces[6] = self.twist_cw(pieces[6], 2)
            else:
                raise "BadMoveError", move + " is not one of [FUR][ '2]"
        return Rubik(pieces)

    turns = [ None, "", "2", "'" ]
    def reverse_move(self, move):
        return move[0] + self.turns[4 - self.turns.index(move[1:])]

    """TODO: Ethan, this is all you"""
    def __str__(self):
        #return self.T+'|'+self.R+'|'+self.L+'|'+self.BT+'|'+self.BR+'|'+self.BL
        solved_cube = (("U","R","F"),("U","F","L"),("U","B","R"),("U","L","B"),("D","F","R"),("D","L","F"),("D","R","B"),("D","B","L")) # solved pieces are how each cube (as represented in the picture above) would look going clockwise
        pieces = list(self.pieces)
        current_state = []
        for x in range(len(pieces)):
            current_chunk = []
            real_piece = pieces[x] / 3
            rotations_from_orientation = pieces[x] % 3
            current_chunk.append(solved_cube[real_piece][(0-rotations_from_orientation)]) #top piece
            current_chunk.append(solved_cube[real_piece][(1-rotations_from_orientation)]) #right piece
            current_chunk.append(solved_cube[real_piece][(2-rotations_from_orientation)]) #left piece
            current_state.append(current_chunk) # This is the stickers on piece[x] arranged clockwise
        cube_string = "         +-----+\n"
        cube_string += "         |"+current_state[3][0]+" U "+current_state[2][0]+"|\n"
        cube_string += "         |"+current_state[1][0]+"   "+current_state[0][0]+"|\n"
        cube_string += "         +-----+\n"
        cube_string += " +-----+ +-----+ +-----+ +-----+\n"
        cube_string += " |"+current_state[3][1]+" L "+current_state[1][2]+"| |"+current_state[1][1]+" F "+current_state[0][2]+"| |"+current_state[0][1]+" R "+current_state[2][2]+"| |"+current_state[2][1]+" B "+current_state[3][2]+"|\n"
        cube_string += " |"+current_state[7][2]+"   "+current_state[5][1]+"| |"+current_state[5][2]+"   "+current_state[4][1]+"| |"+current_state[4][2]+"   "+current_state[6][1]+"| |"+current_state[6][2]+"   "+current_state[7][1]+"|\n"
        cube_string += " +-----+ +-----+ +-----+ +-----+\n"
        cube_string += "         +-----+\n"
        cube_string += "         |"+current_state[5][0]+" D "+current_state[4][0]+"|\n"
        cube_string += "         |"+current_state[7][0]+"   "+current_state[6][0]+"|\n"
        cube_string += "         +-----+\n"
        return cube_string
    
    def __hash__(self):
        hash = 0
        for i, piece in enumerate(self.pieces):
            hash |= piece << (5 * i)
        return hash
