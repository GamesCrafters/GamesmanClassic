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
    set gPosition $gInitialPosition
    
    ### Set the different colors to be displayed in showMove!!
    set valueMoveColorWin green
    set valueMoveColorLose red4
    set valueMoveColorTie yellow

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "Player who has an even number of objects (matches) at end game WINS"
    set kMisereString "Player who has an even number of objects (matches) at end game LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Obtain an even number of objects (matches) at the end of the game"
    } else {
	SetToWinString "To Win: Obtain an odd number of objects (matches) at the end of the game"
    }
	#MAY HAVE TO CHANGE, PICK 1 TO 3 OBJECTS SHOULD BE AN ARBITRARY NUMBER AS WE FIXED THE GAME!!!
    SetToMoveString "To Move: Pick 1 to 3 objects (matches) from the row"
		
    
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
    $c configure -width 500 -height 500
    
	global canvasLength canvasWidth gYPosition spaceBetween rectWidth rectLength ovalHWidth ovalHLength backgroundColor
	#Note that spaceBetween will the variable that HandleMove and ShowMove will be using
	set gRows 15
	set canvasLength 500
	set canvasWidth 500
	set gYPosition [expr $canvasLength - 200]
	set myFont Courier
	set backgroundColor aquamarine

	$c create rect 0 0 $canvasWidth $canvasLength -fill $backgroundColor -outline white -tag base
	#$c create oval [expr 200 - $canvasWidth/1.2] [expr 400 - $canvasLength/3] \
	#			[expr 200 + $canvasWidth/1.2] [expr 400 + $canvasLength/3] -fill green -outline black -tag base
	#$c create text 50 80 \
	#-text "First Player: 0"
	#$c create text 330 80 \
	#-text "Second Player: 0"

	set rectWidth 5
	set rectLength 90
	set ovalHWidth 7
	set ovalHLength 12
	set iPosition 20
	set spaceBetween [expr ($canvasWidth-$iPosition) / $gRows]
	set h 1
		
	for {set p 0} {$p<$gRows} {incr p} {
		set x [expr $p*$spaceBetween + $iPosition]
		set y $gYPosition
		set w [expr $p + 1]
		set m [expr $p + 1]
		
		#TESTING move-$m
		$c create rect $x $y [expr $x + $rectWidth] [expr $y + $rectLength] \
		-outline black -fill yellow -tag move-$m
		$c create oval [expr ($x+$x+$rectWidth) / 2 - $ovalHWidth] [expr $y+10-$ovalHLength] \
					[expr ($x+$x+$rectWidth) / 2 + $ovalHWidth] [expr $y+10+$ovalHLength] \
		-outline black -fill red  -tag move-$m
		#Move this to ShowMove!!!
		#ReturnFromHumanMove -> pass a "move" that clicking on that match represents...
		#Find out where the move is carried out and have to change it!!!
	}

	#THIS PORTION IS FOR TEST OF ANIMATIONS FOR NEW GAME
	#Is there any way to maybe hold the input so that it will move much slower?
	for {set k 1} {$k<500} {set k [expr $k + 1]} {
		$c create text [expr $k - 1] 80 \
		-text "Odd Or Even" -font {{Times New Roman} 14} -fill $backgroundColor
		$c create text $k 80 \
		-text "Odd Or Even" -font {{Times New Roman} 14}
	}
	update idletasks
	#END OF TEST OF ANIMATION

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
	set gRows [expr ($position/10000)%100]
	set canvasLength 500
	set canvasWidth 500
	set gYPosition [expr $canvasLength - 200]
	set myFont Courier
	set currentTurn [expr ($position/1000000)*100] 
	
	##Remember to remove all these hard-coded portion and replace it with variables
	#if {$temp == 1} {
	#	set currentTurn 200
	#}
	#if {$temp == 2} {
#		set currentTurn 100
	#}

	$c create rect 0 0 $canvasWidth $canvasLength -fill $backgroundColor -outline white -tag base
	$c create text 50 80 \
	-text "First Player: [expr ($position/100)%100] "
	$c create text 330 80 \
	-text "Second Player:  [expr $position%100]"

	for {set p 0} {$p<$gRows} {incr p} {
		set m [expr $p + 1]
		$c raise move-$m
	} 

	#for {set p 1} {$p<=$gRows && $p<=3} {incr p} {
	#	set m [expr $gRows - $p + 1]
	#	$c bind move-$m <ButtonRelease-1> "ReturnFromHumanMove [expr $currentTurn + $p]"
	#}

	
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

	### TODO: Fill this in
    ##Two for loops, one for the number of moves, one for the moving matches...
	set oldNumberMatches [expr ($oldPosition / 10000) % 100]
	set newNumberMatches [expr ($newPosition / 10000) % 100]
	
	#for {set i oldNumberMatches} {$i > newNumberMatches} {decr i} {
	#	set x [expr oldNumberMatches*spaceBetween + iPosition]
	#	set y $gYPosition
	#	#animation portion
	#	for {set x $x} {$x < 500} {incr x} {
	#		$c create rect $x $y [expr $x + $rectWidth] [expr $y + $rectLength] \
	#		-outline black -fill yellow
	#		$c create oval [expr ($x+$x+$rectWidth) / 2 - $ovalHWidth] [expr $y+10-$ovalHLength] \
	#					[expr ($x+$x+$rectWidth) / 2 + $ovalHWidth] [expr $y+10+$ovalHLength] \
	#		-outline black -fill $backgroundColor
	#		$c create rect $x $y [expr $x + $rectWidth] [expr $y + $rectLength] \
	#		-outline black -fill yellow
	#		$c create oval [expr ($x+$x+$rectWidth) / 2 - $ovalHWidth] [expr $y+10-$ovalHLength] \
	#					[expr ($x+$x+$rectWidth) / 2 + $ovalHWidth] [expr $y+10+$ovalHLength] \
	#		-outline black -fill $backgroundColor
	#	}
	#}
	#end
	
	for {set i $oldNumberMatches} {$i > $newNumberMatches} {set i [expr $i - 1]} {
		$c lower move-$i
	}
	GS_DrawPosition $c $newPosition
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

	global valueMoveColorLose
      global valueMoveColorWin
	global valueMoveColorTie

      set currentMatches [expr ($position/10000)%100]
	foreach item $moveList {
		set theMove  [lindex $item 0]
		set value [lindex $item 1]	
		#set color white

		#Wierd, the command "else" generate an error for unknown command 'else'
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

		set m [expr $currentMatches - $theMove%100 + 1]
		$c bind move-$m <ButtonRelease-1> "ReturnFromHumanMove $theMove"
		#$c itemconfigure move-$m -fill $color

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

    ### TODO if needed
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
