####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################

#variable used to determine whether this turn is a colored move or a neutral
#move
global isColorMove
set isColorMove 1

set margin 50
set boardSize 3

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
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "3 Spot"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 0x001009BD
    set gPosition 0x001009BD

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to ______ WINS"
    set kMisereString "First player to ______ LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Score 12 points when your opponent has scored at least 6. If player A scores 12 points but player B has not yet reached 6, then B is the winner."
    } else {
	SetToWinString "To Win: Score at least 6 points and force your opponent to score at least 12 points ot score 12 points but force your opponent to score lower than 6 points."
    }
    SetToMoveString "To Move: During a player's turn, move the colored piece that belongs to you. The piece must stay on the board and must cover at least one new square. Afterwards, the same player must move the neutral piece to a new position as well."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Attila Gyulassy"
    set kTclAuthors "Simon Tao"
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

    return [list R B W]

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

    return [list red blue white]
    
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

    DrawBoard $c

} 

proc min { a b } {
    if { $a < $b } {
	return $a
    }
    return $b
}

proc DrawBoard { c } {
    global gFrameWidth gFrameHeight boardSize size margin square
    set size [min $gFrameWidth $gFrameHeight]
    set square [expr ($size - 2*$margin) / $boardSize]
    set radius [expr 0.2 * $square]
    
    # Draw a gray background
    $c create rect $margin $margin [expr $size-$margin] [expr $size-$margin] -fill gray

    # Draw a 3x3 grid
    for { set i 0 } { $i < $boardSize-1 } { incr i } {
	$c create line $margin [expr $margin + ($i + 1) * $square] [expr $size - $margin] [expr $margin + ($i + 1) * $square] -width 2 -fill "\#696969"
	$c create line [expr $margin + ($i + 1) * $square] $margin [expr $margin + ($i + 1) * $square] [expr $size - $margin] -width 2 -fill "\#696969"
    }

    drawCircle $c

}

proc drawCircle { c } {
    global gFrameWidth gFrameHeight boardSize size margin square
    set size [min $gFrameWidth $gFrameHeight]
    set square [expr ($size - 2*$margin) / $boardSize]
    set radius [expr 0.2 * $square]

    # Draw three spots
    for { set i 0 } { $i < $boardSize } { incr i } {
	$c create oval \
	    [expr $margin + $square * ($i + 0.5) - $radius] \
	    [expr $margin + 2.5 * $square - $radius] \
	    [expr $margin + $square * ($i + 0.5) + $radius] \
	    [expr $margin + 2.5 * $square + $radius] \
	    -fill darkgray
    }
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

    #piecePos
    #123
    #456
    #789

    set redPos [getPos 1 $position]
    set red0 [pieceToBoard $redPos 0]
    set red1 [pieceToBoard $redPos 1]
    set bluePos [getPos 0 $position]
    set blue0 [pieceToBoard $bluePos 0]
    set blue1 [pieceToBoard $bluePos 1]
    set whitePos [getPos 2 $position]
    set white0 [pieceToBoard $whitePos 0]
    set white1 [pieceToBoard $whitePos 1]

#puts "red [expr $red0] [expr $red1] blue [expr $blue0] [expr $blue1] white [expr $white0] [expr $white1]"

    drawSquare $c $red0 "red"
    drawSquare $c $red1 "red"
    drawSquare $c $blue0 "blue"
    drawSquare $c $blue1 "blue"
    drawSquare $c $white0 "white"
    drawSquare $c $white1 "white"
}

proc getPos { color position } {
    #red
    if { $color == 1 } {
        return [expr ($position & 0x00000f00) >> 8]
    #blue
    } elseif { $color == 0 } {
        return [expr ($position & 0x0000000f) >> 0]
    #white
    } else {
        return [expr ($position & 0x000000f0) >> 4]
    }
}

proc pieceToBoard { position piecePart } {
#puts $position
    set posToBoard [list 7 1 4 2 1 3 8 4 5 5 2 6]
    set move [lindex $posToBoard [expr $position-2]]

    if { $piecePart == 0 } {
	return $move
    } else {
	if { [expr $position & 0x00000001] == 0 } {
	    incr move
	} else {
	    set move [expr $move + 3]
	}
	return $move
    }

}

proc drawSquare { c num color } {
    global gFrameWidth gFrameHeight boardSize size margin square
    set size [min $gFrameWidth $gFrameHeight]
    set square [expr ($size - 2*$margin) / $boardSize]

    set row [expr ($num-1)/3]
    set col [expr ($num-1)%3]

    #draw the square
    $c create rect \
        [expr $margin + $square * $col] \
        [expr $margin + $square * $row] \
        [expr $margin + $square * ($col + 1)] \
        [expr $margin + $square * ($row + 1)] \
        -fill $color -width 2

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
    # player 1 = 1 or red, player 2 = 0 or blue
    return [expr $position >> 20]
   
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
    
    global isColorMove

    animateMove $c $theMove
    GS_DrawPosition $c $newPosition

    if { $isColorMove == 1 } {
        set isColorMove 0
    } else {
        set isColorMove 1
    }

}

proc animateMove { c theMove } {
    global gFrameWidth gFrameHeight boardSize size margin square
    set size [min $gFrameWidth $gFrameHeight]
    set square [expr ($size - 2*$margin) / $boardSize]

    

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
    $c delete moves
puts $moveList
    global isColorMove
    set trimmedMoveList [trimMoveList $position $moveList $isColorMove]
    

    foreach move $trimmedMoveList {
        set colorPos [getPos 0 [lindex $move 0]]
        set color0 [pieceToBoard $colorPos 0]
        set color1 [pieceToBoard $colorPos 1]
        set whitePos [getPos 2 [lindex $move 0]]
        set white0 [pieceToBoard $whitePos 0]
        set white1 [pieceToBoard $whitePos 1]

        #puts "color [expr $color0] [expr $color1] white [expr $white0] [expr $white1]"

	drawMove $c [lindex $move 0] $moveType [lindex $move 1]
    }

}

proc trimMoveList { position moveList isColorMove} {
    #piecePos
    #123
    #456
    #789

    #white's move
    if { $isColorMove == 0 } {
        set trimmedList [trim $position $moveList 2 $isColorMove]
    #colored's move
    } else {
        set turn [GS_WhoseMove $position]
        #red's move
        if { $turn == 1 } {
            set trimmedList [trim $position $moveList 1 $isColorMove]
        #blue's move
        } else {
            set trimmedList [trim $position $moveList 0 $isColorMove]
        }
    }

    return $trimmedList

}

proc trim { position moveList color isColorMove} {
    set redPos [getPos 1 $position]
#puts $redPos
    set red0 [pieceToBoard $redPos 0]
    set red1 [pieceToBoard $redPos 1]
    set bluePos [getPos 0 $position]
#puts $bluePos
    set blue0 [pieceToBoard $bluePos 0]
    set blue1 [pieceToBoard $bluePos 1]
    set whitePos [getPos 2 $position]
#puts $whitePos
    set white0 [pieceToBoard $whitePos 0]
    set white1 [pieceToBoard $whitePos 1]

    if { $color == 1 } {
        set ownPos0 $red0
        set ownPos1 $red1
        set otherPos0 $blue0
        set otherPos1 $blue1
        set otherPos2 $white0
        set otherPos3 $white1
    } elseif { $color == 0 } {
        set ownPos0 $blue0
        set ownPos1 $blue1
        set otherPos0 $red0
        set otherPos1 $red1
        set otherPos2 $white0
        set otherPos3 $white1
    } else {
        set ownPos0 $white0
        set ownPos1 $white1
        set otherPos0 $red0
        set otherPos1 $red1
        set otherPos2 $blue0
        set otherPos3 $blue1
    }


    set result {}

    foreach move $moveList {
        if { $isColorMove == 1 } {
            set piecePos [getPos 0 [lindex $move 0]]
        } else {
            set piecePos [getPos 2 [lindex $move 0]]
        }
        set movePos0 [pieceToBoard $piecePos 0]
        set movePos1 [pieceToBoard $piecePos 1]
        #puts "movePos0 [expr $movePos0] != ownPos0 [expr $ownPos0] && movePos1 [expr $movePos1] != ownPos1 [expr $ownPos1]" 

        if { $movePos0 != $otherPos0 &&
             $movePos0 != $otherPos1 &&
             $movePos0 != $otherPos2 &&
             $movePos0 != $otherPos3 &&
             $movePos1 != $otherPos0 &&
             $movePos1 != $otherPos1 &&
             $movePos1 != $otherPos2 &&
             $movePos1 != $otherPos3 } {
            if { !($movePos0 == $ownPos0 && $movePos1 == $ownPos1) &&
                 !($movePos1 == $ownPos0 && $movePos0 == $ownPos1) } {
                lappend result $move
                #puts "inside"
            }
        }
        #puts $result
    }

    #puts ""

    return $result

}

proc drawMove { c move moveType type } {
    
    switch $moveType {
	value {

	    # If the move leads to a win, it is a losing move (RED)
	    # If the move leads to a losing position, it is a winning move (GREEN)
	    # If the move leads to a tieing position, it is a tieing move (YELLOW)
	    switch $type {
		Win { set color darkred }
		Lose { set color green }
		Tie { set color yellow }
		default { set color cyan }
	    }
	}
	default {
	    set color cyan
	}
    }

    
    global isColorMove

    set colorPos [getPos 0 $move]
    set color0 [pieceToBoard $colorPos 0]
    set color1 [pieceToBoard $colorPos 1]
    set whitePos [getPos 2 $move]
    set white0 [pieceToBoard $whitePos 0]
    set white1 [pieceToBoard $whitePos 1]


    global gFrameWidth gFrameHeight boardSize size margin square
    set size [min $gFrameWidth $gFrameHeight]
    set square [expr ($size - 2*$margin) / $boardSize]
    set longradius [expr 0.2 * $square]
    set shortradius [expr 0.1 * $square]

    if { $isColorMove == 1 } {
        set space0 $color0
        set space1 $color1
        set row [expr ($color0-1)/3]
        set col [expr ($color0-1)%3]
    } else {
        set space0 $white0
        set space1 $white1
        set row [expr ($white0-1)/3]
        set col [expr ($white0-1)%3]
    }
    
    #left-right oval
    if { [expr $space1-$space0] == 1} {
        set temp [$c create oval \
                      [expr $margin + $square * ($col+1) - $longradius] \
                      [expr $margin + $square * ($row+0.5) - $shortradius] \
                      [expr $margin + $square * ($col+1) + $longradius] \
                      [expr $margin + $square * ($row+0.5) + $shortradius] \
                      -fill $color]

    #top-bottom oval
    } else {
        set temp [$c create oval \
                      [expr $margin + $square * ($col+0.5) - $shortradius] \
                      [expr $margin + $square * ($row+1) - $longradius] \
                      [expr $margin + $square * ($col+0.5) + $shortradius] \
                      [expr $margin + $square * ($row+1) + $longradius] \
                      -fill $color]
    }


    $c bind $temp <Enter> "$c itemconfigure $temp -fill black"
    $c bind $temp <Leave> "$c itemconfigure $temp -fill $color"
    $c bind $temp <ButtonRelease-1> "ReturnFromHumanMove $move"

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

	### TODO if needed
	
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
