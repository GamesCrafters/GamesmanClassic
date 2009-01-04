import sys
import pygamesman
from pygamesman import export

win = 0
lose = 1
tie = 2
undecided = 3

class Game:
	
	def __init__(self):
		pygamesman.export(
			gInitialPosition = long(self.gInitialPosition),
			gNumberOfPositions = long(self.gNumberOfPositions),
			kBadPosition = long(self.kBadPosition),
			kGameSpecificMenu = bool(self.kGameSpecificMenu),
			kPartizan = bool(self.kPartizan),
			kDebugMenu = bool(self.kDebugMenu),
			kTieIsPossible = bool(self.kTieIsPossible),
			kLoopy = bool(self.kLoopy),
			kDebugDetermineValue = bool(self.kDebugDetermineValue),
			kAuthorName = str(self.kAuthorName),
			kGameName = str(self.kGameName),
			kHelpGraphicInterface = str(self.kHelpGraphicInterface),
			kHelpTextInterface = str(self.kHelpTextInterface),
			kHelpOnYourTurn = str(self.kHelpOnYourTurn),
			kHelpStandardObjective = str(self.kHelpStandardObjective),
			kHelpReverseObjective = str(self.kHelpReverseObjective),
			kHelpTieOccursWhen = str(self.kHelpTieOccursWhen),
			kHelpExample = str(self.kHelpExample),
			kDBName = str(self.kDBName))
		# Import psyco for JIT compilation, if available
		try:
			import psyco
			psyco.full()
		except ImportError:
			pass
		pygamesman.initialize(sys.argv, self.call)

	def call(self, name, *args):
		return apply(getattr(self, name), args)
	
	def InitializeGame(self):
		pass
