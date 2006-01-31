####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################

#############################################################################
# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.
############################################################################

##A COUPLE OF NOTES FOR COLORCODING, INITIAL POSITION...
#WIN, LOSE -> RED4, YELLOW, GREEN

proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "Odd Or Even"

    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition valueMoveColorWin valueMoveColorLose valueMoveColorTie
	#How do you call original C code to determine initial position?
    set gInitialPosition 1150000; #MIGHT HAVE TO CHANGE TO A VARIABLE (C_GetInitialPosition or something...)
    ##set gInitialPosition [C_InitialPosition]
    set gPosition $gInitialPosition
    
    ### Set the different colors to be displayed in showMove!!
    set valueMoveColorWin green
    set valueMoveColorLose red4
    set valueMoveColorTie yellow

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "Player who has an even number of matches (objects) at end game WINS"
    set kMisereString "Player who has an even number of matches (objects) at end game LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Obtain an even number of matches (objects) at the end of the game"
    } else {
	SetToWinString "To Win: Obtain an odd number of matches (objects) at the end of the game"
    }
	#MAY HAVE TO CHANGE, PICK 1 TO 3 OBJECTS SHOULD BE AN ARBITRARY NUMBER AS WE FIXED THE GAME!!!
    SetToMoveString "To Move: Pick n matches (1 to 3) from the row by clicking the nth match head from the right"
		
    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Pai-hsien (Peter) Yu"
    set kTclAuthors "Pai-hsien (Peter) Yu"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
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

    return [list left right]

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

	return [list yellow yellow]
	#since there is no concept of side
    
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

    set standardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	    ]

    # List of all rules, in some order
    set ruleset [list $standardRule]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame"]

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
    # TODO: Needs to change with more variants
    global gMisereGame
    set option 1
    set option [expr $option + (1-$gMisereGame)]
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
    # TODO: Needs to change with more variants
    global gMisereGame
    set option [expr $option - 1]
    set gMisereGame [expr 1-($option%2)]
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

    # you may want to start by setting the size of the canvas; this line isn't cecessary
    #$c configure -width 500 -height 500
    
	global canvasLength canvasWidth gYPosition spaceBetween rectWidth rectLength ovalHWidth ovalHLength backgroundColor
	global pieceColor movablePieceColor
	#Note that spaceBetween will the variable that HandleMove and ShowMove will be using
	set gRows 15
	set canvasLength 500
	set canvasWidth 500
	set gYPosition [expr $canvasLength - 180]
	set myFont Courier
	set backgroundColor grey
	set pieceColor Cyan
	set movablePieceColor magenta
	set spaceScaleFactor 80

	$c create rect 0 0 $canvasWidth $canvasLength -fill $backgroundColor -outline white -tag base
	##$c create rect 0 300 $canvasWidth $canvasLength -fill brown -outline black -tag base

	set rectWidth 3
	set rectLength 15
	set ovalHWidth 20
	set ovalHLength 30
	set iPosition [expr 20 + $spaceScaleFactor / 2]
	set spaceBetween [expr ($canvasWidth-$iPosition-$spaceScaleFactor) / $gRows]
	set h 1
		
	for {set p 0} {$p<$gRows} {incr p} {
		set x [expr $p*$spaceBetween + $iPosition]
		set y $gYPosition
		set w [expr $p + 1]
		set m [expr $p + 1]

		##EITHER DRAW COINS, GOBLETS, OR BALLOONS WILL BE SET THROUGH OPTIONS
		Draw_Matches $c $ovalHWidth $ovalHLength $rectWidth $rectLength $x $y $m
		
	}

} 

proc Draw_Matches { c ovalHWidth ovalHLength rectWidth rectLength xCoord yCoord tagNum } {

		global attachFactor MaxMove 
		set placeBelow 45
		
		##Variable MaxMove
		##Probably not as descriptive as it should be, it is an indication of where the binding of the bottom stick
		## of our matches should start
		##NOTE: MAXMOVE WILL ULTIMATELY BE DEPENDENT ON THE USER INPUT OF THE NUMBER OF MATCHES
		set MaxMove 20
	 	
		global canvasLength canvasWidth gYPosition spaceBetween backgroundColor pieceColor 
		$c create oval [expr $xCoord - $ovalHWidth] [expr $yCoord - $ovalHLength] \
			     [expr $xCoord + $ovalHWidth] [expr $yCoord + $ovalHLength] \
		-outline black -fill $pieceColor -tag move-$tagNum
		$c create rect [expr $xCoord-$rectWidth] [expr $yCoord + $placeBelow -$rectLength] [expr $xCoord+$rectWidth] [expr $yCoord + $placeBelow +$rectLength] \
		-outline black -fill brown -tag move-[expr $tagNum + $MaxMove]

}

proc Raise_Match { c m } {

	global MaxMove	

	$c raise move-$m
	$c raise move-[expr $m + $MaxMove]
}

#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why this
# is here, so whoever put this here should update this.  -Jeff
#############################################################################
proc GS_Deinitialize { c } {
    $c delete all
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
    
    ### TODO: Fill this in

	global canvasLength canvasWidth gYPosition backgroundColor gRows iPosition

	#try it out...DON'T KNOW WHETHER WE'RE ALLOWED TO DO THIS...
	#global variables extracted directly from gamesman3.tcl
	global gLeftName gRightName
	#end try it out

	set gRows [expr ($position/10000)%100]
	set canvasLength 500
	set canvasWidth 500
	set gYPosition [expr $canvasLength - 200]
	set myFont Courier
	set currentTurn [expr ($position/1000000)*100] 
	
	$c create rect 0 0 $canvasWidth $canvasLength -fill $backgroundColor -outline white -tag base
	$c create text 100 80 \
	-text "$gLeftName: [expr ($position/100)%100] " -font {{$myFont} 14} -justify center 
	$c create text 330 80 \
	-text "$gRightName: [expr $position%100]" -font {{$myFont} 14} -justify center 
	

	for {set p 0} {$p<$gRows} {incr p} {
		set m [expr $p + 1]
		Raise_Match $c $m;
	}	
}


#############################################################################
# GS_NewGame should start playing the game. 
# It's arguments are a canvas, c, where you should draw 
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.
#############################################################################
proc GS_NewGame { c position } {
    # TODO: The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
      GS_DrawPosition $c $position
}


#############################################################################
# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
#############################################################################
proc GS_WhoseMove { position } {
    # Optional Procedure
	set temp [expr $position / 1000000]
	if ($temp == 1)
		return "left"
	else
		return "right"
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

	global maxframes xSpacePerMove ySpacePerMove attachFactor
	### TODO: Fill this in
    ##Two for loops, one for the number of moves, one for the moving matches...
	set oldNumberMatches [expr ($oldPosition / 10000) % 100]
	set newNumberMatches [expr ($newPosition / 10000) % 100]

##NEED TO PERFECT ANIMATION
##1. IT WOULD NEED TO MOVE BOTH UP-DOWN AND LEFT-RIGHT
##2. NEED TO CONTROL THE SPEED OF THE MOVEMENTS
	
	##Maxframe, control number of animations executed!!!
      set maxframes 300
	set xSpacePerMove 1
	set ySpacePerMove 0
	set xBackgroundMove 1
	set yBackgroundMove 0
	set maxReturnFrames [expr $maxframes / $xBackgroundMove]

	Animate_Match_Move $c $maxframes $oldNumberMatches $newNumberMatches $xSpacePerMove $ySpacePerMove	
	GS_DrawPosition $c $newPosition
	
	Animate_Match_Move_Back $c $maxReturnFrames $oldNumberMatches $newNumberMatches $xBackgroundMove $yBackgroundMove
}
 
proc Animate_Match_Move { c maxframes oldNumberMatches newNumberMatches xSpacePerMove ySpacePerMove} {
	global MaxMove
 
	for {set frame 0} {$frame < $maxframes} {incr frame} {
		for {set i $oldNumberMatches} {$i > $newNumberMatches} {set i [expr $i - 1]} {
			$c move move-$i $xSpacePerMove $ySpacePerMove
			$c move move-[expr $i + $MaxMove] $xSpacePerMove $ySpacePerMove
		}
		update idletasks
	}
}

proc Animate_Match_Move_Back { c maxReturnFrames oldNumberMatches newNumberMatches xBackgroundMove yBackgroundMove} {
	global MaxMove

	##TO MOVE THE PIECES IN THE BACKGROUND BACK INTO ITS ORIGINAL POSITION!!!
	for {set frame 0} {$frame < $maxReturnFrames} {incr frame} {
		for {set i $oldNumberMatches} {$i > $newNumberMatches} {set i [expr $i - 1]} {
			$c move move-$i [expr 0 - $xBackgroundMove] $yBackgroundMove
			$c move move-[expr $i + $MaxMove] [expr 0 - $xBackgroundMove] $yBackgroundMove
		}
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
# Example:  moveList: { 73 Win } { 158 Lose } { 22 Tie } 
#############################################################################
proc GS_ShowMoves { c moveType position moveList } {

	##*****************************************************
	##PROBLEM!!!
	##Because of the way I bind the moves, if you Undo with Value turned on, then you are going to see the
	##values of every three moves because it doesn't erase!!!
	##ANY WAY TO "UNBIND" THE VALUE COLORS???? (ASK!!!!!!!!!!!!!!!!!)
	##*****************************************************
	global valueMoveColorLose
      global valueMoveColorWin
	global valueMoveColorTie
	global movablePieceColor
	global pieceColor

      set currentMatches [expr ($position/10000)%100]
	foreach item $moveList {
		set theMove  [lindex $item 0]
		set value [lindex $item 1]	
		set color $pieceColor
		
		set m [expr $currentMatches - $theMove%100 + 1]
		$c bind move-$m <ButtonRelease-1> "ReturnFromHumanMove $theMove"
		$c bind move-$m <Enter> "Mouseover_Move $c $position $m"
		$c bind move-$m <Leave> "Mouseleave_Move $c $position $m $moveType $value"

		if {$moveType == "value"} {
	    		if {$value == "Tie"} {
				set color $valueMoveColorTie
	    		}
			if {$value == "Lose"} {
				set color $valueMoveColorWin
	    		}
			if {$value == "Win"} {
				set color $valueMoveColorLose
	 	   	}
			
		}
		$c itemconfigure move-$m -fill $color
	}
}

##DOESN'T WORK
##RIGHT NOW EVEN THE BOTTOM STICKS WILL LIGHT UP!!
proc Mouseover_Move {c currentPosition moveNumber} {
	global movablePieceColor

	##replace constants with variables in C code!!!
	set currentMatches [expr ($currentPosition / 10000) % 100] 
	set i $moveNumber
	set color $movablePieceColor

	while { $i <= $currentMatches } {
		$c itemconfigure move-$i -fill $color
		set i [expr $i + 1]
	}

	##FOR TESTING PURPOSES:
	#$c create text 250 200 \
	#-text "currentMatches = $currentMatches" -font {{Arial} 12} -fill black
	#$c create text 250 250 \
	#-text "i = $i" -font {{Arial} 12} -fill black
	
}

##doesn't work!!!
proc Mouseleave_Move {c currentPosition moveNumber moveType value} {
	global pieceColor valueMoveColorTie valueMoveColorWin valueMoveColorLose

	##replace constants with variables in C code!!!
	set currentMatches [expr ($currentPosition / 10000) % 100] 
	set i $moveNumber
	set color $pieceColor

	while { $i <= $currentMatches } {
		if {$moveType == "value"} {
	    		if {$value == "Tie"} {
				set color $valueMoveColorTie
	    		}
			if {$value == "Lose"} {
				set color $valueMoveColorWin
	    		}
			if {$value == "Win"} {
				set color $valueMoveColorLose
	 	   	}
			
		}
		$c itemconfigure move-$i -fill $color
		set i [expr $i + 1]
	}
}

#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    ### TODO: Fill this in

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

    
    GS_DrawPosition $c $positionAfterUndo
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

	$c create text 250 200 \
	-text "Great Job $nameOfWinner" -font {{Arial} 30} -fill green
	#Add more features...
	#For Testing only
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

	### TODO if needed

}