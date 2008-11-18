from Solver import Solver
import cPickle

class UnreverseSolver(Solver):
    """
    This is a solver of a single start state with a class of solutions.
    Level 0 is the initial state.
    Level x is the number of moves required to reach a certain state.
    """
    def __init__(self):
        self.levels = {}         # level_num : [hashed_position, hashed_position, ...]
        self.seen = {}           # hashed_position : (level_num, value)
        self.move = {}           # hashed_position : [move, move, ...]
        self.undoMove = {}       # hashed_position : [reverse_move, reverse_move, ...]
        self.visited = {}        # hashed_position : True
        self.path_to_answer = {} # hashed_position : True
        self.solutions = []      # [hashed_solution, hashed_solution, ...]
        self.deadends = []       # [hashed_deadend, hashed_deadend, ...]
        self.goodpaths = []      # [[(hashed_solution, value), (hashed_lastmove, value), ...]]
        self.deadpaths = []      # [[(hashed_deadend, value), (hashed_lastmove, value), ...]]
        self.puzzle = None

    def solve(self, puzzle, verbose=False, max_level=-1):
        '''
        Always run this first. Builds the dictionaries and lists for other methods.
        '''
        self.puzzle = puzzle
        self.levels[0] = [hash(puzzle.generate_start())]
        level = 0
        self.seen[hash(puzzle.generate_start())] = (level, puzzle.generate_start().value())
        if verbose:
            print "Level 0 : 1"
        while self.levels[level] and (max_level==-1 or level<max_level):
            self.levels[level+1] = []
            for hashed_position in self.levels[level]:
                position = puzzle.unhash(hashed_position)
                if not position.is_leaf():
                    for move in position.generate_moves():
                        child = position + move
                        hashed_child = hash(child)
                        if not child.is_illegal():
                            if hashed_child not in self.seen.keys():  # first time we've seen it
                                self.seen[hashed_child] = (level+1, child.value())
                                self.levels[level+1].append(hashed_child)
                                if not hashed_position in self.move:
                                    self.move[hashed_position] = [move]
                                else:
                                    self.move[hashed_position].append(move)
                                self.undoMove[hashed_child] = [move]
                            elif self.seen[hashed_child][0] == level+1: # another sol path!
                                if not hashed_position in self.move:
                                    self.move[hashed_position] = [move]
                                else:
                                    self.move[hashed_position].append(move)
                                self.undoMove[hashed_child].append(move)
                            else:
                                pass # we've seen it before; it's not making progress
                elif position.is_a_solution():
                    self.solutions.append(hashed_position)
                else: # deadend
                    self.deadends.append(hashed_position)

            if verbose and len(self.levels[level+1]) > 0:
                print "Level " + str(level+1) + " : " + str(len(self.levels[level+1]))
            level += 1
        del self.levels[level] # the last one is always empty
        self.findSolutionPaths()
        self.findDeadendPaths()

    def findSolutionPaths(self):
        '''
        Find all solution paths.
        '''
        stack = []
        for hashed_solution in self.solutions:
            stack.append((hashed_solution, []))
        while len(stack) > 0:
            current = stack[-1]
            hashed_puzzle = current[0]
            unhashed_puzzle = self.puzzle.unhash(hashed_puzzle)
            stack = stack[0:-1]
            if hashed_puzzle in self.undoMove.keys():
                for move in self.undoMove[hashed_puzzle]:
                    l_copy = current[1][:]
                    l_copy.append((current[0], unhashed_puzzle.value()))
                    parent = self.puzzle.unhash(hashed_puzzle) - move
                    stack.append((hash(parent), l_copy))
            elif self.seen[hashed_puzzle][0] == 0:
                current[1].append((current[0], unhashed_puzzle.value()))
                self.goodpaths.append(current[1])
            else:
                print "Error: invalid position"

    def findDeadendPaths(self):
        '''
        Find all deadend paths.
        '''
        stack = []
        for hashed_deadend in self.deadends:
            stack.append((hashed_deadend, []))
        while len(stack) > 0:
            current = stack[-1]
            hashed_puzzle = current[0]
            unhashed_puzzle = self.puzzle.unhash(hashed_puzzle)
            stack = stack[0:-1]
            if hashed_puzzle in self.undoMove.keys():
                for move in self.undoMove[hashed_puzzle]:
                    l_copy = current[1][:]
                    l_copy.append((current[0], unhashed_puzzle.value()))
                    parent = self.puzzle.unhash(hashed_puzzle) - move
                    stack.append((hash(parent), l_copy))
            elif hashed_puzzle in self.seen and self.seen[hashed_puzzle][0] == 0:
                current[1].append((current[0], unhashed_puzzle.value()))
                self.deadpaths.append(current[1])
            else:
                print "Error: invalid position"

    def findPath(self, puzzle):
        '''
        Find the path that least to the greatest result value.
        If values are equal, finds the closest result.
        '''
        seen = False
        dist = ()
        maxVal = 0
        result = None
        if hash(puzzle) in self.seen.keys():
            tup = (hash(puzzle), puzzle.value())
            for path in self.goodpaths:
                if tup in path:
                    d = path.index(tup)
                    if d < dist:
                        seen = True
                        dist = d
                        result = path[:]
            if seen:
                return result
            for path in self.deadpaths:
                if tup in path:
                    val = path[0][1]
                    if val >= maxVal:
                        d = path.index(tup)
                        if d < dist:
                            seen = True
                            dist = d
                            result = path[:]
            if seen:
                return result
        print 'Not found in any paths, something is wrong with findPath.'
        return None
    
    def printPath(self, puzzle):
        '''
        Prints a path that contains the given puzzle.
        '''
        path = self.findPath(puzzle)
        if len(path) == 0:
            if hash(puzzle) not in self.seen.keys():
                print "Sorry, position not in database"
            else:
                print "Sorry, no solution from this position"
        while len(path) > 0:
            print "Level: " + str(self.seen[path[-1][0]][0])
            print "Value: " + str(self.seen[path[-1][0]][1])
            print puzzle.unhash(path[-1][0])
            path = path[0:-1]

    def maxValue(self, puzzle):
        '''
        Finds the maximum value attainable from this point and distance to that value.
        '''
        path = self.findPath(puzzle)
        puzzleIndex = path.index((hash(puzzle), puzzle.value()))
        index = puzzleIndex
        maxValue = None
        maxIndex = None
        while (index >= 0):
            value = path[index][1]
            if value > maxValue:
                maxIndex = index
                maxValue = value
            index -= 1
        if maxValue == None:
            raise ValueError
        distance = puzzleIndex - maxIndex
        return (maxValue, distance)

    def nextMove(self, puzzle):
        '''
        Returns the next move to get to the maximum value possible.
        '''
        path = self.findPath(puzzle)
        if len(path) == 0:
            if hash(puzzle) not in self.seen.keys():
                print "Sorry, position not in database"
                return None
        else:
            if not ((puzzle in self.solutions) or (puzzle in self.deadends)):
                if self.maxValue(puzzle)[0] > puzzle.value():
                    return puzzle.unhash(path[path.index((hash(puzzle), puzzle.value()))-1][0])
            return None

    def save(self, fname):
        '''
        Saves the data to a file.
        '''
        f = open(fname, 'w')
        cPickle.dump((self.levels, self.deadends, self.goodpaths, self.deadpaths), f)
        f.close()

    def load(self, fname, puzzle):
        '''
        Loads the information from a file and a puzzle.
        '''
        self.puzzle = puzzle
        f = open(fname)
        self.levels, self.deadends, self.goodpaths, self.deadpaths = cPickle.load(f)
        f.close()
        for level in self.levels:
            for elt in self.levels[level]:
                self.seen[elt] = level
        for position in self.seen.keys():
            self.move[position] = []
            old_lv = self.seen[position]
            current = self.puzzle.unhash(position)
            for move in current.generate_moves():
                copy = current
                new_lv = self.seen[hash(copy+move)]
                if new_lv > old_lv:
                    self.move[position].append(move)
                elif new_lv < old_lv:
                    self.undoMove[position].append(move)
        self.solutions = self.puzzle.generate_solutions()

    # mark_path_to_answer and graph are not checked
    def mark_path_to_answer(self, puzzle):
        for path in self.goodpaths:
            for node in path:
                self.path_to_answer[node] = True

    def graph(self, puzzle, print_levels=True):
        print "graph G {"

        if print_levels:
            print " {\n   node [shape=plaintext];\n  ",
            level = 0
            while level < max(self.levels.keys()):
                print str(level) + " --",
                level += 1
            print str(level) + ";\n }"

        print " node [fontname = \"Courier\"];",

        start = (puzzle.unhash(self.levels[0][0])).generate_start() # Ask for its start
        if start: # Make starting points inv triangles
            print "  \""  + str(start) + "\" [shape=invtriangle]"

        self.mark_path_to_answer(start)
        for p in self.path_to_answer.keys():
            print "  \""  + str(puzzle.unhash(p)) + "\" [style=filled, color=\".7 .3 1.0\"];"

        for level in self.levels.keys():          
            rankstr = "  { rank=same; " + str(level) + "; "
            for hashed_p in self.levels[level]:
                p = puzzle.unhash(hashed_p)
                if hashed_p in self.solutions:
                    print "  \""  + str(puzzle.unhash(hashed_p)) + "\" [shape=triangle]"
                elif hashed_p in self.move.keys():
                    for move in self.move[hashed_p]:
                        answer = p + move                    
                        print "  \""  + str(p) + "\" -- \"" + \
                            str(answer) + "\"" + " [label = \"  " + str(move) + "\"]"
                rankstr += "\"" + str(p) + "\"; "
            if print_levels:
                print rankstr + " }"

        print "}"
