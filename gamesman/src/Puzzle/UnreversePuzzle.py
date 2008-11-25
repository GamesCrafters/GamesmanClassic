from Puzzle import Puzzle

class UnreversePuzzle(Puzzle):
    """This is the base class for a puzzle"""

    def __init__(self,  one_solution = True):
        self.one_solution = one_solution

    def generate_start(self):
        return False

    def generate_solutions(self):
        return []
    
    def generate_moves(self):
        return []

    def do_move(self, move):
        return []

    def undo_move(self, move):
        return []

    def __add__(self, move):
        return self.do_move(move)

    def __sub__(self, move):
        return self.undo_move(move)

    def is_a_solution(self):
        return self in self.generate_solutions()

    def is_deadend(self):
        return (not self.is_a_solution()) and (self.is_leaf())

    def is_leaf(self):
        return (len(self.generate_moves) == 0)

    def is_illegal(self):
        return False

    def __cmp__(self, other):
        return cmp(hash(self), hash(other))

    def __hash__(self):
        return 0

    def unhash(self, number):
        return self

    def __str__(self):
        return "Default Puzzle string"

    def value(self):
        return 1



    