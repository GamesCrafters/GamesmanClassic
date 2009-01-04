#!/usr/bin/python

from gamesman import *
from output import *

class Pyckle(Game):

	kGameName = "Pyckle"
	kAuthorName = "Hal9000"
	kPartizan = True
	kLoopy = False
	kGameSpecificMenu = False
	kTieIsPossible = True
	kDebugDetermineValue = False
	kDebugMenu = False
	kHelpGraphicInterface = ""
	kHelpTextInterface = ""
	kHelpOnYourTurn = ""
	kHelpStandardObjective = ""
	kHelpReverseObjective = ""
	kHelpTieOccursWhen = ""
	kHelpExample = ""
	kDBName = ""
	kBadPosition = 65536			# Sloppy syntax, should be 256L but is converted in superclass when exported
	gInitialPosition = 0		
	gNumberOfPositions = 65536L 	# Correct syntax since POSITION is long in python

	def PrintPosition(self, position, playerName, usersTurn):
		output="\n\t"
		for cell in range(0, 16, 2):
			output += yellow("[") + {0:" ", 1:blue("x"), 2:red("*")}[(position >> cell) & 3] + yellow("]")
		output += "\t\t(" + playerName + "'s unit is " + {True:blue("x"), False:red("*")}[usersTurn] + ")"
		print output
	
	def GenerateMoves(self, position):
		movelist = []
		for cell in range(0, 16, 2):
			if ((position >> cell) & 3) == 0:
				movelist.append(cell/2)
		return movelist

	def PrintPrompt(self, position, playerName):
		sys.stdout.write("\n" + playerName + "'s move [(undo)/[0-7]] ")
		sys.stdout.flush()

	def PrintMove(self, move):
		sys.stdout.write(str(move));
		sys.stdout.flush();

	def PrintComputersMove(self, move, computerName):
		print computerName + "'s move was: " + str(move)

	def ValidTextInput(self, input):
		valid = False
		try:
			move = int(input)
			if move < 16 and move > -1:
				valid = True
		finally:
			return valid

	def ConvertTextInputToMove(self, input):
		return int(input)

	def DoMove(self, position, move):
		players = {0:0, 1:0, 2:0}
		for cell in range(0, 16, 2):
			players[(position >> cell) & 3] += 1
		position |= ( {True:2, False:1}[players[1] > players[2]] ) << 2*move
		return position

	def Primitive(self, position):
		previous = 0
		blanks = 0
		for cell in range(0, 16, 2):
			current = (position >> cell) & 3
			if current == 0:
				blanks += 1
			elif previous == current:
				return lose
			previous = current
		if blanks == 0:
			return tie
		else:
			return undecided

if __name__ == "__main__":
	Pyckle()
