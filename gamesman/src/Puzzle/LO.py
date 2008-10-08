from Puzzle import *
from math import sqrt
import copy
import string

class LO(Puzzle):
    """This is the Lights Out puzzle class"""
   
    def __init__(self, board):
        #size = side length 1, 2, 3, etc...
        len = board.__len__()
        
    
        size = sqrt(board.__len__())
       
        if round(size) != size:
            raise 'BadBoardError', str(len) + ' is not a valid board size, yo.'
            return
        self.size = int(size)
        self.board = self.rep_board(board)
        
        #self.board = board
        #save point
        
        Puzzle(True) 

    def generate_start(self):
        '''
        li = list()
        for i in range(self.size**2):
            li+=[1]
        return LO(li)
        '''
        return LO(list(self.board))

#   def is_solution(self):
#       return self.F == True

    def generate_solutions(self):
        board = list()
        for i in range(self.size ** 2):
            board += [0]
        return [LO(board)]
   
    def generate_moves(self):
        '''
        moves = []
        for i in range(self.size **2):
            possible_move=self.rep_board(self.do_move(i).board)
            if not possible_move in boards:
                boards+=[possible_move]
                moves+=[i]
        return moves
        '''
        return range(self.size ** 2)
        
        '''
        moves = []
        for i in range(self.size **2):
            temp = self.do_move_to_board(i)
            if temp == self.rep_board(temp):
                moves += [i]
            print str(temp) + ";" + str(self.rep_board(temp))
        return moves
        '''

    
    def do_move(self, move):
         
        board = list(self.board)
        size = self.size
        posx = move%size
        posy = move/size
        board[move] = int(not board[move]) #swap current square
        if posx > 0:
            board[size * posy + (posx - 1)] = not board[size * posy + (posx - 1)]
        if posy > 0:
            board[size * (posy - 1) + posx] = not board[size * (posy - 1) + posx]
        if posx < size-1:
            board[size * posy + (posx + 1)] = not board[size * posy + (posx + 1)]
        if posy < size-1:
            board[size * (posy + 1) + posx] = not board[size * (posy + 1) + posx]
        
        return LO(board)
        
        #  0 1 2 3 x
        #0 X X X X
        #1 X X X X
        #2 X X X X
        #3 X X X X
        #y
    def rep_board(self,board):
        size = int(sqrt(board.__len__()))
        boardlist = [board]
        rotboard = list(board)
        for i in range(4):
            boardlist+=[self.reverse(rotboard)]
	    boardlist+=[self.reversey(rotboard)]
            rotboard = self.rotate(rotboard)
            boardlist += [rotboard]
        least = 2**(size**2)
        ileast = 0
        for j in range(8):
            hash = 0
            for i in boardlist[j]:
                hash <<= 1
                hash += i
            if least > hash:
                least = hash
                ileast = j
        return boardlist[ileast]
        
    def rotate(self,board):
        #rotates clockwise
        size = int(sqrt(board.__len__()))
        newboard = list(board)
        for y in range(size):
            for x in range(size):
                newboard[y*size + size - x -1]  = board[x*size+y]
        return newboard

    def reverse(self,board):
        #flip along X axis
        size = int(sqrt(board.__len__()))
        newboard = list(board)
        for y in range(size):
            for x in range(self.size):
                newboard[(y+1)*size - (x+1)]  = board[y*size+x]
                
                
        return newboard
    def reversey(self,board):
        #flip along y axis
        size = int(sqrt(board.__len__()))
        newboard = list(board)
        for y in range(size):
            for x in range(size):
                newboard[size ** 2 - y*size - size + x]  = board[y*size+x]
               
        return newboard            
    def is_illegal(self):
        ### Illegal if Chicken is near Fox or Grain without Boat
        ### i.e., C == F != B  OR  C == G != B
        return False

    def __str__(self):
   
        toreturn = str()
        for a in range(self.size):
            for b in range(self.size):
                toreturn += str(int(self.board[a*self.size+b]))
            toreturn+="\\n"
     
        return toreturn.__getslice__(0,toreturn.__len__()-2)

    def __hash__(self):
        hash = 0
    
        
        for i in self.board:
            hash <<= 1
            hash += i
        
        return hash
            

        
       
               
        
      

     
            
