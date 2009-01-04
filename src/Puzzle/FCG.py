from Puzzle import *
import string

class FCG(Puzzle):
    """This is the Fox-Chicken-Grain (Boat) puzzle class"""

    def __init__(self, F = False, C = False, G = False, B = False):
        self.F, self.C, self.G, self.B = F, C, G, B
        Puzzle(True) # Fox | Chicken | Grain | Boat

    def generate_start(self):
        return FCG() # default _is_ start

#    def is_solution(self):
#        return self.F == True

    def generate_solutions(self):
        return [FCG(True,True,True,True)] # True = there, False = here
   
    def generate_moves(self):
        moves = ['-']          # Can always take nothing
        if self.B == self.F:
            moves.append('F')  # If the boat is near Fox, move him!
        if self.B == self.C:
            moves.append('C')  # If the boat is near Chicken, move her!
        if self.B == self.G:
            moves.append('G')  # If the boat is near Grain, move it!
        return moves

    def do_move(self, move):
        F, C, G, B = self.F, self.C, self.G, self.B

        B = not B              # Every move swaps the boat
        if move == '-':        # If you want to bring nothing, done!
            pass
        elif string.upper(move) == 'F':
            F = not F          # Bringing the Fox over
        elif string.upper(move) == 'C':
            C = not C          # Bringing the Chicken over
        elif string.upper(move) == 'G':
            G = not G          # Bringing the Goose over
        else:
            raise 'BadMoveError', move + ' is not one of -,G,C,F'
        return FCG(F,C,G,B) # Make a new position and return it
            
    def is_illegal(self):
        ### Illegal if Chicken is near Fox or Grain without Boat
        ### i.e., C == F != B  OR  C == G != B
        return (self.C == self.F != self.B) or (self.C == self.G != self.B)

    def __str__(self):
        here = ""
        there = ""
        for item in ['F', 'C', 'G', 'B']:
            if not eval("self." + item):
                here += item
            else:
                there += item
        return here + " | " + there

    def __hash__(self):
        return (2 ** 3) * self.F + \
               (2 ** 2) * self.C + \
               (2 ** 1) * self.G + \
               (2 ** 0) * self.B

    def serialize(self):
        return str(self)

    default_options = {}
    @staticmethod
    def unserialize(options, bd=""):
        if '|' not in bd:
            bd = bd + '|'
        here, there = bd.split('|', 1)
        here = here.strip().upper()
        there = there.strip().upper()
        
        args = {}
        for c in here:
            args[c] = False
        for c in there:
            args[c] = True

        return FCG(**args)


