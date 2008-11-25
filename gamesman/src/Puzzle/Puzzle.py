class Puzzle:
    """This is the base class for a puzzle"""

    def __init__(self,  one_solution = True):
        self.one_solution = one_solution

    def generate_start(self):      # By default, no starting point (like Rubik's cube)
        return False

    def generate_solutions(self):
        return []
    
    def generate_moves(self):
        return []

    def do_move(self, move):
        return []

    def __add__(self, move):
        return self.do_move(move)

    def is_a_solution(self):
        return self in self.generate_solutions()

    def is_illegal(self):
        return False

    def reverse_move(self, move):
        return move

    def __cmp__(self, other):
        return cmp(hash(self), hash(other))

    def __hash__(self):
        return 0

    def unhash(self, num): # add this function to whatever you have
        return self.generate_start()

    def __str__(self):
        return "Default Puzzle string"

    # static methods
    def serialize(self):
        return

    def unserialize(self):
        return
