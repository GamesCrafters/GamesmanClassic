class Puzzle:
    """This is the base class for a puzzle"""

    def __init__(self,  one_solution = True):
        self.one_solution = one_solution

    def generate_start(self):      # By default, no starting point (like Rubik's cube)
        return False

    def generate_solutions(self):
        return []

    #use self.is_a_solution() instead
    #def is_solution(self):
    #    return False
    
    def generate_moves(self):
        return []

    def do_move(self, move):
        return []

    def undo_move(self, move): # new
        return []

    def __add__(self, move):
        return self.do_move(move)

    def __sub__(self, move): # new
        return self.undo_move(move)

    def is_a_solution(self): # new
        return self in self.generate_solutions()

    def is_deadend(self): # new
        return False

    def is_leaf(self): # new
        return self.is_a_solution() or self.is_deadend()

    def is_illegal(self):
        return False

    def __cmp__(self, other):
        return cmp(hash(self), hash(other))

    def __hash__(self):
        return 0

    def unhash(self, number): #new
        return self

    def __str__(self):
        return "Default Puzzle string"

    def value(self):
        return 1



    