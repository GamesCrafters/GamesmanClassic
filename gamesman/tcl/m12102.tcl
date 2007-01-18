#	8/12/06		hacked to get automove at end to display correct color
#			need to find better way to do it aside from breaking
#			abstraction with use of gWhoseTurn - dchan
# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.
proc GS_InitGameSpecific {} {
    
    ### Set the name of the game

    global kGameName
    set kGameName "1,2,...,10"
    
    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to reach 10 WINS"
    set kMisereString "First player to reach 10 LOSES"

    ### Set the strings to tell the user how to move and what the goal is.

	global gMisereGame
	if {!$gMisereGame} {
		SetToWinString "To Win: Be the first player to reach the top slot (10)"
	} else {
		SetToWinString "To Win: Force your opponent to reach the top slot (10) first"
	}

    SetToMoveString "To Move: Click with the left button 1 or 2 slots above the current highest slot"
	    
    # Authors Info
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Gamesman Spring 2003 Whole Team!"
    set kTclAuthors "Gamesman Spring 2003 Whole Team!"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
}

# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"

proc GS_NameOfPieces {} {
    return [list left right]
}

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

proc GS_ColorOfPlayers {} {
    return [list blue red]
}

# Setup the rules frame
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


# Get the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code

proc GS_GetOption { } {
    global gMisereGame
    global kMinRows kMaxRows
    if {$gMisereGame == 0} {
	set option 1
    } else {
	set option 2
    }
    # set option 1
    # set option [expr $option + (1-$gMisereGame)]
    return $option
}


# Modify the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option.
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing

proc GS_SetOption { option } {
    global gMisereGame
    global kMaxRows kMinRows
    if {$option == 1} {
	set gMisereGame 0
    } else {
	set gMisereGame 1
    }
    # set option [expr $option - 1]
    # set gMisereGame [expr 1-($option%2)]
}

# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"

proc GS_Initialize { c } {

	global kLabelFont gMisereGame	

    # you may want to start by setting the size of the canvas; this line isn't cecessary
    #$c configure -width 500 -height 500
    #why did we ever hard code this!!!!!!!!
	global gFrameWidth gFrameHeight
	set edgeBuffer 50
	set gameHeight [expr $gFrameHeight - $edgeBuffer*2]
        set gameWidth [expr $gameHeight/10]

	for {set j 0} {$j<10} {incr j} {
	    set i [expr 10 - $j]
	    $c create rect \
		[expr [expr $gFrameWidth/2] - [expr $gameWidth/2]] \
		[expr $gameWidth * $j + $edgeBuffer] \
		[expr [expr $gFrameWidth/2] + [expr $gameWidth/2]] \
		[expr $gameWidth * [expr $j + 1] + $edgeBuffer] \
		-fill white -outline black -tag [list base base$i]
	    $c create oval \
		[expr [expr $gFrameWidth/2] - [expr $gameWidth * 0.3]] \
		[expr $j * $gameWidth + [expr $gameWidth/5] + $edgeBuffer] \
		[expr [expr $gFrameWidth/2] + [expr $gameWidth * .3]] \
		[expr $j * $gameWidth + [expr $gameWidth*4/5] + $edgeBuffer] \
		-width 2 -outline "" -tag [list pieces piece-$i]
	    $c create oval \
		[expr [expr $gFrameWidth/2] - [expr $gameWidth/10]] \
		[expr $j * $gameWidth + [expr $gameWidth*2/5] + $edgeBuffer] \
		[expr [expr $gFrameWidth/2] + [expr $gameWidth/10]] \
		[expr $j * $gameWidth + [expr $gameWidth*3/5] + $edgeBuffer] \
		-width 2 -outline "" -tag [list moveindicators mi-$i]
	    $c lower piece$i base
	    
	    set theText [$c create text [expr [expr $gFrameWidth/2] - [expr $gameWidth*0.36]] [expr ($j+1)*$gameWidth - 5  + $edgeBuffer] \
			     -text "$i" \
			     -anchor sw \
			     -font $kLabelFont \
			     -fill black \
			     -tag tagText]
	
	    $c addtag tagTextSlot$i withtag $theText
	    #puts "theText = $theText"
	
	    if { $j == 0 } {
	
			if { !$gMisereGame } {
			    set theMessage "WIN"
			} else {
			    set theMessage "LOSE"
			}
		
			set theText [$c create text [expr $gFrameWidth/2] [expr 5 + $edgeBuffer] \
				-text $theMessage \
				-anchor n \
				-font $kLabelFont \
				-fill  black \
				-tag tagText]
		
			$c addtag tagTextSlot$i withtag $theText
		}
    }

    $c raise base
    $c raise tagText

    global curPlayer gWhoseTurn
    if {$gWhoseTurn == "Left"} {
	set curPlayer blue;
    } else {
	set curPlayer red;
    }
} 

proc GS_Deinitialize { c } {
    $c delete all
}


# GS_DrawPosition this draws the board in an arbitrary position.
# It's arguments are a canvas, c, where you should draw and the
# (hashed) position.  For example, if your game is a rearranger,
# here is where you would draw pieces on the board in their correct positions.
# Imagine that this function is called when the player
# loads a saved game, and you must quickly return the board to its saved
# state.  It probably shouldn't animate, but it can if you want.

# BY THE WAY: Before you go any further, I recommend writing a tcl function that 
# UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.


proc GS_DrawPosition { c position } {
    global curPlayer    
    # BTW too: don't make any assumptions about the state of the board.

    set newcolor $curPlayer

    $c itemconfigure piece-$position -fill $newcolor

    $c raise base
    $c raise tagText

    if {$position != 10} {	
		$c raise piece-$position base	
	}

    update idletasks	
}

# GS_NewGame should start playing the game. "let's play"  :)
# It's arguments are a canvas, c, where you should draw and
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.

proc GS_NewGame { c position } {
    # The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    GS_DrawPosition $c $position
}

# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.

proc GS_WhoseMove { position } {
    return ""    
}

# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

proc GS_HandleMove { c oldPosition theMove newPosition } {
#    $c lower pieces base
    $c itemconfigure base$newPosition -fill white
    $c itemconfigure tagTextSlot$newPosition -fill black
    global curPlayer

    set newcolor $curPlayer
    if {$curPlayer == "blue"} {
	set curPlayer red
    } else {
	set curPlayer blue
    }

    $c itemconfigure piece-$newPosition -fill $newcolor
    $c raise piece-$newPosition base
}

# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever the
# player clicks to make the move)  It is also the function that handles coloring
# of the moves according to value. It is called by gamesman just before the player
# is prompted for a move.

# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which radio button is down
# position = the current hashed position
# moveList = a list of available moves to the player.  These moves are represented as numbers (same as in C)

# The code snippet herein may be helpful but is not necessary to do it that way.
# We provide a procedure called MoveTypeToColor that takes in moveType and
# returns the correct color.

proc GS_ShowMoves { c moveType position moveList } {
    
    foreach item $moveList {
        set move  [expr [lindex $item 0] + $position]
        set value [lindex $item 1]
        set color cyan
        
        $c raise mi-$move base
        
        $c bind base$move <ButtonRelease-1> "ReturnFromHumanMove [lindex $item 0]"
        $c bind mi-$move <ButtonRelease-1> "ReturnFromHumanMove [lindex $item 0]"
        $c bind tagTextSlot$move <ButtonRelease-1> "ReturnFromHumanMove [lindex $item 0]"
        $c bind base$move <Enter> "$c itemconfigure base$move -fill black; $c itemconfigure tagTextSlot$move -fill white"
        $c bind base$move <Leave> "$c itemconfigure base$move -fill white; $c itemconfigure tagTextSlot$move -fill black"

        if {$moveType == "value"} {
            if {$value == "Tie"} {
                set color yellow
            } elseif {$value == "Lose"} {
                set color green
            } else {
                set color red4
            }
        }

        $c itemconfigure mi-$move -fill $color
    }

    update idletasks
}

# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.

proc GS_HideMoves { c moveType position moveList} {
    foreach item $moveList {
        set move  [expr [lindex $item 0] + $position]
        set value [lindex $item 1]
        set color cyan        

        $c bind base$move <ButtonRelease-1> ""
        $c bind mi-$move <ButtonRelease-1> ""
        $c bind tagTextSlot$move <ButtonRelease-1> ""
        $c bind base$move <Enter> ""
        $c bind base$move <Leave> ""
    }

    $c lower moveindicators base    
    update idletasks
}

# GS_HandleUndo draws a move undoing itself sortof.
# the names of the arguments explain themsleves but just to clarify...
# The game was in position A, a player makes move M bringing the game to position B
# then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A

# By default this function just calls GS_DrawPosition, but you certainly don't need to keep that.

proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {
 	$c lower piece-$currentPosition base
    update idletasks
}

# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
proc GS_GetGameSpecificOptions { } {
}

# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# you could use this function to draw the line striking out the winning row in tic tac toe for instance
# or you could congratulate the winner or do nothing if you want.

proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove } {
}


# GS_UndoGameOver is called then the player hits undo after the game is finished.
# this is provided so that you may undo the drawing you did in GS_GameOver if you drew something.
# for instance, if you drew a line crossing out the winning row in tic tac toe, this is where you sould delete the line.

# note: GS_HandleUndo is called regardless of whether the move undoes the end of the game, so IF you choose to do nothing in
# GS_GameOver, you needn't do anything here either.

proc GS_UndoGameOver { c position } {
}
