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
    #if it has either the U or D color on the U or D face.
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
        self.orientations = []
        visited = [ False for i in range(8) ]
        for c in range(8):
            up = c < 4
            s0 = eval("DU"[up])[(c - 2*(not up)) % 4]   # this sticker is facing up or down
            s2 = eval("RFBLFLRB"[c])[3 * up]            # this sticker is clockwise of s0
            s1 = eval("RFBLFLRB"[(c + 4) % 8])[1 + up]  # this sticker is counter clockwise of s0
            n = self.values[s0] + self.values[s1] + self.values[s2]
            #checking if we've visited this corner already
            if visited[n]:
                raise "Duplicate corner"
            visited[n] = True
            
            self.pieces.append(n)
            self.orientations.append(1*self.is_oriented(s1) + 2*self.is_oriented(s2))
        
        if sum(self.orientations) % 3 != 0:
            raise "Invalid corner orientation"
        if self.pieces[7] != 7:
            raise "Piece 7 must be in the BLD location"
        if self.orientations[7] != 0:
            raise "Piece 7 must be correctly oriented"

    def __init__(self, pieces = range(8), orientations = [0 for i in range(8)]):
        self.pieces = pieces
        self.orientations = orientations
        if len(pieces) != len(orientations):
            raise "Pieces array different length than orientations array"
        Puzzle(True)
        
    default_options = {}
    @staticmethod
    def unserialize(options, position=None):
        if position==None or position=="[]":
            position = "0,1,2,3,4,5,6,7;0,1,2,3,4,5,6,7"
        pieces, orientations = position.split(";");
        pieces = [int(x) for x in pieces.split(",")]
        orientations = [int(x) for x in orientations.split(",")]
        return Rubik(pieces, orientations)

    def serialize(self):
        return ','.join(str(x) for x in self.pieces) + ";" + ','.join(str(x) for x in self.orientations)

    def generate_solutions(self):
        return [Rubik()]   # Default _is_ solution; what else?
    
    #We're anchoring the BLD corner, so legal moves are F, U, R (and their multiples)
    #F means rotate front face clockwise 90 degrees, F' means 90 degrees counterclockwise, F2 means 180 degrees
    def generate_moves(self):
        return ["F", "F2", "F'", "U", "U'", "U2", "R", "R'", "R2"]

    def cycle_pieces(self, p1, p2, pieces):
        pieces[p1], pieces[p1+4], pieces[p2], pieces[p2-4] = pieces[p2-4], pieces[p1], pieces[p1+4], pieces[p2]
        
    def do_move(self, move):
        times = self.turns.index(move[1:])
        pieces = list(self.pieces)
        orientations = list(self.orientations)
        for i in range(times):
            if move[0] == 'F':
                self.cycle_pieces(0, 5, pieces)
                self.cycle_pieces(0, 5, orientations)
                orientations[0] = (orientations[0] + 2) % 3
                orientations[4] = (orientations[4] + 1) % 3
                orientations[5] = (orientations[5] + 2) % 3
                orientations[1] = (orientations[1] + 1) % 3
            elif move[0] == 'U':
                pieces[0], pieces[1], pieces[3], pieces[2] = pieces[2], pieces[0], pieces[1], pieces[3]
            elif move[0] == 'R':
                self.cycle_pieces(2, 4, pieces)
                self.cycle_pieces(2, 4, orientations)
                orientations[2] = (orientations[2] + 2) % 3
                orientations[6] = (orientations[6] + 1) % 3
                orientations[4] = (orientations[4] + 2) % 3
                orientations[0] = (orientations[0] + 1) % 3
            else:
                raise "BadMoveError", move + " is not one of [FUR][ '2]"
        return Rubik(pieces, orientations)

    turns = [ None, "", "2", "'" ]
    def reverse_move(self, move):
        return move[0] + self.turns[4 - self.turns.index(move[1:])]

    dotty_color_scheme = { "U" : "white", "F" : "red", "R" : "blue", "B" : "orange", "L" : "green", "D" : "yellow" }
    console_color_scheme = { "U" : "1;29", "F" : "0;31", "R" : "0;36", "B" : "1;31", "L" : "0;32", "D" : "0;33" }
    
#<lenox> Black       0;30     Dark Gray     1;30
#<lenox> Blue        0;34     Light Blue    1;34
#<lenox> Green       0;32     Light Green   1;32
#<lenox> Cyan        0;36     Light Cyan    1;36
#<lenox> Red         0;31     Light Red     1;31
#<lenox> Purple      0;35     Light Purple  1;35
#<lenox> Brown       0;33     Yellow        1;33
#<lenox> Light Gray  0;37     White         1;37
#<lenox> Other codes available include 4: Underscore, 5: Blink, 7: = Inverse, and 8: Concealed.
    
    def __str__(self):
        nl = '\n'
        import inspect
        graphing = inspect.stack()[1][3] == "graph"
        if graphing:
            nl = '<br align="left" />' #for dotty (\l for left alignment!)
            
        solved_cube = (("U","R","F"),("U","F","L"),("U","B","R"),("U","L","B"),("D","F","R"),("D","L","F"),("D","R","B"),("D","B","L")) # solved pieces are how each cube (as represented in the picture above) would look going clockwise
        current_state = []
        for i, piece in enumerate(self.pieces):
            current_chunk = []
            orientation = self.orientations[i]
            current_chunk.append(solved_cube[piece][orientation]) #top piece
            current_chunk.append(solved_cube[piece][(orientation + 1) % 3]) #right piece
            current_chunk.append(solved_cube[piece][(orientation + 2) % 3]) #left piece
            current_state.append(current_chunk) # This is the stickers on piece[x] arranged clockwise
            
#        cube_string =  "        +-----+" + nl
#        cube_string += "        |"+current_state[3][0]+" U "+current_state[2][0]+"|" + nl
#        cube_string += "        |"+current_state[1][0]+"   "+current_state[0][0]+"|" + nl
#        cube_string += "        +-----+" + nl
#        cube_string += "+-----+ +-----+ +-----+ +-----+" + nl
#        cube_string += "|"+current_state[3][1]+" L "+current_state[1][2]+"| |"+current_state[1][1]+" F "+current_state[0][2]+"| |"+current_state[0][1]+" R "+current_state[2][2]+"| |"+current_state[2][1]+" B "+current_state[3][2]+"|" + nl
#        cube_string += "|"+current_state[7][2]+"   "+current_state[5][1]+"| |"+current_state[5][2]+"   "+current_state[4][1]+"| |"+current_state[4][2]+"   "+current_state[6][1]+"| |"+current_state[6][2]+"   "+current_state[7][1]+"|" + nl
#        cube_string += "+-----+ +-----+ +-----+ +-----+" + nl
#        cube_string += "        +-----+" + nl
#        cube_string += "        |"+current_state[5][0]+" D "+current_state[4][0]+"|" + nl
#        cube_string += "        |"+current_state[7][0]+"   "+current_state[6][0]+"|" + nl
#        cube_string += "        +-----+" + nl
        
        cube_string =  "                                  ___________" + nl
        cube_string += "                                 |     |     |" + nl
        cube_string += "                    __________   |  "+current_state[2][1]+"  |  "+current_state[3][2]+"  |" + nl
        cube_string += "   /|              / "+current_state[3][0]+"  / "+current_state[2][0]+"  /|  |_____|_____|" + nl
        cube_string += "  / |             /____/____/ |  |     |     |" + nl
        cube_string += " /| |            / "+current_state[1][0]+"  / "+current_state[0][0]+"  /| |  |  "+current_state[6][2]+"  |  "+current_state[7][1]+"  |" + nl
        cube_string += "/ |"+current_state[1][2]+"|           /____/____/ |"+current_state[2][2]+"|  |_____|_____|" + nl
        cube_string += "|"+current_state[3][1]+"| |          |     |    |"+current_state[0][1]+"| |     BACK" + nl
        cube_string += "| |/|          |  "+current_state[1][1]+"  | "+current_state[0][2]+"  | |/|" + nl
        cube_string += "|/|"+current_state[5][1]+"|          |_____|____|/|"+current_state[6][1]+"|" + nl
        cube_string += "|"+current_state[7][2]+"| |          |     |    |"+current_state[4][2]+"| |" + nl
        cube_string += "| |/           |  "+current_state[5][2]+"  | "+current_state[4][1]+"  | |/" + nl
        cube_string += "|/LEFT         |_____|____|/" + nl + nl + nl 
        cube_string += "              __________" + nl
        cube_string += "             / "+current_state[5][0]+"  / "+current_state[4][0]+"  /" + nl
        cube_string += "            /____/____/" + nl
        cube_string += "           / "+current_state[7][0]+"  / "+current_state[6][0]+"  /" + nl
        cube_string += "          /____/____/" + nl
        cube_string += "              DOWN" + nl
        
        if graphing:
            for face, color in self.dotty_color_scheme.iteritems():
                cube_string = cube_string.replace(face, '<font color="' + color + '">' + face + '</font>')
            cube_string = "<table><tr><td bgcolor=\"gray\">" + cube_string + "</td></tr></table>"
        else:
            for face, color in self.console_color_scheme.iteritems():
                cube_string = cube_string.replace(face, '\033[' + color + 'm' + face + '\033[m')
                
        return cube_string
    
    def __hash__(self):
        hash = 0
        #only need to hash the first 6 pieces to be unique
        for piece in self.pieces[:-2]:
            hash <<= 3
            hash |= piece
        for orientation in self.orientations[:-2]:
            hash <<= 2
            hash |= orientation
        return hash
