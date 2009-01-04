class Puzzle:
    """This is the base class for a puzzle"""

    def __init__(self,  one_solution = True):
        self.one_solution = one_solution

    def generate_start(self):
        return False

    def generate_solutions(self):
        return []
    
    def generate_moves(self):
        return []

    def reverse_move(self, move):
        """deprecated"""
        return move

    def get_permutations(self):
        """deprecated"""
        return 0

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

    # Functions required for web framework:
    def serialize(self):
        """Returns a string representation of this object that can be unserialized.

           Has a similar relation as __hash__ and unhash, but should be independent
           of the hashing function used.  In some cases, __str__ can be used.
           """
        return str(self)
    
    # Dictionary containing the default options passed to constructor.
    default_options = {}
    
    @staticmethod
    def unserialize(options, boardstr):
        """Takes the output of serialize() and returns a new Puzzle object.

           If the board is invalid, returns None
           'options' is a dictionary containing any other arguments for the constructor.
           """
        pass # return new object with passed options and parse board string
