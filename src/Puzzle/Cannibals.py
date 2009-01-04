from Puzzle import *
import string

class Cannibals(Puzzle):
    """This is the Cannibals puzzle class"""

    def __init__(self, M = 3, C = 3, B = True):
        self.M, self.C, self.B = M, C, B
        Puzzle(True) # Missionaries-here | Cannibals-here | Boat-here

    def generate_solutions(self):
        return [Cannibals(0,0,False)] # Solution
    
    def generate_start(self):
        return Cannibals()  # default _is_ start
    
    def generate_moves(self):
        # A move is encoded as a two char string of "<C-to-take><M-to-take>"

        moves = []  # Remember, someone has to run boat, so no (0,0)!
        if self.M >= 2 and self.B or self.M <= 1 and not self.B:
            moves.append("20")
        if self.C >= 2 and self.B or self.C <= 1 and not self.B:
            moves.append("02")
        if self.M >= 1 and self.B or self.M <= 2 and not self.B:
            moves.append("10")
        if self.C >= 1 and self.B or self.C <= 2 and not self.B:
            moves.append("01")
        if (self.M >= 1 and self.C >= 1 and self.B) or \
           (self.M <= 2 and self.C <= 2 and not self.B):
            moves.append("11")
        
        return moves

    def do_move(self, move):
        M, C, B = self.M, self.C, self.B
        if B:                     # If boat is here, move TAKES them over
            M -= int(move[0])
            C -= int(move[1])
        else:                     # If boat is away, move BRINGS them over
            M += int(move[0])
            C += int(move[1])
        B = not B                 # Every move swaps the boat
        return Cannibals(M,C,B)   # Make a new position and return it
            
    def is_illegal(self):
        ### Illegal if Cannibals more than non-zero Missionaries any time
        return ((self.C > self.M) and (self.M > 0)) or \
               ((self.C < self.M) and (self.M < 3))

    def __str__(self):
        return str(self.M) + str(self.C) + str(int(self.B))

    def __hash__(self):
        return (4 * self.M + self.C) + 16 * int(self.B)
