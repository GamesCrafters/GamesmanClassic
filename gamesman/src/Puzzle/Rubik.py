from Puzzle import *
import string

# 11022480
#Level 0 : 1       | 
#Level 1 : 9       | 9
#Level 2 : 54      | 6
#Level 3 : 321     | 5.94
#Level 4 : 1847    | 5.75
#Level 5 : 9992    | 5.41
#Level 6 : 50135   | 5.01
#Level 7 : 227510  | 4.53
#Level 8 : 869715  | 3.82
#Level 9 : 1885639 | 2.16

# cat = concatenate
def cat(s1, s2): return s1 + s2

# fact
def fact(n):
    if n < 1:
        return 1
    else:
        return n * fact(n-1)

class Rubik(Puzzle):
    """This is the 2x2x2 Rubiks Cube puzzle class"""

    def __init__(self, T="WWW",R="BBB",L="RRR",BT="YYYY",BR="OOOO",BL="GGGG"):
        # anchor on Top=white, Right=Blue, Left=Red, then flip around hand axis
        # clockwise, always (T3,R3,L3,BT4,BR4,BL4)
        self.T, self.R, self.L, self.BT, self.BR, self.BL = T,R,L,BT,BR,BL
        Puzzle(True) 

    def generate_solutions(self):
        return [Rubik()]   # Default _is_ solution; what else?
    
    def generate_moves(self):
        return ['B1','B2','B3','R1','R2','R3','L1','L2','L3']

    def do_move(self, move):
        T,   R,  L = list(self.T), list(self.R), list(self.L)
        BT, BR, BL = list(self.BT),list(self.BR),list(self.BL)
        times = int(move[1])
        for i in range(times):
            if move[0] == 'B':
                R[1],   R[2],  L[0],  L[1],  \
                BT[0], BT[1], BT[2], BT[3],  \
                BR[0], BR[1], BL[0], BL[3] = \
                L[0],  L[1],  BL[3], BL[0],  \
                BT[3], BT[0], BT[1], BT[2],  \
                R[1],  R[2],  BR[1], BR[0]
            elif move[0] == 'R':
                T[1],   T[2],  R[0],   R[1], BT[0], BT[3], \
                BR[0], BR[1], BR[2], BR[3],                \
                BL[0], BL[1] =                             \
                R[0],   R[1], BT[3], BT[0],  BL[1], BL[0], \
                BR[3], BR[0], BR[1], BR[2],                \
                T[1],   T[2]
            elif move[0] == 'L':
                T[0],   T[1],  L[1],  L[2],  \
                BT[0], BT[1], BR[0], BR[3],  \
                BL[0], BL[1], BL[2], BL[3] = \
                BR[3], BR[0],  T[0],  T[1],  \
                L[1],  L[2], BT[1], BT[0],   \
                BL[3], BL[0], BL[1], BL[2] 
            else:
                raise 'BadMoveError', move + ' is not one of [BRL][123]'
        return Rubik(reduce(cat,T),  reduce(cat,R),  reduce(cat,L),
                     reduce(cat,BT), reduce(cat,BR), reduce(cat,BL))
            
    def reverse_move(self, move):
        return move[0] + str(4-int(move[1])) 

    def __str__(self):
        return self.T+'|'+self.R+'|'+self.L+'|'+self.BT+'|'+self.BR+'|'+self.BL

    def __hash__(self):
        return hash(str(self))
    
