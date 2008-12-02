from Puzzle import * 
import string
import sys

class TCross(Puzzle):
    # IMPLIED BOARD:
    # [0   1   |2|  |3|   4   |5|  |6|   7   8
    #  9  |10| |11| |12| |13| |14| |15| |16| 17
    #  18 |19| |20| |21| |22| |23| |24| |25| 26
    #  27  28  |29| |30|  31  |32| |33|  --  --]
   
    # gameboard consists of a 9 element array, with each element representing
    # the index in the above implied board for each of the 9 pieces
    # (note: the reason for not storing the entire board is solely for saving memory when solving)
    # INDEX of gameboard = PIECE
    # 0                  = top-left semicircle (topl)
    # 1                  = top-right semicircle (topr)
    # 2                  = bottom-left semicircle (botl)
    # 3                  = bottom-right semicircle (botr)
    # 4                  = "binary" (bin)
    # 5                  = "arts" (art)
    # 6, 7, 8            = orange dot (dot)
    def __init__(self, gameboard = [25, 16, 30, 24, 20, 22, 10, 11, 19], # the nine pieces' index in board
                                    horizontalPos = 0, verticalPos = True, # state of sliders
                                    circle = 1, binArt = 1, dots = 0, exactSol = 1): # options
        # only store the pieces that are necessary (otherwise hash will fail)
        self.gameboard = [-1, -1, -1, -1, -1, -1, -1, -1, -1]
        if circle:
            self.gameboard[:4] = gameboard[:4]
        if binArt:
            self.gameboard[4:6] = gameboard[4:6]
        if dots:
            self.gameboard[6:9] = gameboard[6:9]
        
        # horizontalPos is either -1 (left), 0 (centered), or 1 (right)
        # verticalPos is either true (left slider down and right up) or false (left slider up and right down)
        self.horizontalPos = horizontalPos
        self.verticalPos = verticalPos
                
        self.circle = circle
        self.binArt = binArt
        self.dots = dots
        self.exactSol = exactSol
        Puzzle()
        
    def serialize(self):
        boardString = ""
        for piece in range(9): # loop through the pieces
            if (self.gameboard[piece] < 10 and self.gameboard[piece] >= 0):
                boardString += " " # add an extra space for 1-digit numbers that are non-negative
            boardString += self.gameboard[piece].__str__() + " "
        if (self.horizontalPos >= 0):
            boardString += " " # extra space needed because no negative sign
        boardString += self.horizontalPos.__str__() + " "
        boardString += (self.verticalPos * 1).__str__()
        return boardString
    
    default_options = {"circle":"1", "binArt":"1", "dots":"0", "exactSol":"1"}
    
    @staticmethod
    def unserialize(options, 
                    boardString = "25 16 30 24 20 22 10 11 19  0 1"):
        gameboard = [-1, -1, -1, -1, -1, -1, -1, -1, -1] # blank board
        
        i = 0 # string index
        for piece in range(9): # loop through pieces 0 to 8
            gameboard[piece] = int(boardString[i:i+3])
            i += 3
            
        horizPos = int(boardString[i:i+3])
        
        i += 3
        vertPos = False
        if int(boardString[i]):
            vertPos = True

        return TCross(gameboard, horizPos, vertPos, 
                      int(options["circle"]), int(options["binArt"]), int(options["dots"]), int(options["exactSol"]))
    
    def is_a_solution(self):
        # if exactSol, then just look in generate_solutions, otherwise look at pieces' relationships to determine if solution
        if self.exactSol:
            return self in self.generate_solutions()
        else:
            board = self.gameboard
            if self.circle:
                if (   (board[0] != board[1] - 1) # if topl isn't directly to left of topr
                    or (board[2] != board[3] - 1) # if botl isn't directly to the left of botr
                    or (board[0] != board[2] - 9) # if topl isn't directly above botl 
                    or (board[1] != board[3] - 9)): # if topr isn't directly above botr
                    return False
            if self.binArt:
                if (board[4] != board[5] - 1): # if bin isn't directly to left of art
                    return False
            if self.dots:
                if (    (board[7] + 1 != board[6] or board[6] != board[8] - 1)   # if not 768
                    and (board[8] + 1 != board[6] or board[6] != board[7] - 1)   # if not 867
                    and (board[6] + 1 != board[7] or board[7] != board[8] - 1)   # if not 678
                    and (board[8] + 1 != board[7] or board[7] != board[6] - 1)   # if not 876
                    and (board[6] + 1 != board[8] or board[8] != board[7] - 1)   # if not 687
                    and (board[7] + 1 != board[8] or board[8] != board[6] - 1)): # if not 786
                    return False
            return True

    def generate_solutions(self):
        # is used if doesn't return [], so return [] if exactSol is false
        if self.exactSol: 
            gameboard = [15, 16, 24, 25, 29, 30, 10, 11, 12]
            return [TCross(gameboard, 0, True, self.circle, self.binArt, self.dots, self.exactSol)];
        else:
            return []

    def generate_moves(self):
        moves = []
        
        if self.horizontalPos > -1:
            moves.append('L')  # shift slider to the left
        if self.horizontalPos < 1:
            moves.append('R') # shift slider to the right
            
        if self.verticalPos:
            moves.append('U') # shift left vertical slider up
        else:
            moves.append('D') # shift left vertical slider down
            
        return moves

    def do_move(self, move):
        # get values of self.gameboard -- not a reference to it
        gameboard = self.gameboard[:len(self.gameboard)]

        horizontalPos = self.horizontalPos
        verticalPos = self.verticalPos
        
        if move == 'L':
            for i in range(9): # look at each piece
                # slide middle rows left
                if gameboard[i] > 8 and gameboard[i] < 27:
                    gameboard[i] -= 1
                    
            horizontalPos -= 1
            
        elif move == 'R':
            for i in range(9): # look at each piece
                # slide middle rows right
                if gameboard[i] > 8 and gameboard[i] < 27:
                    gameboard[i] += 1
                    
            horizontalPos += 1
            
        elif move == 'U':
            for i in range(9): # look at each piece
                # shift left slider up (left slider = 2, 11, 20, 29 and 3, 12, 21, 30)
                if ((gameboard[i] - 2) % 9 == 0) or ((gameboard[i] - 3) % 9 == 0):
                    gameboard[i] -= 9
                # shift right slider down (right slider = 5, 14, 23, 32 and 6, 15, 24, 33)
                elif ((gameboard[i] - 5) % 9 == 0) or ((gameboard[i] - 6) % 9 == 0):
                    gameboard[i] += 9

            verticalPos = False  
  
        elif move == 'D': 
            for i in range(9): # look at each piece
                # shift left slider down (left slider = 2, 11, 20, 29 and 3, 12, 21, 30)
                if ((gameboard[i] - 2) % 9 == 0) or ((gameboard[i] - 3) % 9 == 0):
                    gameboard[i] += 9
                # shift right slider up (right slider = 5, 14, 23, 32 and 6, 15, 24, 33)
                elif ((gameboard[i] - 5) % 9 == 0) or ((gameboard[i] - 6) % 9 == 0):
                    gameboard[i] -= 9

            verticalPos = True

        else:
            raise 'BadMoveError', move + ' is not one of L,R,U,D'
        
        return TCross(gameboard, horizontalPos, verticalPos, self.circle, self.binArt, self.dots, self.exactSol)
    
    def reverse_move(self, move):
        if move == 'L':
            return 'R'
        if move == 'R':
            return 'L'
        if move == 'U':
            return 'D'
        if move == 'D':
            return 'U'
        else:
            raise 'BadMoveError', move + ' is not one of L,R,U,D' 
    
    def __str__(self):
        # create a temporary board to be converted to a string
        board = [0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0      ]
        
        if self.circle: # add the circle to board if circle option is selected
            for i in range(4):
                board[self.gameboard[i]] = i + 1
        if self.binArt: # add binary and arts to the board if binArt option is selected
            for i in range(4, 6):
                board[self.gameboard[i]] = i + 1
        if self.dots: # add the dots to board if dots option is selected
            for i in range(6, 9):
                 board[self.gameboard[i]] = 7
    
        boardString = ""
        shift = self.horizontalPos
        
        # add the top row (NOTE: see helper function below)
        if self.verticalPos:
            boardString += "     " + listToString(board[5:7]) + "\n" # for dotty, change to "\\n"
        else:
            boardString += "  " + listToString(board[2:4]) + "\n" # for dotty, change to "\\n"
        
        # shift the middle rows based on horizontalPos
        if shift == 0:
            boardString += " " # for dotty, total: 2 spaces
        elif shift == 1:
            boardString += "  " # for dotty, total: 4 spaces
        boardString +=  listToString(board[10 + shift:17 + shift]) + "\n" # for dotty, change to "\\n"
        
        if shift == 0:
            boardString += " " # for dotty, total: 2 spaces
        elif shift == 1:
            boardString += "  " # for dotty, total: 4 spaces
        boardString += listToString(board[19 + shift:26 + shift]) + "\n" # for dotty, change to "\\n"
        
        # add the bottom row
        if self.verticalPos:
            boardString += "  " + listToString(board[29:31]) # for not dotty, total: 2 spaces
        else:
            boardString += "     " + listToString(board[32:34]) # for not dotty, total: 5 spaces
        
        return boardString
   
    def __hash__(self): # NOTE: uses helper functions below
        # convert imaginary board from 0 to 33 -> 0 to 17
        conversion = convertBoard(self.horizontalPos, self.verticalPos)
        
        boardLen = len(conversion)
        hashCode = 0
        
        # set up the counts of pieces based on options selected
        # counts starts with topl and ends with dot
        cs = [0, 0, 0, 0, 0, 0, 0]
        if self.circle: # if circle option selected, include its count
            for i in range(4):
                cs[i] = 1
        if self.binArt: # if binArt option selected, include its count
            for i in range(4, 6):
                cs[i] = 1
        if self.dots: # if dots option selected, include its count
            cs[6] = 3

        # loop through each spot on the board (0 -> 17)
        for i in range(boardLen):
            # check if the board has a piece at position i
            if self.gameboard.__contains__(conversion[i]): 
                slots = boardLen - i - 1 # the number of slots to the right of the index
                
                piece = self.gameboard.index(conversion[i]) # the piece at the current position i
                if (piece > 5): # if its a dot
                    piece = 6 # all dots should just be numbered 6 in the hash function

                if (totalCount(cs) != slots + 1): # if still emptys left
                    hashCode += rearranger(cs, slots)
                        
                # values in terms of ordering for pieces: 
                # empty - 0, topl - 1, topr - 2, botl - 3, botr - 4, bin - 5, art - 6, dot - 7
                
                # getRestOfHashForPiece considers the given value of the piece and determines for which pieces
                # it should act as, calling rearranger multiple times as per generic hash
                hashCode += getRestOfHashForPiece(cs, slots, piece)

                cs[piece] -= 1 # decrement the count of the given piece
        
        # now account for the verticalPos and horizontalPos:
        # maxHash is the total number of possible combinations without taking into
        # account the verticalPos and horizontalPos: (numPositions)!/(numEmptys)!/(numDots)!
        maxHash = divFact(boardLen, boardLen - self.circle * 4 - self.binArt * 2 - self.dots * 3)
        maxHash /= fact(self.dots * 3)
        # offset the current hash value by the maxHash for verticalPos
        hashCode += maxHash * self.verticalPos
        # offset the current hash value by 2 * maxHash for horizontalPos
        hashCode += 2 * maxHash * (self.horizontalPos + 1)

        return hashCode
    
    def unhash(self, hashCode):
        # set up an empty gameboard to be modified
        gameboard = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        boardLen = len(gameboard)

        # first handle the verticalPos and horizontalPos
        vertPos = False
        horizPos = 0
        # maxHash is the total number of possible combinations without taking into
        # account the verticalPos and horizontalPos: (numPositions)!/(numEmptys)!/(numDots)!
        maxHash = divFact(boardLen, boardLen - self.circle * 4 - self.binArt * 2 - self.dots * 3)
        maxHash /= fact(self.dots * 3)
        # do the reverse of the hash
        for i in range(6):
            if (hashCode < (i + 1) * maxHash):
                hashCode -= i * maxHash
                if (i % 2 == 1): # if the factor is odd, then vertPos is true
                    vertPos = True
                horizPos = (i / 2) - 1 # divide the factor by 2 and subtract 1 
                break
            
        # set up the counts of pieces based on options selected
        # counts starts with topl and ends with dot
        cs = [0, 0, 0, 0, 0, 0, 0]
        if self.circle: # if circle option selected, include its count
            for i in range(4):
                cs[i] = 1
        if self.binArt: # if binArt option selected, include its count
            for i in range(4, 6):
                cs[i] = 1
        if self.dots: # if dots option selected, include its count
            cs[6] = 3
        
        # loop through each slot in the gameboard
        for i in range(boardLen):
            slots = boardLen - i - 1 # number of slots to the right of the current slot
            # check if the hash is greater than just rearranging all the still-available pieces
            # (if its not, then that means there's an empty here)
            if (hashCode >= rearranger(cs, slots)):
                # consider each possible piece 0 to 6
                for piece in range(7):
                    # if this piece is still available and the current hash is less than
                    # all the ways of rearranging the next possible piece
                    if (cs[piece] and hashCode < rearrangerAll(cs, slots, piece + 1)):
                        # we found the piece, so put it on the board, 
                        # subtract its hash from hashCode, and decrement its count
                        gameboard[i] = piece + 1
                        hashCode -= rearrangerAll(cs, slots, piece)
                        cs[piece] -= 1
                        break # stop looking at each piece since we found the right one
        
        # convert the gameboard to the TCross representation of 0 t 8
        board = [-1, -1, -1, -1, -1, -1, -1, -1, -1]
        # get the conversion board
        conversion = convertBoard(horizPos, vertPos)
        # look at each piece
        for i in range(len(board)):
            piece = i + 1
            if (piece > 6): # 3 dots
                piece = 7
            if gameboard.__contains__(piece):
                board[i] = conversion[gameboard.index(piece)]
                # for the 3 dots, have to remove the piece so that
                # they aren't looked at again by gameboard.index(piece)
                gameboard[gameboard.index(piece)] = 0
            
        return TCross(board, horizPos, vertPos, self.circle, self.binArt, self.dots, self.exactSol)
     
#### HELPER FUNCTIONS FOR HASH
# for given counts of piece and slots, find the number of rearrangements possible
def rearranger(counts, slots):
    return divFact(slots, slots - totalCount(counts)) / fact(counts[6])

def rearrangerAll(counts, slots, piece):
    return getRestOfHashForPiece(counts, slots, piece) + rearranger(counts, slots)    

def getRestOfHashForPiece(counts, slots, piece):
    tinyHash = 0    
    # for the given piece, pretend that the piece is really one of the pieces between 0 and piece
    # (as per generic hash), which is called actAsThisPiece 
    for actAsThisPiece in range(piece):
        if counts[actAsThisPiece]: # if still actAsThisPieces to the right of the current piece
            counts[actAsThisPiece] -= 1 # temporarily subtract 1 from the count of actAsThisPiece (as per generic hash)
            
            tinyHash += rearranger(counts, slots)
            
            counts[actAsThisPiece] += 1 # return the count to what it was
    
    return tinyHash            
    
def convertBoard(horizontalPos, verticalPos):
    # go through each value in the imaginary board indexed from 0 to 33 
    # conversion converts from 0 to 33 -> 0 to 17
    hs = horizontalPos # horizontal shift
    vs1 = 9 * verticalPos # vertical shift for left slider
    cs1 = hs + vs1 # combined horizontal and vertical shifts for left slider
    vs2 = 9 * (not verticalPos) # vertical shift for right slider (opposite of left slider)
    cs2 = hs + vs2
    if vs1 == 0:
        return          [2  + vs1, 3  + vs1,          5  + cs2, 6  + cs2,
                10 + hs, 11 + cs1, 12 + cs1, 13 + hs, 14 + cs2, 15 + cs2, 16 + hs,
                19 + hs, 20 + cs1, 21 + cs1, 22 + hs, 23 + vs2, 24 + vs2, 25 + hs]
    elif vs1 == 9:
        return          [2  + cs1, 3  + cs1,          5  + vs2, 6  + vs2,
                10 + hs, 11 + cs1, 12 + cs1, 13 + hs, 14 + cs2, 15 + cs2, 16 + hs,
                19 + hs, 20 + vs1, 21 + vs1, 22 + hs, 23 + cs2, 24 + cs2, 25 + hs]
  
# calculates factorial of n
facts = [1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 
        39916800, 479001600, 6227020800, 87178291200, 1307674368000, 20922789888000,
        355687428096000, 6402373705728000]
def fact(n):
    return facts[n]

# simplifies dividing a large factorial by a small one
def divFact(big, small):
    if small > big:
        raise 'divFactError', 'denominator larger than numerator'
    if big == small:
        return 1 # cancel out the small factorial on the denominator with larger one on numerator
    return big * divFact(big - 1, small)

def totalCount(counts): # hardcoded count function 
    return counts[0] + counts[1] + counts[2] + counts[3] + counts[4] + counts[5] + counts[6]

#### HELPER FUNCTION FOR STR
def listToString(list):
    listString = ""
    for i in range(len(list)):
        listString += list[i].__str__()
    return listString
