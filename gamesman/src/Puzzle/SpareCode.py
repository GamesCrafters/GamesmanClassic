'''        
    def find_solutions(self, puzzle):
        if puzzle.is_a_solution():
            print "FOUND SOLUTION"
            return [puzzle]
        else:
            solutions = []
            for move in puzzle.generate_moves():
                child = puzzle + move
                if not child.is_illegal():
                    if hash(child) not in self.visited:
                        self.visited[hash(child)] = True
                        solutions += self.find_solutions(child)
            return solutions
'''
