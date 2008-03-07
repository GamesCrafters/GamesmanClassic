import sys

class Solver:
    """This is a solver of a single solution"""
    
    def __init__(self):
        self.levels = {}         # level_num : [position, position, ...]
        self.seen = {}           # hashed_position : level_num
        self.move = {}           # hashed_position : [reversed_move, reversed_move, ...]
        self.visited = {}        # hashed_position : True
        self.path_to_answer = {} # hashed_position : True

    def find_solutions(self, puzzle):
        if puzzle.is_illegal() or hash(puzzle) in self.visited:
            return []
        else:
            self.visited[hash(puzzle)] = True # Visit myself
            solutions = []
            if puzzle.is_a_solution():
                solutions += [puzzle]
            for move in puzzle.generate_moves():
                child = puzzle + move
                solutions += self.find_solutions(child)
            return solutions


    def solve(self, puzzle, verbose=False, max_level=-1):
        solutions = puzzle.generate_solutions()

        if not solutions:
            solutions = self.find_solutions(puzzle)

        self.levels[0] = solutions
        level = 0
        for solution in solutions:
            self.seen[hash(solution)] = level
        if verbose:
            print "Level 0 : " + str(len(solutions))
        while self.levels[level] and (max_level==-1 or level<max_level):
            self.levels[level+1] = []
            for position in self.levels[level]:
                for move in position.generate_moves():
                    child = position + move
                    if not child.is_illegal():
                        if hash(child) not in self.seen:  # first time we've seen it
                            self.seen[hash(child)] = level+1
                            self.levels[level+1].append(child)
                            self.move[hash(child)] = [position.reverse_move(move)]
                        elif self.seen[hash(child)] == level+1: # another sol path!
                            self.move[hash(child)].append(position.reverse_move(move))
                        else:
                            pass # we've seen it before, but it isn't a solution path

            if verbose and len(self.levels[level+1]) > 0:
                print "Level " + str(level+1) + " : " + str(len(self.levels[level+1]))
            level += 1
        del self.levels[level] # the last one is always empty

    def path(self, puzzle):
        if puzzle not in self.seen:
            print "Sorry, position not in database"
        else:
            solutions = puzzle.generate_solutions()
            if not solutions:
                self.visited = {}
                solutions = self.find_solutions(puzzle)
            while puzzle not in solutions:
                print "        LEVEL: " + str(self.seen[hash(puzzle)])
                print puzzle
                print ""
                puzzle += self.move[hash(puzzle)][0] ### Walk left side of tree
            print "        LEVEL: " + str(self.seen[hash(puzzle)])
            print puzzle

    def mark_path_to_answer(self, puzzle):
        if puzzle.generate_start():
            start = puzzle.generate_start()
            self.path_to_answer[puzzle] = True
            level = self.seen[hash(puzzle)]
            active_positions = [puzzle]
            while level >= 0:
                next_active_positions = []
                for p in active_positions:
                    for m in p.generate_moves():
                        child = p + m
                        if not child.is_illegal() and self.seen[hash(child)] == level - 1 and child not in self.path_to_answer.keys():
                            self.path_to_answer[child] = True
                            next_active_positions.append(child)
                active_positions = next_active_positions
                level -= 1

    def graph(self, print_levels=True):
        print "graph G {"

        if print_levels:
            print " {\n   node [shape=plaintext];\n  ",
            level = max(self.levels.keys())
            while level > 0:
                print str(level) + " --",
                level -= 1
            print "0;\n }"

        print " node [fontname = \"Courier\"];",

        start = self.levels[0][0].generate_start() # Ask 1st solution for its start
        if start:                         # Make starting points inv triangles
            print "  \""  + str(start) + "\" [shape=invtriangle]"

        self.mark_path_to_answer(start)
        for p in self.path_to_answer.keys():
            print "  \""  + str(p) + "\" [style=filled, color=\".7 .3 1.0\"];"

        rankstr = "  { rank=same; 0; "
        for solution in self.levels[0]: # Make solutions triangles
            rankstr += "\"" + str(solution) + "\"; "
            print "  \""  + str(solution) + "\" [shape=triangle]"
        if print_levels:
            print rankstr + " }"

        for level in self.levels.keys():          
            if level != 0:                              # don't include solutions
                rankstr = "  { rank=same; " + str(level) + "; "
                for p in self.levels[level]:
                    for move in self.move[hash(p)]:
                        answer = p + move
                        rankstr += "\"" + str(p) + "\"; "
                        print "  \""  + str(p) + "\" -- \"" + \
                              str(answer) + "\"" + " [label = \"  " + str(move) + "\"]"
                if print_levels:
                    print rankstr + " }"

        print "}"            

	def savetofile(self, filename):
		outputfile = sys.open(filename, 'w')
		outputfile.write('<levels>\n')
		outputfile.write(str(self.levels))
		outputfile.write('\n</levels>\n')
		outputfile.write('<seen>\n')
		outputfile.write(str(self.seen))
		outputfile.write('\n</seen>\n')
		outputfile.write('<move>\n')
		outputfile.write(str(self.move))
		outputfile.write('\n</move>\n')
		outputfile.write('<visit>\n')
		outputfile.write(str(self.visit))
		outputfile.write('\n</visit>\n')
		outputfile.write('<path_to_answer>\n')
		outputfile.write(str(self.levels))
		outputfile.write('\n</path_to_answer>\n')
		outputfile.close()
		