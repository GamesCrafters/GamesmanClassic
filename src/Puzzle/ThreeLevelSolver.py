import cPickle

# simpler solver that takes much less RAM, but is only good for creating databases,
# since less info is stored
class Solver:
    """This is a solver of a single solution"""
    def __init__(self):
        self.seen = {}           # hashed_position : level_num
       
        self.visited = {}        # hashed_position : True
        self.puzzle = None
        self.maxHash = 0
        self.maxLevel = 0

    # non-recursive find_solutions
    def find_solutions(self, puzzle):
        puzzleQueue = []
        solutions = []
        puzzleQueue.extend([hash(puzzle)])
        while puzzleQueue:
            h_currPuzzle = puzzleQueue.pop()
            currPuzzle = puzzle.unhash(h_currPuzzle)
            if currPuzzle.is_illegal() or h_currPuzzle in self.visited:
                continue
            else:
                self.visited[h_currPuzzle] = True # Visit myself
                
                if currPuzzle.is_a_solution():
                    solutions += [currPuzzle]
                    
                h_currChildren = []
                for move in currPuzzle.generate_moves():
                    h_currChildren += [hash(currPuzzle + move)]
                    
                puzzleQueue.extend(h_currChildren)
        del self.visited
        return solutions

    def get_max_level(self):
        return self.maxLevel
    
    def solve(self, puzzle, verbose=False, max_level=-1):
        self.puzzle = puzzle
        solutions = puzzle.generate_solutions()

        curLevel = {}       # hashed_position : True
        prevLevel = {}      # hashed_position : True
        nextLevel = {}      # hashed_position : True

        if not solutions:
            solutions = self.find_solutions(puzzle)

        level = 0
        for solution in solutions:
            curLevel[hash(solution)] = True
            
        del solutions
        
        while curLevel and (max_level == -1 or level < max_level):                    
            # yield to database writer
            self.maxHash = max(max(curLevel), self.maxHash)
            yield (level, curLevel)
            if verbose and len(curLevel) > 0:
                print "Level " + str(level) + " : " + str(len(curLevel))
            
            for h_position in curLevel:
                position = puzzle.unhash(h_position)
                for move in position.generate_moves():
                    child = position + move
                    if not child.is_illegal():
                        h_child = hash(child)
                        if h_child not in prevLevel and h_child not in curLevel:  # first time we've seen it
                            nextLevel[h_child] = True
            level += 1
            
            prevLevel = curLevel
            curLevel = nextLevel
            nextLevel = {}
        self.maxLevel = level - 1
  
    def solveOld(self, puzzle, verbose=False, max_level=-1):
        self.puzzle = puzzle
        solutions = puzzle.generate_solutions()

        if not solutions:
            solutions = self.find_solutions(puzzle)

        level = 0
        hashPosAtLevel = []
        for solution in solutions:
            h_sol = hash(solution)
            self.seen[h_sol] = level
            hashPosAtLevel += [h_sol]
            
        del solutions
        
        while self.seen.values().__contains__(level) and (max_level==-1 or level<max_level):
            if level > 0:
                hashPosAtLevel = []
                seenItems = self.seen.items()
                # sort by level
                seenItems.sort(lambda x,y: cmp(x[1],y[1]))
                levelIndex = map(lambda x: x[1], seenItems).index(level)
                hashPosAtLevel = map(lambda x: x[0], seenItems[levelIndex:])
                del seenItems
                    
            for h_position in hashPosAtLevel:
                position = self.puzzle.unhash(h_position)
                for move in position.generate_moves():
                    child = position + move
                    if not child.is_illegal():
                        h_child = hash(child)
                        if h_child not in self.seen:  # first time we've seen it
                            self.seen[h_child] = level+1 
            if verbose and len(hashPosAtLevel) > 0:
                print "Level " + str(level) + " : " + str(len(hashPosAtLevel))
            level += 1
        
        # solved, now get the max hash
        self.get_max_hash()
