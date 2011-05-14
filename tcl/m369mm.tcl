#############################################################################
# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.
############################################################################
proc GS_InitGameSpecific {} {
	
	### puts "GS_InitGameSpecific"

	### Set the name of the game
	global kGameName
	set kGameName "369 Men's Morris"
	
	### Set the initial position of the board (default 0)
	global gInitialPosition gPosition
	set gInitialPosition 0
	set gPosition $gInitialPosition

	### Set the strings to be used in the Edit Rules
	global kStandardString kMisereString
	set kStandardString "First player to ____ WINS"
	set kMisereString "First player to ____ LOSES"

	### Set the strings to tell the user how to move and what the goal is.
	### If you have more options, you will need to edit this section
	
	global gFlyingRule
	global gGameRule
	global gMillRule
	#set gGameRule 1
	global gMisereGame
	if {!$gMisereGame} {
		SetToWinString "To Win: (fill in)"
	} else {
		SetToWinString "To Win: (fill in)"
	}
	SetToMoveString "To Move: (fill in)"
	
	# Authors Info. Change if desired
	global kRootDir
	global kCAuthors kTclAuthors kGifAuthors
	set kCAuthors "(Fill this in)"
	set kTclAuthors "Daniel Wei, Kevin Liu, Patricia Fong, Dounan Shi"
	set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
	
	############   GLOBAL VARS ######################
	
	## IMPORTANT: These are variables used to change the board.
	global gFrameWidth gFrameHeight
	global boardSize squareSize leftBuffer topBuffer scale
	global numPositions
	
	set boardSize [expr [min $gFrameWidth $gFrameHeight] - 150]
	set squareSize [expr $boardSize / 5]
	set leftBuffer [expr [expr $gFrameWidth - $boardSize] / 2]
	set topBuffer [expr [expr $gFrameHeight - $boardSize] / 2]
	#set scale 1
	#set numPositions 16
	
	### Used to keep track of multi-click moves
	global clickCounter pMoves
	set clickCounter 0
	set pMoves undefined
	
	### Used to keep track of phase 1 or not phase 1
	global totalPieces
	#set totalPieces 12
	
	# A unique id for each playing piece
	global playingPieceId
	set playingPieceId 0
	
	# used to show move values
	global showMovesMoveType showMovesPosition showMovesMoveList
	# set these in case they are not initialized (avoid: "no such variable error")
	set showMovesMoveType ""
	set showMovesPosition ""
	set showMovesMoveList ""
	
	# used for click/drag piece movement
	global dragging dragPiece dragPiecePositionId mousePrevX mousePrevY prevDragMove
	set dragging 0
	set prevDragMove [list]
	# set these in case they are not initialized (avoid: "no such variable error")
	set dragPiece ""
	set dragPiecePositionId ""
	set mousePrevX ""
	set mousePrevY ""
	set prevDragMove ""
	
	# track if game is over
	global isGameOver
	set isGameOver 0
	
}

#############################################################################
# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"
#############################################################################
proc GS_NameOfPieces {} {
	return [list X O _]
}

#############################################################################
# GS_ColorOfPlayers should return a list of two strings, 
# each representing the color of a player.
# If a specific color appears uniquely on one player's pieces,
# it might be a good choice for that player's color.
# In impartial games, both players may share the same color.
# If the game is tic tac toe, this might be the line 
# return [list blue red]
# If the game is nim, this might be the line
# return [list green green]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he clicks "New Game"
# The left player's color should be the first item in the list.
# The right player's color should be second.
#############################################################################
proc GS_ColorOfPlayers {} {
	return [list #eeeeee #000000]
}

#############################################################################
# GS_SetupRulesFrame sets up the rules frame;
# Adds widgets to the rules frame that will allow the user to 
# select the variant of this game to play. The options 
# selected by the user should be stored in a set of global
# variables.
# This procedure must initialize the global variables to some
# valid game variant.
# The rules frame must include a standard/misere setting.
# Args: rulesFrame (Frame) - The rules frame to which widgets
# should be added
# Modifies: the rules frame and its global variables
# Returns: nothing
#############################################################################
proc GS_SetupRulesFrame { rulesFrame } {
	
	# Create the font to be used by the "Cancel Move" button
	font create cancelMoveFont -size 14 -family helvetica -weight bold
	
	# Create the font to be used by the "Game Over" text
	font create gameOverFont -size 40 -family helvetica -weight bold

	set standardRule \
	[list \
		 "What would you like your winning condition to be:" \
		 "Standard" \
		 "Misere" \
		]
	
	set flyingRule \
	[list \
		"Would you like to enable flying?" \
		"No Flying" \
		"Flying" \
		]
		
	set gameRule \
	[list \
		"Which Men's Morris game would you like to play?" \
		"Three" \
		"Six" \
		"Nine" \
		]
	
	set millRule \
	[list \
		"Which variant to remove pieces would you like to play?" \
		"non-mill unless only choice" \
		"any" \
		"non-mill regardless" \
		]
	
	# List of all rules, in some order
	set ruleset [list $standardRule $flyingRule $gameRule $millRule]

	# Declare and initialize rule globals
	global gMisereGame
	set gMisereGame 0
	

	global gFlyingRule
	set gFlyingRule 0
	
	global gGameRule
	set gGameRule 1
	
	
	global gMillRule
	set gMillRule 0
	
	# List of all rule globals, in same order as rule list
	set ruleSettingGlobalNames [list "gMisereGame" "gFlyingRule" "gGameRule" "gMillRule"]

	global kLabelFont
	set ruleNum 0
	foreach rule $ruleset {
		frame $rulesFrame.rule$ruleNum -borderwidth 2 -relief raised
		pack $rulesFrame.rule$ruleNum  -fill both -expand 1
		message $rulesFrame.rule$ruleNum.label -text [lindex $rule 0] -font $kLabelFont
		pack $rulesFrame.rule$ruleNum.label -side left
		set rulePartNum 0
		foreach rulePart [lrange $rule 1 end] {
			radiobutton $rulesFrame.rule$ruleNum.p$rulePartNum -text $rulePart -variable [lindex $ruleSettingGlobalNames $ruleNum] -value $rulePartNum -highlightthickness 0 -font $kLabelFont
			pack $rulesFrame.rule$ruleNum.p$rulePartNum -side left -expand 1 -fill both
			incr rulePartNum
		}
		incr ruleNum
	} 

}

#############################################################################
# GS_GetOption gets the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code
#############################################################################
proc GS_GetOption { } {
	global gMisereGame gGameRule gFlyingRule gMillRule
	
	set option 1
	set option [expr $option + (1 - $gMisereGame)]
	set option [expr $option + (2 * $gFlyingRule)]
	set option [expr $option + (4 * $gGameRule)]
	set option [expr $option + (16 * $gMillRule)]
	
	
	return $option
}

#############################################################################
# GS_SetOption modifies the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option.
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing
#############################################################################
proc GS_SetOption { option } {
	global gMisereGame gFlyingRule gGameRule numPositions totalPieces scale kGameName gMillRule
	set option [expr $option - 1]
	
	set gMisereGame [expr 1 - ($option % 2)]
	set gFlyingRule [expr ($option / 2) % 2]
	set gGameRule [expr ($option / 4) % 4]
	set gMillRule [expr $option / 16]

	if {$gGameRule == 0} {
		set scale 2
		set numPositions 9
		set totalPieces 6
	}
	if {$gGameRule == 1} {
		set scale 1
		set temp 2
		set numPositions 16
		set totalPieces 12
	}
	if {$gGameRule == 2} {
		set temp 3
		set scale 1
		set totalPieces 18
		set numPositions 24
	}
}

#############################################################################
# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"
#############################################################################
proc GS_Initialize { c } {

	# you may want to start by setting the size of the canvas; this line isn't necessary
	#$c configure -width 500 -height 500

	global boardSize squareSize leftBuffer topBuffer scale
	global numPositions totalPieces gGameRule
	
	set positionId 0
	set i 0
	set j 0

	if {$gGameRule == 0} {
		set scale 2
		set numPositions 9
		set totalPieces 6
	}
	if {$gGameRule == 1} {
		set scale 1
		set temp 2
		set numPositions 16
		set totalPieces 12
	}
	if {$gGameRule == 2} {
		set temp 3
		set scale 0.7
		set totalPieces 18
		set numPositions 24
	}	
	set scaledSquareSize [expr $scale*$squareSize]
	
	#Sets coordinates for center of game table
	set mx [expr $leftBuffer + [expr (2.5) * $squareSize]]
	set my [expr $topBuffer + [expr (2.5) * $squareSize]]
	
	# graphics
	set canvasColor #bb917b
	set lineColor #4c3225
	set positionMarkerColor $lineColor
	set lineWidth 3
	
if { $gGameRule == 0 } {
	for {set k 0} {$k < 3} {incr k} {         ;# //traverse rows
		set ty [expr ($leftBuffer+$k*$scaledSquareSize + 44)]
		for {set l 0} {$l < 3} {incr l} {        ;# //traverse columns
			set tx [expr ($topBuffer+$l*$scaledSquareSize + 44)]
			set clickRadius [getRadiusGivenScale [expr 2*$scale]]
			set markerRadius [getRadiusGivenScale [expr $scale - 0.4]]
			makeOval $c $tx $ty mi-$positionId $clickRadius $canvasColor
			makeOval $c $tx $ty [list base positionMarker] $markerRadius $canvasColor
			set mx-$positionId $tx
			set my-$positionId $ty
			incr positionId
		}
	}
	
}	
	
	
if {$gGameRule != 0} {
	if {$gGameRule == 2} {
		set scale 1
	}

	#Handles top half points
	for {set k 0} {$k < $temp} {incr k} {         ;# //traverse rows
		for {set l 0} {$l <= 2} {incr l} {        ;# //traverse columns
			set ty [expr ($my-($temp-$k)*$scaledSquareSize)]
			set tx [expr (($mx+$k*$scaledSquareSize)-($temp-($temp-$k)*$l)*$scaledSquareSize)]
			set clickRadius [getRadiusGivenScale [expr 2*$scale]]
			set markerRadius [getRadiusGivenScale [expr $scale - 0.4]]
			makeOval $c $tx $ty mi-$positionId $clickRadius $canvasColor
			makeOval $c $tx $ty [list base positionMarker] $markerRadius $canvasColor
			set mx-$positionId $tx
			set my-$positionId $ty
			incr positionId
		}
	}

	#Handles middle row
	set ty $my
	set tx [expr ($mx-$temp*$scaledSquareSize) ]
	
	for {set l 0} {$l <= [expr (2*$temp)]} {incr l} {
		if {$l!=$temp} {
			set clickRadius [getRadiusGivenScale [expr 2*$scale]]
			set markerRadius [getRadiusGivenScale [expr $scale - 0.4]]
			makeOval $c $tx $ty mi-$positionId $clickRadius $canvasColor
			makeOval $c $tx $ty [list base positionMarker] $markerRadius $canvasColor
			set mx-$positionId $tx
			set my-$positionId $ty
			incr positionId
		}	
		set tx [expr ($tx+$scaledSquareSize)]
	}
	
	#Handles bottom half
	for {set k [expr $temp-1]} {$k > -1} {set k [expr $k-1]} {         ;# //traverse rows
		for {set l 0} {$l <= 2} {incr l} {        ;# //traverse columns
			set ty [expr ($my+($temp-$k)*$scaledSquareSize)]
			set tx [expr (($mx+$k*$scaledSquareSize)-($temp-($temp-$k)*$l)*$scaledSquareSize)]
			set clickRadius [getRadiusGivenScale [expr 2*$scale]]
			set markerRadius [getRadiusGivenScale [expr $scale - 0.4]]
			makeOval $c $tx $ty mi-$positionId $clickRadius $canvasColor
			makeOval $c $tx $ty [list base positionMarker] $markerRadius $canvasColor
			set mx-$positionId $tx
			set my-$positionId $ty
			incr positionId
		}
	}
}	
	$c itemconfig positionMarker -outline $positionMarkerColor -width $lineWidth
	
	$c create rect \
		[expr $leftBuffer + 0 * $squareSize - 36] \
		[expr $topBuffer + 0 * $squareSize - 36] \
		[expr $leftBuffer + 5 * $squareSize + 36] \
		[expr $topBuffer + 5 * $squareSize + 36] \
		-fill $canvasColor -tag [list bg base] -outline white -width 4
		
	#drawing lines
	if {$gGameRule == 0} {
		#horizontal
		$c create line ${mx-0} ${my-0} ${mx-2} ${my-2} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-3} ${my-3} ${mx-5} ${my-5} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-6} ${my-6} ${mx-8} ${my-8} -tag base -width $lineWidth -fill $lineColor
		#vertical
		$c create line ${mx-0} ${my-0} ${mx-6} ${my-6} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-1} ${my-1} ${mx-7} ${my-7} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-2} ${my-2} ${mx-8} ${my-8} -tag base -width $lineWidth -fill $lineColor
	}
	if {$gGameRule == 1} {
		#horizontal
		$c create line ${mx-0} ${my-0} ${mx-2} ${my-2} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-3} ${my-3} ${mx-5} ${my-5} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-6} ${my-6} ${mx-7} ${my-7} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-8} ${my-8} ${mx-9} ${my-9} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-10} ${my-10} ${mx-12} ${my-12} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-13} ${my-13} ${mx-15} ${my-15} -tag base -width $lineWidth -fill $lineColor
		#vertical
		$c create line ${mx-0} ${my-0} ${mx-13} ${my-13} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-3} ${my-3} ${mx-10} ${my-10} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-1} ${my-1} ${mx-4} ${my-4} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-11} ${my-11} ${mx-14} ${my-14} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-5} ${my-5} ${mx-12} ${my-12} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-2} ${my-2} ${mx-15} ${my-15} -tag base -width $lineWidth -fill $lineColor
	}
	if {$gGameRule == 2} {
		#horizontal
		$c create line ${mx-0} ${my-0} ${mx-2} ${my-2} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-3} ${my-3} ${mx-5} ${my-5} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-6} ${my-6} ${mx-8} ${my-8} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-9} ${my-9} ${mx-11} ${my-11} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-12} ${my-12} ${mx-14} ${my-14} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-15} ${my-15} ${mx-17} ${my-17} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-18} ${my-18} ${mx-20} ${my-20} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-21} ${my-21} ${mx-23} ${my-23} -tag base -width $lineWidth -fill $lineColor
		#vertical
		$c create line ${mx-0} ${my-0} ${mx-21} ${my-21} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-3} ${my-3} ${mx-18} ${my-18} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-6} ${my-6} ${mx-15} ${my-15} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-1} ${my-1} ${mx-7} ${my-7} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-16} ${my-16} ${mx-22} ${my-22} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-8} ${my-8} ${mx-17} ${my-17} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-5} ${my-5} ${mx-20} ${my-20} -tag base -width $lineWidth -fill $lineColor
		$c create line ${mx-2} ${my-2} ${mx-23} ${my-23} -tag base -width $lineWidth -fill $lineColor
	}
	
	# draw cancel button
	set bgCoord [getCoords $c bg]
	set bgX [lindex $bgCoord 0]
	set bgY [expr $topBuffer + 5*$squareSize + 13]
	$c create rect \
		[expr $bgX - 50] \
		[expr $bgY - 10] \
		[expr $bgX + 50] \
		[expr $bgY + 10] \
		-fill $canvasColor -outline "" -tag cancelMove
	$c create text $bgX $bgY \
		-justify center -text "Cancel Move" -font cancelMoveFont \
		-fill #42322a \
		-tag [list cancelMove cancelMoveTxt]
	
	$c bind cancelMove <ButtonRelease-1> "cancelMove $c"
	$c bind cancelMove <Enter> "cancelMoveRollover $c"
	$c bind cancelMove <Leave> "cancelMoveRollout $c"
	
	$c lower cancelMove
	
	$c raise positionMarker
	
	# gameover screen
	$c create rect \
		[expr $leftBuffer + 0 * $squareSize - 36] \
		[expr $my - 30] \
		[expr $leftBuffer + 5 * $squareSize + 36] \
		[expr $my + 30] \
		-fill $canvasColor -tag [list gameover gameoverBg] -outline white -width 4
	$c create text $mx $my \
		-justify center -text "Game Over" -font gameOverFont \
		-fill #42322a \
		-tag [list gameover gameoverText]
	$c lower gameover

}

proc GS_Deinitialize { c } {
	global pMoves
	
	set pMoves undefined
	$c delete deletable
}

#############################################################################
# GS_DrawPosition this draws the board in an arbitrary position.
# It's arguments are a canvas, c, where you should draw and the
# (hashed) position.  For example, if your game is a rearranger,
# here is where you would draw pieces on the board in their correct positions.
# Imagine that this function is called when the player
# loads a saved game, and you must quickly return the board to its saved
# state.  It probably shouldn't animate, but it can if you want.
#
# BY THE WAY: Before you go any further, I recommend writing a tcl function that 
# UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.
#############################################################################
proc GS_DrawPosition { c position } {
}

#############################################################################
# GS_NewGame should start playing the game. 
# It's arguments are a canvas, c, where you should draw 
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.
#############################################################################
proc GS_NewGame { c position } {
	GS_Deinitialize $c
	$c raise base
	$c raise positionMarker
	GS_InitGameSpecific
	bind $c <ButtonPress-1> "handlePress $c %x %y"
	bind $c <ButtonRelease-1> "handleRelease $c %x %y"
	bind $c <Motion> "handleMotion $c %x %y"
}

#############################################################################
# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
#############################################################################
proc GS_WhoseMove { position } {
	global numPositions
	
	set turn [C_ReturnTurn $position]
	
	if {$turn == "X"} {
		set val X
	} else {
		set val O
	}
	return $val
}

#############################################################################
# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"
#
# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.
#############################################################################
proc GS_HandleMove { c oldPosition theMove newPosition } {
	
	set move [unhashMove $theMove 1]
	set theMoveFrom [lindex $move 0]
	set theMoveTo [lindex $move 1]
	set theMoveRemove [lindex $move 2]
	
	if {$theMoveFrom == $theMoveTo} {
		
		set indicatorCoords [getCoords $c mi-$theMoveFrom]
		set ix [lindex $indicatorCoords 0]
		set iy [lindex $indicatorCoords 1]
		set playingPiece [getTagOfOverlappingItem $c $ix $iy playingPiece]
		
		if { $playingPiece == "" } {
			unhashBoard $newPosition a
			makePlayingPiece $c $theMoveFrom [getColor $c $a($theMoveFrom)]
		}
		
	} else {
		set coords [getCoords $c mi-$theMoveFrom]
		set endCoords [getCoords $c mi-$theMoveTo]
		set playingPiece [getTagOfOverlappingItem \
			 $c \
			 [lindex $coords 0] \
			 [lindex $coords 1] \
			 playingPiece]
		if { $playingPiece != "" } {
			movePlayingPiece $c $playingPiece $endCoords
		}
	}
	if { $theMoveRemove != $theMoveFrom } {
		removePlayingPiece $c $theMoveRemove
	}
}

#############################################################################
# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever the
# player clicks to make the move)  It is also the function that handles coloring
# of the moves according to value. It is called by gamesman just before the player
# is prompted for a move.
#
# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which radio button is down
# position = the current hashed position
# moveList = a list of lists.  Each list contains a move and its value.
# These moves are represented as numbers (same as in C)
# The value will be either "Win" "Lose" or "Tie"
# Example:  moveList: { [7 3] Win } { [1 2 3] Lose } { [15 2] Tie } 
#############################################################################
proc GS_ShowMoves { c moveType position moveList } {	
	
	global showMovesMoveType showMovesPosition showMovesMoveList
	
	set showMovesMoveType $moveType
	set showMovesPosition $position
	set showMovesMoveList $moveList
	
	showMoves $c $moveType $position $moveList
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {
	
	global showMovesMoveType showMovesPosition showMovesMoveList
	
	set showMovesMoveType $moveType
	set showMovesPosition $position
	set showMovesMoveList $moveList
	
	$c delete valueIndicator
	update idletasks
}


#############################################################################
# GS_HandleUndo handles undoing a move (possibly with animation)
# Here's the undo logic
# The game was in position A, a player makes move M bringing the game to position B
# then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A
#
# By default this function just calls GS_DrawPosition, but you certainly don't 
# need to keep that.
#############################################################################
proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {
	
	global pMoves

	cancelMove $c
	
	set move [unhashMove $theMoveToUndo 1]
	set theMoveFrom [lindex $move 0]
	set theMoveTo [lindex $move 1]
	set theMoveRemove [lindex $move 2]
	
	if { $theMoveRemove != $theMoveFrom } {
			
		unhashBoard $positionAfterUndo a
		
		if {$a($theMoveRemove) == "X"} {
			set color #eeeeee
		} else {
			set color #000000
		}
		
		makePlayingPiece $c $theMoveRemove $color
	}
	
	if {$theMoveFrom == $theMoveTo} {
		removePlayingPiece $c $theMoveFrom
	} else {
		set coords [getCoords $c mi-$theMoveTo]
		set endCoords [getCoords $c mi-$theMoveFrom]
		set playingPiece [getTagOfOverlappingItem \
			$c \
			[lindex $coords 0] \
			[lindex $coords 1] \
			playingPiece]
		movePlayingPiece $c $playingPiece $endCoords
	}
}

#############################################################################
# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
#############################################################################
proc GS_GetGameSpecificOptions { } {
}


#############################################################################
# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# You could use this function to draw the line striking out the winning row in 
# tic tac toe for instance.  Or, you could congratulate the winner.
# Or, do nothing.
#############################################################################
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove} {
	global isGameOver
	set isGameOver 1
	animateGameOver $c
}


#############################################################################
# GS_UndoGameOver is called when the player hits undo after the game is finished.
# This is provided so that you may undo the drawing you did in GS_GameOver if you 
# drew something.
# For instance, if you drew a line crossing out the winning row in tic tac toe, 
# this is where you sould delete the line.
#
# note: GS_HandleUndo is called regardless of whether the move undoes the end of the 
# game, so IF you choose to do nothing in GS_GameOver, you needn't do anything here either.
#############################################################################
proc GS_UndoGameOver { c position } {
	global isGameOver
	set isGameOver 0
	animateUndoGameOver $c
}



############################################################################
# Helper methods
############################################################################

proc unhashBoard {position arrayname} {
	global numPositions
	upvar $arrayname a
	set board [C_CustomUnhash $position]
	
	for {set i 0} {$i < $numPositions} {incr i} {
		if {[string equal [string index $board $i] "X"]} {   
			set a($i) X
		} elseif {[string equal [string index $board $i] "O"]} {
			set a($i) O
		} else {
			set a($i) _
		}
	}
}

proc unhashMove {theMove adjustForPhase} {
	
	global numPositions
	
	set theMoveFrom [expr $theMove/( $numPositions*$numPositions )]
	set theMoveTo [expr [expr $theMove/$numPositions] % $numPositions]
	set theMoveRemove [expr $theMove % $numPositions]
	
	# In phase 1, if you form a mill the move contains a second part: the piece to remove.
	# This is stored in theMoveTo, instead of theMoveRemove.
	# Set adjustForPhase to be 1 (true) to store the remove piece in theMoveRemove.
	
	if { $adjustForPhase && [isPhase1] } {
		set theMoveRemove $theMoveTo
		set theMoveTo $theMoveFrom
	}
	
	return [list $theMoveFrom $theMoveTo $theMoveRemove]
}

proc handleMotion { c x y } {
	
	global dragging dragPiece mousePrevX mousePrevY
	
	if { $dragging } {
		set dx [expr $x - $mousePrevX]
		set dy [expr $y - $mousePrevY]
		set mousePrevX $x
		set mousePrevY $y
		$c move $dragPiece $dx $dy
	}
}
proc handlePress { c x y } {
	
	global clickCounter
	global pMoves totalPieces
	global dragging dragPiece dragPiecePositionId mousePrevX mousePrevY
	global showMovesMoveType showMovesPosition showMovesMoveList

	if { ![isPhase1] } {
		# not phase 1

		set positionId [getPositionId $c $x $y]
		
		if { $positionId != "" && $dragPiece == ""} {
			
			set indicatorCoords [getCoords $c mi-$positionId]
			set ix [lindex $indicatorCoords 0]
			set iy [lindex $indicatorCoords 1]
			set playingPiece [getTagOfOverlappingItem $c $ix $iy playingPiece]
			
			if { $playingPiece != "" } {
				
				if { $pMoves == "undefined" } {
					set pMoves [getPossibleMoves]
				}
				filterPossibleMoves $positionId
				set dragging 1
				set dragPiece $playingPiece
				set dragPiecePositionId $positionId
				set mousePrevX $x
				set mousePrevY $y
				incr clickCounter
				
				# call after increasing clickCounter
				# showMoves uses the value of clickCounter
				showMoves $c $showMovesMoveType $showMovesPosition $showMovesMoveList
			}
		}
	}
}

proc handleRelease { c x y } {
	
	global gPosition
	global clickCounter
	global pMoves
	global dragging dragPiece dragPiecePositionId
	global showMovesMoveType showMovesPosition showMovesMoveList
	
	if { $pMoves == "undefined" } {
		set pMoves [getPossibleMoves]
	}
	set tempMoves $pMoves
	
	set positionId [getPositionId $c $x $y]
	
	global isGameOver
	if { $isGameOver } { 
		set positionId ""
	}

	if { [isPhase1] } {
		# phase 1
		
		if { $positionId != "" } {

			filterPossibleMoves $positionId

			if { [llength $pMoves] == 0 } {
				set pMoves $tempMoves
			} elseif { [llength $pMoves] == 1 && [llength [lindex $pMoves 0]] == [expr $clickCounter + 1] } {
				executeMove $c [lindex $pMoves 0]
			} else {
				
				# mill formed during phase 1
				# create piece
				set colors [GS_ColorOfPlayers]
				set whoseTurn [GS_WhoseMove $gPosition]
				if { $whoseTurn == "X" } {
					set color [lindex $colors 0]
				} else {
					set color [lindex $colors 1]
				}
				makePlayingPiece $c $positionId $color
				
				incr clickCounter
				
				# call after increasing clickCounter
				# showMoves uses the value of clickCounter
				showMoves $c $showMovesMoveType $showMovesPosition $showMovesMoveList
				
				$c raise cancelMove
			}
		}
		
	} else {
		# not phase 1
		
		if { $positionId != "" } {
			
			filterPossibleMoves $positionId
			
			if { [llength $pMoves] == 0 } {
				if { $dragging } {
					releaseDrag $c
				} else {
					set pMoves $tempMoves
				}
			} else {
				
				if { $dragging } {
					set dragging 0
					set endCoords [getCoords $c mi-$positionId]
					movePlayingPiece $c $dragPiece $endCoords
				}
				
				if { [llength $pMoves] == 1 && [llength [lindex $pMoves 0]] == [expr $clickCounter + 1] } {
					executeMove $c [lindex $pMoves 0]
				} else {
					incr clickCounter
					
					# call after increasing clickCounter
					# showMoves uses the value of clickCounter
					showMoves $c $showMovesMoveType $showMovesPosition $showMovesMoveList
				
					$c raise cancelMove
				}
			}
			
		} else {
			if { $dragging } {
				releaseDrag $c
			}
		}
	}
}

proc releaseDrag { c } {
	
	global dragPiece dragPiecePositionId
	
	set endCoords [getCoords $c mi-$dragPiecePositionId]
	movePlayingPiece $c $dragPiece $endCoords
	resetMove $c
}

proc showMoves {c moveType position moveList} {
	
	global isGameOver
	if { $isGameOver } { return }
	
	global scale pMoves clickCounter
	
	GS_HideMoves $c $moveType $position $moveList
	
	# filter moves from showMovesMoveList according to users current move selection
	set compareMove [lindex $pMoves 0]
	for {set i [expr [llength $moveList] - 1]} {$i >= 0} {set i [expr $i - 1]} {
		set item [lindex $moveList $i]
		set move [unhashMove [lindex $item 0] 0]
		set remove 0
		for {set j 0} {$j < $clickCounter} {incr j} {
			if { [lindex $compareMove $j] != [lindex $move $j] } {
				set remove 1
			}
		}
		if {$remove} {
			set moveList [lreplace $moveList $i $i]
		}
	}
	
	# keep only the best options
	set tempMoveList [list]
	foreach item $moveList {
		set move [unhashMove [lindex $item 0] 0]
		set positionId [lindex $move $clickCounter]
		set value [lindex $item 1]
		set betterValue 0
		set exists 0
		
		# loop using indexes to allow for item removal
		for {set i [expr [llength $tempMoveList] - 1]} {$i >= 0} {set i [expr $i - 1]} {
			set item2 [lindex $tempMoveList $i]
			set move2 [unhashMove [lindex $item2 0] 0]
			set positionId2 [lindex $move2 $clickCounter]
			if {$positionId == $positionId2} {
				set exists 1
				set value2 [lindex $item2 1]
				if { $value == "Lose" || $value2 == "Win" } {
					set betterValue 1
					set tempMoveList [lreplace $tempMoveList $i $i]
				}
			}
		}
		if {$betterValue || !$exists} {
			lappend tempMoveList $item
		}
	}

	foreach item $tempMoveList {
		
		set move [unhashMove [lindex $item 0] 0]
		set positionId [lindex $move $clickCounter]
		set value [lindex $item 1]
		
		if {$moveType == "all"} {
			set color #81dbfc
		} else {
			if {$value == "Tie"} {
				set color yellow
			} elseif {$value == "Lose"} {
				set color green
			} elseif {$value == "Win"} {
				set color red4
			}
		}
		
		set indicatorCoords [getCoords $c mi-$positionId]
		set cx [lindex $indicatorCoords 0]
		set cy [lindex $indicatorCoords 1]
		set tag [list base valueIndicator vind-$positionId]
		if { [getTagOfOverlappingItem $c $cx $cy playingPiece] != "" } {
			set indicatorRadius [getRadiusGivenScale $scale]
			makeOval $c $cx $cy $tag $indicatorRadius ""
			$c itemconfig vind-$positionId -outline $color -width 3
		} else {
			set indicatorRadius [getRadiusGivenScale [expr $scale - 0.5]]
			makeOval $c $cx $cy $tag $indicatorRadius $color
		}
	}
	
	update idletasks
}

proc getPossibleMoves {} {
	global gPosition
	return [C_PossibleMoves $gPosition]
}

# Remove moves inconsistent with positionId.
proc filterPossibleMoves { positionId } {
	
	global clickCounter pMoves
	
	for {set i [expr [llength $pMoves] - 1]} {$i >= 0} {set i [expr $i - 1]} {
		set move [lindex $pMoves $i]
		if { [lindex $move $clickCounter] != [list $positionId] } {
			set pMoves [lreplace $pMoves $i $i]
		}
	}
	
}

# Returns number of pieces left to be placed for phase 1 to finish
# Value is 11 at first. After first piece is placed, still 11. After second piece, 10.
# Returns 0 when phase 1 is over.
proc getPiecesLeft {} {
	set currentTier [expr [C_CurrentTier]/100]
	return $currentTier
}

proc executeMove { c move } {
	
	global numPositions
	
	set m1 [lindex $move 0]
	set m2 [lindex $move 0]
	set m3 [lindex $move 0]
	
	if { [llength $move] == 2 } {
		set m2 [lindex $move 1]
	} elseif { [llength $move] == 3 } {
		set m2 [lindex $move 1]
		set m3 [lindex $move 2]
	}
	
	$c lower cancelMove
	
	ReturnFromHumanMove [expr $m1*$numPositions*$numPositions + $m2*$numPositions + $m3]
	resetMove $c
}

proc cancelMove { c } {
	
	global clickCounter pMoves
	
	$c lower cancelMove
	
	if { $clickCounter > 0 } {
		set move [lindex $pMoves 0]
		if { [isPhase1] } {
			removePlayingPiece $c [lindex $move 0]
		} else {
			set from [lindex $move 1]
			set to [lindex $move 0]
			
			set indicatorCoords [getCoords $c mi-$from]
			set ix [lindex $indicatorCoords 0]
			set iy [lindex $indicatorCoords 1]
			set playingPiece [getTagOfOverlappingItem $c $ix $iy playingPiece]
			movePlayingPiece $c $playingPiece [getCoords $c mi-$to]
		}
	}
		
	resetMove $c
}

proc cancelMoveRollover { c } {
	$c itemconfigure cancelMoveTxt -fill #6d5447
}

proc cancelMoveRollout { c } {
	$c itemconfigure cancelMoveTxt -fill #42322a
}

proc resetMove { c } {
	
	global clickCounter pMoves
	global showMovesMoveType showMovesPosition showMovesMoveList
	global dragging dragPiece dragPiecePositionId prevDragMove
	
	set clickCounter 0
	set pMoves undefined
	set dragging 0
	set dragPiece ""
	set dragPiecePositionId ""
	set prevDragMove [list]
	
	# call after updating clickCounter
	# showMoves uses the value of clickCounter
	showMoves $c $showMovesMoveType $showMovesPosition $showMovesMoveList
}

proc isPhase1 {} {
	return [expr [getPiecesLeft] > 0]
}

proc getRadiusGivenScale { scale } {
	global squareSize gGameRule
	return [expr $squareSize*$scale/5]
}

# Gets the positionId given the coordinates.
# Returns an empty string if coordiantes are not over a move indicator
proc getPositionId { c x y } {
	# find which move indicator over which the action is performed
	set tag [getTagOfOverlappingItem $c $x $y mi]
	if {$tag == ""} {
		return
	} else {
		return [string range $tag [expr [string first "-" $tag] + 1] end]
	}
}

# Gets the first tag of the first item with a tag name
# starting with prefix within the bounding box specified.
# Returns empty string if prefix not found.
proc getTagOfOverlappingItem {c x1 y1 prefix} {
	set x2 [expr $x1 + 1]
	set y2 [expr $y1 + 1]
	set items [$c find overlapping $x1 $y1 $x2 $y2]
	foreach item $items {
		set tags [$c gettags $item]
		foreach tag $tags {
			if {[string match $prefix* $tag]} {
				return $tag
			}
		}
	}
	return
}

# Gets the coordinates [list x y] of tagId
proc getCoords { c tagId } {
	set coordinates [$c coords $tagId]
	set cx [expr ([lindex $coordinates 0]+[lindex $coordinates 2])/2.0]
	set cy [expr ([lindex $coordinates 1]+[lindex $coordinates 3])/2.0]
	return [list $cx $cy]
}

proc getDimensions { c tagId } {
	set itemCoords [$c coords $tagId]
	set width [expr [lindex $itemCoords 2] - [lindex $itemCoords 0] ]
	set height [expr [lindex $itemCoords 3] - [lindex $itemCoords 1] ]
	return [list $width $height]
}

proc makeOval { c x y tag radius color} {
	$c create oval \
		[expr $x - $radius] \
		[expr $y - $radius] \
		[expr $x + $radius] \
		[expr $y + $radius] \
		-fill $color -tag $tag -outline ""
}

proc getColor { c nameOfPiece } {
	set colors [GS_ColorOfPlayers]
	if {$nameOfPiece == "X"} {
		return [lindex $colors 0]
	} else {
		return [lindex $colors 1]
	}
}

proc makePlayingPiece {c positionId color} {
	
	global playingPieceId squareSize scale
	
	set indicatorCoords [getCoords $c mi-$positionId]
	set cx [lindex $indicatorCoords 0]
	set cy [lindex $indicatorCoords 1]
	
	# The number in the tagname of the playing piece is used only to distinguish
	# the playing pieces, not to represent actual locations.
	makeOval $c $cx $cy [list playingPiece-$playingPieceId deletable] 1 $color
	
	animateScale $c playingPiece-$playingPieceId $cx $cy 2 [expr 2*$squareSize*$scale/5]
	incr playingPieceId
}

proc removePlayingPiece {c positionId} {
	
	set indicatorCoords [getCoords $c mi-$positionId]
	set cx [lindex $indicatorCoords 0]
	set cy [lindex $indicatorCoords 1]
	set removePiece [getTagOfOverlappingItem $c $cx $cy playingPiece]
	
	if { $removePiece != "" } {
		animateScale $c $removePiece $cx $cy [lindex [getDimensions $c $removePiece] 0] 2
		$c delete $removePiece
		update idletasks
	}
}

proc movePlayingPiece {c playingPiece endCoords} {
	
	set startCoords [getCoords $c $playingPiece]
	
	set steps [ScaleDownAnimation 20]
	
	set animDuration [ScaleDownAnimation 1000]
	set numFrames [expr int([ScaleDownAnimation 15])+1]
	set clicksPerFrame [expr $animDuration / $numFrames]
	
	set dx [expr 1.0*([lindex $endCoords 0] - [lindex $startCoords 0])]
	set dy [expr 1.0*([lindex $endCoords 1] - [lindex $startCoords 1])]
	set stepx [expr $dx/$numFrames]
	set stepy [expr $dy/$numFrames]
	
	set currentTime [clock clicks]
	set endTime [expr $currentTime + $animDuration - $clicksPerFrame]
	
	for {set i 0} {$i < $numFrames} {incr i} {
		$c move $playingPiece $stepx $stepy
		update idletasks
	
		set waitClicks [expr int($currentTime + $clicksPerFrame - [clock clicks])]
		if {$waitClicks > 0} {
			after $waitClicks
		}
		set $currentTime [expr $currentTime + $clicksPerFrame]
	}
}

proc animateScale { c tagId cx cy startDiameter trgtDiameter } {
	
	global gAnimationSpeed
	
	set curDiameter $startDiameter
	set currentTime [clock clicks]
	set clicksPerFrame 100
	
	while { [expr abs( $curDiameter - $trgtDiameter )] > 2 } {
		set change [expr 1.0*( $trgtDiameter - $curDiameter )/[expr 12 - 2*$gAnimationSpeed]]
		set curDiameter [expr $curDiameter + $change]
		set ratio [expr $curDiameter/( $curDiameter - $change )]
		$c scale $tagId $cx $cy $ratio $ratio
		update idletasks
		
		set waitClicks [expr $currentTime + $clicksPerFrame - [clock clicks]]
		if {$waitClicks > 0} {
			after $waitClicks
		}
		set $currentTime [expr $currentTime + $clicksPerFrame]
	}
	set change [expr  $trgtDiameter - $curDiameter]
	set ratio [expr $trgtDiameter/( $trgtDiameter - $change )]
	$c scale $tagId $cx $cy $ratio $ratio
	update idletasks
}

proc animateGameOver {c} {
	$c raise gameover
}

proc animateUndoGameOver {c} {
	$c lower gameover
}