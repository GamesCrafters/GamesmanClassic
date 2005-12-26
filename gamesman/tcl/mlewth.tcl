#############################################################################
# 
# mlewth.tcl 
#
# C authored by Yuliya Sarkisyan and Cindy Song
# Tcl/Tk authored by Yuliya Sarkisyan, Cindy Song, Suthee Chaidaroon 
# 
# Note: Yuliya, Cindy: were original author
# 	Suthee Chaidaroon: Added animation with speed.
#			   Fixed a board drawing function  
#			   Added undo
#			   Added Winning text
#			   Implemented a multi-pieces move variant
# Bug Report:
#	The mlewth.c does not return a correct moveList when it handles the move
#	around the conner and edge. So we will see some extra arrows. 
#############################################################################

#############################################################################
# All Initial setup will be invoked here
#############################################################################

# Set Board dimensions
global height width boardSize 

# Board information (needed for Hashing)
global numPositions numSlots numPieces

# piece representations
global X O

# Space position (needed for showmove and animation)
global spaceI spaceJ 

# for handle undo
global undomake
set undomake "OFF"

# for gameover
global wintext gameover

###########################################
# Set up variables for drawing the board
###########################################

# board variable
global base
global pieceGap vertGap horizGap 
global tileSize

set pieceGap 5
set height 5
set width 3

# Arrow 
global arrowLength lineWidth ArWFactor ArShpLst
global ArPosOffsetX ArPosOffsetY ArGap
global left right up down

set up 0
set down 1
set left 3
set right 2
set arrowLength 38
set lineWidth 2
set ArWFactor 3 
set ArPosOffsetX 21
set ArPosOffsetY 21
set ArShpLst [list 8 8 5]
set ArGap 15

# color set up
global xColor oColor lineColor baseColor

set xColor red
set oColor blue
set lineColor CadetBlue4
set baseColor lightgrey
set X 4
set O 5

#animation
global basespeed
set basespeed 200

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
    
    global gFrameHeight gFrameWidth
    global numSlots numPieces
    global numPositions
   
    ### Set the name of the game
    global kGameName
    set kGameName "Lewthwaite's Game"
    
    ### Set the initial position of the board (default 0)
    global gInitialPosition gPosition
    set gInitialPosition [C_InitialPosition]
    
    set gInitialPosition 31193
    set gPosition $gInitialPosition
    
    # game varible specific setup
    global boardSize height width base pieceGap vertGap horizGap tileSize 
    
    set boardSize [max $width $height]
    set tileSize [expr ($gFrameHeight - 2*50)/$boardSize]
    set horizGap [expr ($gFrameWidth - $width*$tileSize)/2]
    set vertGap  [expr ($gFrameHeight - $height*$tileSize)/2]
    set numSlots $width*$height
    set numPieces [expr ($numSlots - 1)/2]
    set numPositions 	[expr 2 * [factorial $numSlots]\
    			 /[factorial $numPieces]\
    			 /[factorial $numPieces]\
			 /[factorial [expr $numSlots - 2*$numPieces]]]

    ### Set the strings to be used in the Edit Rules
    global kStandardString kMisereString
    set stdStr "To Win: have your opponent to be"
    set misStr "To Win: be "
    set gameEnd "the first player who has no moves "
    set kStandardString [concat $stdStr $gameEnd]
    set kMisereString [concat $misStr $gameEnd]
    
    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section
    global gMisereGame
   
    if {!$gMisereGame} {
	SetToWinString $kStandardString
    } else {
	SetToWinString $kMisereString
    }
    SetToMoveString "To Move: Choose one of your pieces to the left, right, \
                     top, or bottom of the empty space to move into the \
                     space. (With standard rules, you can only move one \
                     piece at a time.)"
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Yuliya Sarkisyan, Cindy Song"
    set kTclAuthors "Yuliya Sarkisyan, Cindy Song, Suthee Chaidaroon"
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
    return [list X O]
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
    return [list red blue]
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

    set multipleMoves \
	[list \
	     "Would you like to be able to slide multiple pieces:" \
	     "No" \
	     "Yes" \
	     ]
	  
    # List of all rules, in some order
    set ruleset [list $standardRule $multipleMoves]

    # Declare and initialize rule globals
    global gMisereGame multiplePieceMoves
    set gMisereGame 0
    set multiplePieceMoves 0
    
    # board dimension global
    global heightflag widthflag
    set heightflag 1
    set widthflag  0

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "multiplePieceMoves"]

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
    global gMisereGame multiplePieceMoves heightflag widthflag
    set option 0
    set option [expr $option + $gMisereGame + 2*$multiplePieceMoves]
    
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
    
    global gMisereGame multiplePieceMoves heightflag widthflag height width
    set option 0
    set gMisereGame [expr $option%2]
    set multiplePieceMoves [expr ($option/2)%2]
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
    
    global gFrameWidth gFrameHeight 
    
    # Save the GC background skin
    $c addtag background all

    # draw a canvas with a default width and height
    $c configure -width $gFrameWidth -height $gFrameHeight
   
    # clean up all objects (except a background) on canvas before drawing a new board
    $c delete {!background}
    
    # drawing a board
    makeBoard $c
} 

#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why 
# this is here, so whoever put this here should update this.  -Jeff
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
# BY THE WAY: Before you go any further, I recommend writing a tcl function 
# that UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.
#############################################################################
proc GS_DrawPosition { c position } {
    
    global X O height width numSlots spaceI spaceJ
    
    # get a board's string representation
    set boardList [C_GenericUnhash $position $numSlots]
  
    $c raise base all
    for {set i 0} {$i < $height} {incr i} {
	for {set j 0} {$j < $width} {incr j} {
	    set piece [string index $boardList [expr $i*$width + $j]]
	    if { [string compare $piece "O"] == 0 } {
		$c itemconfig piece-$i$j -fill blue
		$c raise piece-$i$j base
	    } elseif { [string compare $piece "X"] == 0 } {
		$c itemconfig piece-$i$j -fill red
		$c raise piece-$i$j base
	    } elseif { [string compare $piece "-"] == 0 } {
		set spaceI $i
		set spaceJ $j
	    }
	}
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
    GS_Deinitialize $c
    GS_Initialize $c

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
    return ""    
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
	
	global numSlots width height X O spaceI spaceJ
	global xColor oColor
	global undomake
	
	set oldboard [C_GenericUnhash $oldPosition $numSlots]
	set newboard [C_GenericUnhash $newPosition $numSlots]
	
	# Calculate the direction the piece will move to
	for {set j 0} {$j < $height} {incr j} {
		for {set i 0} {$i < $width} {incr i} {
			set index [expr $j * $width + $i]
			set space [string index $oldboard $index]
			if {[string compare $space "-"] == 0} {
				set spacex $i
				set spacey $j
			}
		}
	}
	
	# Calculate the direction the piece will move to
	for {set j 0} {$j < $height} {incr j} {
		for {set i 0} {$i < $width} {incr i} {
			set index [expr $j * $width + $i]
			set space [string index $newboard $index]
			if {[string compare $space "-"] == 0} {
				set spacenewx $i
				set spacenewy $j
			}
		}
	}
	
	# get a direction number representation
	set d [getDirection $theMove]
	set numPiece [getNumPieces $theMove]
	
	#if it is undo, we need to reverse a direction
	if {$undomake == "ON"} {
		switch $d {
			"0" { set d 1}
			"1" { set d 0}
			"2" { set d 3}
			"3" { set d 2}
			default { puts "NOT MATCH"}
		}
		# if undo was called, asserted the undo flag back to OFF
		set undomake "OFF"
   	}
	
	# get a list of color 
	set lcolor [getListColor $oldboard $d $spacex $spacey]
	# determine which piece we have to handle its animation
	set movepiece [string index $oldboard [expr $spacenewy * $width + $spacenewx]]
	
	# handle a color of moving piece
	if {$movepiece == "O"} { 
		$c itemconfig piece-$spacey$spacex -fill blue -outline black
		set color $oColor
	} else {
		$c itemconfig piece-$spacey$spacex -fill red -outline black
		set color $xColor
	}
	
	# slide the piece (doing an animation)
	doAnimation $c $lcolor $spacex $spacey $spacenewx $spacenewy $numPiece
	
	GS_DrawPosition $c $newPosition  
	# arrow handles (passing the space's new position to ShowMove procedure)
	$c lower arrow all
	set spaceJ $spacenewx
	set spaceI $spacenewy
}
	
#############################################################################
# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever 
# the player clicks to make the move)  It is also the function that handles 
# coloring of the moves according to value. It is called by gamesman just 
# before the player is prompted for a move.
#
# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which 
#    radio button is down
# position = the current hashed position
# moveList = a list of lists.  Each list contains a move and its value.
# These moves are represented as numbers (same as in C)
# The value will be either "Win" "Lose" or "Tie"
# Example:  moveList: { 73 Win } { 158 Lose } { 22 Tie } 
#############################################################################
proc GS_ShowMoves { c moveType position moveList } {

    global spaceI spaceJ numSlots tileSize ArGap
    
    set lnummove [getNumMove $moveList]
    set numup [lindex $lnummove 0]
    set numdw [lindex $lnummove 1]
    set numrg [lindex $lnummove 2]
    set numlf [lindex $lnummove 3]
    
    # the maximum arrow for each direction
    set valup [lindex $lnummove 0]
    set valdw [lindex $lnummove 1]
    set valrg [lindex $lnummove 2]
    set vallf [lindex $lnummove 3]
    
    # repeat drawing an arrow
    foreach item $moveList {
    	set move  [lindex $item 0]
    	set value [lindex $item 1]
    	set color cyan
    	set direction [getDirection $move]
    	
    	if {$moveType == "value"} {
    	    if {$value == "Tie"} {
    		set color yellow
    	    } elseif {$value == "Lose"} { ;# lose for opponent, win for you
    		set color green
    	    } else {
    		set color red4
    	    }
    	}
    	
    	set name arrow-$move-$spaceI-$spaceJ
    	$c bind $name <ButtonRelease-1> "ReturnFromHumanMove $move"
	$c bind $name <Enter> "SetColour $c $name black"
    	$c bind $name <Leave> "SetColour $c $name $color"
    	
    	set i $spaceI
    	set j $spaceJ
    	set len [expr ([getNumPieces $move] - 1) * 80]
    	global vertGap horizGap arrowLength
    	# shift arrow so that a multiple arrow will be packed nicely
    	switch $direction {
		"0" {
			# move back to an original position first
			$c coords $name [expr $horizGap + $j * $tileSize] \
		       			[expr $vertGap + ($i+1) * $tileSize + 20 + $len] \
		       			[expr $horizGap + $j * $tileSize] \
		       			[expr $vertGap + ($i+1) * $tileSize - $arrowLength + 20]
			# move to an appropriate offset
			$c move $name [expr ($tileSize / 2) - $ArGap * ($valup - $numup)] 0
			set numup [expr $numup - 1]
		    }
		"1" {
			$c coords $name [expr $horizGap + $j * $tileSize] \
					[expr $vertGap + $i * $tileSize - 20 - $len] \
					[expr $horizGap + $j * $tileSize] \
		                        [expr $vertGap + $i * $tileSize + $arrowLength - 20]
			$c move $name [expr ($tileSize / 2) - $ArGap * ($valdw - $numdw)] 0
			set numdw [expr $numdw - 1]
		    }
		"2" {
			$c coords $name [expr $horizGap + ($j+1) * $tileSize + 20 + $len] \
		       			[expr $vertGap + $i * $tileSize] \
		       			[expr $horizGap + ($j+1) * $tileSize - $arrowLength + 20] \
		       			[expr $vertGap + $i * $tileSize] 
			$c move $name 0 [expr ($tileSize / 2) - $ArGap * ($valrg - $numrg)]
			set numrg [expr $numrg - 1]
		    }
		"3" {
			$c coords $name [expr $horizGap + $j * $tileSize - 20 - $len]\
				        [expr $vertGap + $i * $tileSize]\
				        [expr $horizGap + $j * $tileSize + $arrowLength - 20] \
				        [expr $vertGap + $i * $tileSize]
			$c move $name 0 [expr ($tileSize / 2) - $ArGap * ($vallf - $numlf)]
			set numlf [expr $numlf - 1]
		    }
	}
	
    	$c itemconfig $name -fill $color
    	$c raise $name 
        update idletasks
    }
}

#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {
    GS_DrawPosition $c $position
}


#############################################################################
# GS_HandleUndo handles undoing a move (possibly with animation)
# Here's the undo logic
# The game was in position A, a player makes move M bringing the game to 
# position B then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A
#
# By default this function just calls GS_DrawPosition, but you certainly don't 
# need to keep that.
#############################################################################
proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {
	
    global undomake
	
    # initial the undomake 
    set undomake "ON"
    GS_HandleMove $c $currentPosition $theMoveToUndo $positionAfterUndo 
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
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner \
		       lastMove} {

    global wintext gameover gFrameWidth gFrameHeight
	
    set size [min $gFrameWidth $gFrameHeight]
    
    # create a text and box
    set wintext  [$c create rectangle 0 [expr $size/2 - 50] $size [expr $size/2 + 50] -fill gray -width 1 -outline black]
    set gameover [$c create text [expr $size/2] [expr $size/2] -text "Game Over! $nameOfWinner Wins" -font "Arial 18" -fill black]

}


#############################################################################
# GS_UndoGameOver is called when the player hits undo after the game is 
# finished.  This is provided so that you may undo the drawing you did in 
# GS_GameOver if you drew something. For instance, if you drew a line crossing 
# out the winning row in tic tac toe, this is where you sould delete the line.
#
# note: GS_HandleUndo is called regardless of whether the move undoes the end 
# of the game, so IF you choose to do nothing in GS_GameOver, you needn't do 
# anything here either.
#############################################################################
proc GS_UndoGameOver { c position } {

    global wintext gameover
    
    # deleted a text and box
    $c delete $gameover
    $c delete $wintext
}



#############################################################################
# makeBoard
# 
# It draw a board and instantiate all objects that will be needed for 
# animatation and showmove
# param:
# 	c - a canvas to be draw on
#	
#############################################################################
proc makeBoard { c } {
    global lineWidth lineColor baseColor base
    global gFrameWidth gFrameHeight

    global height width boardSize
    global vertGap horizGap 
    global tileSize
    global left right up down
    global X O
	
    set base \
	[$c create rectangle \
	     0 \
	     0 \
	     [expr $gFrameWidth- 1] \
	     [expr $gFrameHeight - 1] \
	     -fill $baseColor -tag base];


    # create horizontal lines
    for {set i 0} {$i <= $height } {incr i} {
	set horiz-$i  \
	    [$c create line \
		 $horizGap \
		 [expr $vertGap + $i * $tileSize] \
		 [expr $gFrameWidth- $horizGap] \
		 [expr $vertGap + $i * $tileSize] \
		 -fill $lineColor -width $lineWidth -tags [list base line]]; 	
    }

    # create vertical lines
    for {set i 0} {$i <= $width} {incr i} {
	set vert-$i  \
	    [$c create line \
		 [expr $horizGap + $i * $tileSize] \
		 $vertGap \
		 [expr $horizGap + $i * $tileSize] \
		 [expr $gFrameHeight- $vertGap] \
		 -fill $lineColor -width $lineWidth -tags [list base line]];
	
    }

    # create pieces
    for {set i 0} {$i < $height} {incr i} {
	for {set j 0} {$j < $width} {incr j} {
	    drawCircle $c $i $j "black" "off" 0 0
	}
    }

    # create arrows
    for {set i 0} {$i < $height} {incr i} {
    	for {set j 0} {$j < $width} {incr j} {
	#down
	    for {set k 0} {$k < $i} {incr k} {
		    set movetype [hashMove $down [expr $k + 1]]
		    drawArrow $c $down $i $j arrow-$movetype-$i-$j \
			      [expr $k * 15] 0 0 [expr $k * 80]
	    }

	#up
	    for {set k 0} {$k < [expr $height - $i - 1]} {incr k} {
		    set movetype [hashMove $up [expr $k + 1]]
		    drawArrow $c $up $i $j arrow-$movetype-$i-$j \
			      [expr $k * 15] 0 0 [expr $k * 80]
	    }

	 #left
	    for {set k 0} {$k < $j} {incr k} {
		    set movetype [hashMove $left [expr $k + 1]]
		    drawArrow $c $left $i $j arrow-$movetype-$i-$j \
			      0 [expr $k * 15] [expr $k * 80] 0
	    }
    	    
	 #right
	    for {set k 0} {$k < [expr $width - $j - 1]} {incr k} {
		    set movetype [hashMove $right [expr $k + 1]]
		    drawArrow $c $right $i $j arrow-$movetype-$i-$j \
			      0 [expr $k * 15] [expr $k * 80] 0
	    }
    	}
    } 	 

    ###  raise the base over the pieces
    $c lower arrow all
    $c raise base all

} 
		 
#############################################################################
# Draws an arrows on canvas c in slot (i,j). Type is one of leftArrow, 
# rightArrow, upArrow, downArrow.
#############################################################################
proc drawArrow { c type i j name {offsetx 0} {offsety 0} {lenx 0} {leny 0}} {
    
    global height width 
    global vertGap horizGap 
    global tileSize 
    global arrowLength left right up down
    global lineWidth ArWFactor ArShpLst ArPosOffsetX ArPosOffsetY
    
    if { $type == $left } {
    	set $name [$c create line \
    		       [expr $horizGap + $j * $tileSize - 20 - $lenx]\
    		       [expr $vertGap + $i * $tileSize]\
    		       [expr $horizGap + $j * $tileSize + $arrowLength - 20] \
    		       [expr $vertGap + $i * $tileSize] \
    		       -tags [list $name arrow] \
    		       -width [expr $lineWidth * $ArWFactor] -arrow last \
    		       -arrowshape $ArShpLst]
    } 
    
    if { $type == $right } {
	set $name [$c create line \
		       [expr $horizGap + ($j+1) * $tileSize + 20 + $lenx] \
		       [expr $vertGap + $i * $tileSize] \
		       [expr $horizGap + ($j+1) * $tileSize - $arrowLength + 20] \
		       [expr $vertGap + $i * $tileSize] \
		       -tags [list $name arrow] \
		       -width [expr $lineWidth * $ArWFactor] -arrow last \
		       -arrowshape $ArShpLst]
    }
    if { $type == $up } {
	set $name [$c create line \
		       [expr $horizGap + $j * $tileSize] \
		       [expr $vertGap + ($i+1) * $tileSize + 20 + $leny] \
		       [expr $horizGap + $j * $tileSize] \
		       [expr $vertGap + ($i+1) * $tileSize - $arrowLength + 20] \
		       -tags [list $name arrow] \
		       -width [expr $lineWidth * $ArWFactor] -arrow last \
		       -arrowshape $ArShpLst]
    }
    if { $type == $down } {
	set $name [$c create line \
		       [expr $horizGap + $j * $tileSize] \
		       [expr $vertGap + $i * $tileSize - 20 - $leny] \
		       [expr $horizGap + $j * $tileSize] \
		       [expr $vertGap + $i * $tileSize + $arrowLength - 20] \
		       -tags [list $name arrow] \
		       -width [expr $lineWidth * $ArWFactor] -arrow last \
		       -arrowshape $ArShpLst]
    }
  
}

#############################################################################
# Draws a circle on canvas c in slot (i,j). fill with color and tag with either
#	piece-$i$j or just a temp tag and offsetx and y
# param:
#	i is a y coordinate 
#	j is an x coordinate
#	tag is a flag to force a tag name to temp-$iden (tags == "on")
#	iden helps us draw multiple temporaly circle namely temp-$iden
#	offsetx used for animation (how much a circle offsets from its orginal position
#	offsety same as offsetx but used for y-coordinate
#############################################################################
proc drawCircle {c i j {color "black"} {tags "off"} {iden 0} {offsetx 0} {offsety 0}} {
	global X O
    	global horizGap vertGap pieceGap tileSize
    	global xColor oColor
    	
    	# check if it is a temp circle (for animation)
	if {$tags == "off"} {
		set nametags piece-$i$j
	} else {
		set nametags "temp-$iden"
	}
	
	# calculate a position for each circle
	set xstart [expr $horizGap + $j * $tileSize + $pieceGap]
	set ystart [expr $vertGap + $i * $tileSize + $pieceGap]
	set xend   [expr $horizGap + ($j+1) * $tileSize - $pieceGap]
	set yend   [expr $vertGap + ($i+1) * $tileSize - $pieceGap]
	
	# drawing a circle onto a canvas
	$c create oval [expr $xstart + $offsetx] \
		       [expr $ystart + $offsety] \
		       [expr $xend + $offsetx] \
		       [expr $yend + $offsety] \
		       -fill $color -tag $nametags
		 
	$c lower $nametags
}

#############################################################################
# doAnimation is called by GS_HandleMove() 
# 
# It animates a piece from an original position (oldx, oldy) 
# to a destination (newx, newy)
# param:
# 	c - a canvas to be draw on
#	lcolor - a list of color to be drawn
#	oldx, oldy - an origin
#	newx, newy - a destination
#############################################################################
proc doAnimation { c lcolor oldx oldy newx newy {numberpiece 1}} {
    
    global basespeed gAnimationSpeed tileSize 
    	
    set speed [expr $basespeed / pow(2, $gAnimationSpeed)]
    set signx [neg [expr $oldx - $newx]]
    set signy [neg [expr $oldy - $newy]]
    set dx [expr $signx * $tileSize / $speed]
    set dy [expr $signy * $tileSize / $speed]
     
    # draw a temporaly circle
    for {set i 0} {$i < $numberpiece} {incr i} {
	set offsetx [expr $signx * $i]
	set offsety [expr $signy * $i]
	set color [lindex $lcolor $i]
	set px [expr $newx + $offsetx]
	set py [expr $newy + $offsety]
	# remove a current piece first
    	$c lower piece-$py$px
    	drawCircle $c $py $px $color "on" $i
    	$c raise temp-$i
    }
    
    # moving a circle
    for {set k 1} {$k < $speed} {incr k} {
    	for {set j 0} {$j < $numberpiece} {incr j} {
    		$c move temp-$j $dx $dy
	}
    	after 1
	update idletasks
    }
    
    # delete all temporaly circle
    for {set j 0} {$j < $numberpiece} {incr j} {
	$c delete temp-$j
    }
}
#############################################################################
# getListColor - get a list of color from the given direction
#
# This proc used for drawing multiple pieces at the same move
#############################################################################
proc getListColor { board direction spacex spacey} {
	global height width
	
	set lcolorup {}
	set lcolordw {}
	set lcolorrg {}
	set lcolorlf {}
	
	# Calculate the direction the piece will move to
	for {set j 0} {$j < $height} {incr j} {
		for {set i 0} {$i < $width} {incr i} {
			set index [expr $j * $width + $i]
			set piece [string index $board $index]
			set color [getColor $piece]
			#check for up
			if {$i == $spacex} {
				if {$j < $spacey} {
					set lcolorup [concat $color $lcolorup]
				} elseif {$j > $spacey} {
					set lcolordw [concat $lcolordw $color]
				}
			} elseif {$j == $spacey} {
				if {$i < $spacex} {
					set lcolorlf [concat $color $lcolorlf]
				} elseif {$i > $spacex} {
					set lcolorrg [concat $lcolorrg $color]
				}	
			}
		}
	}
	switch $direction {
		"0" { set lcolor $lcolordw}
		"1" { set lcolor $lcolorup}
		"2" { set lcolor $lcolorrg}
		"3" { set lcolor $lcolorlf}
		default { puts "NOT MATCH"}
	}
  	return $lcolor	
}
    
#############################################################################
# Hash functions
#
#############################################################################

# Return a string represeted a board 
proc hashMove { direction numPieces } {
    global boardSize
    return [expr $direction*($boardSize-1)+$numPieces]
}

# Calculate direction based on the given move
proc getDirection { move } {
    global boardSize
    set direction [expr $move / ($boardSize-1)]
    return $direction
}

# Calculate number of pieces on the board
proc getNumPieces { move } {
    global boardSize
    return [expr $move%($boardSize-1)]
}


#############################################################################
# Helper functions
#
#############################################################################

# Set the given obj to a specified color
proc SetColour { c obj colour } {
    $c itemconfig $obj -fill $colour
}

# get color from a given piece
proc getColor {piece} {
    global xColor oColor
    if {[string compare $piece "X"] == 0} {
	set color $xColor
    } else {
	set color $oColor
    }
    
    return $color
}

# get a list of number of arrow for each direction	
proc getNumMove { moveList } {
    set up 0
    set dw 0
    set lf 0
    set rg 0
  
    foreach item $moveList {
	set direction [getDirection [lindex $item 0]]
	
	switch $direction {
		"0" { incr up}
		"1" { incr dw}
		"2" { incr rg}
		"3" { incr lf}
	}
    }
    return [concat $up $dw $rg $lf]
}
# Return a factorial value
proc factorial { n } {
    if { $n == 0 || $n == 1 } {
	return 1
    } else {
    	return [expr $n * [factorial [expr $n - 1]]]
    }
} 

# Return a minimum value 
proc min {x y} {
    if {$x > $y} {
	return $y
    } else {
	return $x
    }
}

# Return a minimum value 
proc max {x y} {
    if {$x > $y} {
	return $x
    } else {
	return $y
    }
}

#get a negative 1
proc neg {x} {
    if {$x < 0} {
	return -1
    } elseif {$x == 0} {
	return 0
    } else {
	return 1
    }
}