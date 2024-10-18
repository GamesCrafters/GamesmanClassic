####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################

#############################################################################
##
## NAME:         mlite3.tcl
##
## DESCRIPTION:  The source code for the Tcl component of Mlite3
##
## AUTHOR:       Dan Garcia
##               Alex Perelman
##
## DATE:         04-03-2002
##
## GOLD UPDATE:  Jeffrey Chiang (2004-09-12)
#############################################################################
#############################################################################
# Some Notes:
#   o This tcl code is hardcoded for the 3x3 game.  To make it expandable
#     (If you choose to do so) requres you to change how the order of moves
#     is stored
#   o In fact, there is some not so nice abstraction going on in this code.
#     Essentially, the moves are 1-9 whereas the slots (squares) that 
#     correspond to them are designed as if the moves were 0-8.  This code 
#     accounts for this, but in a not so nice way.
#   o GS_WhoseMove is not implemented.  It may be nice in the future to
#     change all the references of gplayeroneTurn to use GS_WhoseMove instead
#
#############################################################################

proc min { x y } {
    if { $x < $y } {
	return $x
    }
    return $y
}

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
    set kGameName "Lite-3"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set the size of the canvas (can be changed)

    global CANVAS_WIDTH CANVAS_HEIGHT gFrameWidth gFrameHeight

    set CANVAS_WIDTH [min $gFrameWidth $gFrameHeight]
    set CANVAS_HEIGHT $CANVAS_WIDTH

    ### Set the size of the board
    # 3x3 board, element 0 is whoseTurn 
    
    global gSlotsX gSlotsY BOARDSIZE
    set gSlotsX 3
    set gSlotsY 3
    set BOARDSIZE 10 

    ### Calculate and set size of a slot
    ### Allegedly it should be multiple of 80, but 100 is ok
    ### Hmmm, I think X is horizontal and Y is vertical...
    global gSlotSize

    set widthLimit [expr int($CANVAS_WIDTH/$gSlotsX)]
    set heightLimit [expr int($CANVAS_HEIGHT/$gSlotsY)]

    if {$heightLimit < $widthLimit} {
        set gSlotSize $heightLimit
    } else {
        set gSlotSize $widthLimit
    }

    # possibleBoardPieces
    global Blank o1 o2 o3 x1 x2 x3
    set Blank 0
    set o1 1
    set o2 2
    set o3 3
    set x1 4
    set x2 5
    set x3 6


    ### Color of the numbers (to show order)
    global numberColor
    set numberColor grey


    ### These are to keep track of most recently played pieces
    global varXone varXtwo varXthree varOone varOtwo varOthree
    set varXone -1
    set varXtwo -1
    set varXthree -1
    set varOone -1
    set varOtwo -1
    set varOthree -1

    ### Set the color and font of the labels

    global kLabelFont
    global kLabelColor

    set kLabelFont { Helvetica 12 bold }
    set kLabelColor grey40

    ### These are used in the routine that draws the pieces - we want a small
    ### piece used as a label but a larger piece used in the playing board.
    global kBigPiece
    set kBigPiece 1

    global kSmallPiece
    set kSmallPiece 0

    ### Set the procedures that will draw the pieces

    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawCross
    set kRightDrawProc DrawHollowCircle

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to get 3 in a row WINS"
    set kMisereString "First player to get 3 in a row LOSES"
    
    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section
    global gObjNum
    global gMisereGame
    if {!$gMisereGame} {
	if {$gObjNum == 0} {
	    SetToWinString "To Win: Get 3 in a row"
	} elseif {$gObjNum == 1} {
	    SetToWinString "To Win: Trap an opponent's piece with three of your pieces"       
	} else {
	    SetToWinString "To Win: Either get 3 in a row or trap an opponent's piece with three of your pieces"
	}
    } else {
	if {$gObjNum == 0} {
	    SetToWinString "To Win: Force your opponent to get 3 in a row"
	} elseif {$gObjNum == 1} {
	    SetToWinString "To Win: Force your opponent to surround one of your pieces"
	} else {
	    SetToWinString "To Win: Either force your opponent to get 3 in a row or force your opponent to surround one of your pieces"
	}
    }
    SetToMoveString "To Move: Place your piece on an empty space.  If you already have three pieces on the board, the earliest piece you  placed will be removed. This creates an open neutral spot. Select a square to place your piece. The number on the bottom left hand corner corresponds to when you placed the piece. "

	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Alex Perelman, Babak Hamadani"
    set kTclAuthors "Dan Garcia, Alex Perelman, Jeffrey Chiang"
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

    return [list x o]

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

    return [list blue red]
    
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

    set objectiveRule \
	[list \
	     "What would you like the game objective to be:" \
	     "3 in a row" \
	     "Trap your opponent" \
	     "Both" \
	     ]

    # List of all rules, in some order
    set ruleset [list $standardRule $objectiveRule]

    # Declare and initialize rule globals
    global gMisereGame 
    
    # 0 - Three In A Row
    # 1 - Surround
    # 2 - Both
    global gObjNum

    set gMisereGame 0
    set gObjNum 0

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gObjNum"]

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
    global gMisereGame gObjNum

    if {$gObjNum == 0} {
	set objRet 1
    } elseif {$gObjNum == 1} {
	set objRet 2
    } else {
	set objRet 3
    }
    if { $gMisereGame} {
	set gameRet 1 
    } else {
	set gameRet 0
    }
    
    set ret [expr $objRet + [expr 3 * $gameRet]]
    
    return $ret
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
    global gMisereGame gObjNum
    
    set standardGame [expr $option < 4]
    if {$standardGame} {
	set $gMisereGame 0 
    } else {
	set $gMisereGame 1
    }

    set option [expr $option % 3]

    if {$option == 0} {
	set gObjNum 2
    } elseif {$option == 1} {
	set gObjNum 0 
    } else {
	set gObjNum 1
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
    global CANVAS_WIDTH CANVAS_HEIGHT
    global gSlotsX gSlotsY

    # Creates the slots (on the board)
    for {set i 0} {$i < $gSlotsX } {incr i} { 
        for {set j 0} {$j < $gSlotsY } {incr j} {
            CreateSlot $c $i $j
        }
    }
} 


#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why this
# is here, so whoever put this here should update this.  -Jeff
#############################################################################
proc GS_Deinitialize { c } {
    #$c delete all
}


#############################################################################
##
## CreateSlot
##
## Here we create a new slot on the board. The value of the slot in
## (slotX,slotY) tells us where on the board to be.
##
#############################################################################

proc CreateSlot {w slotX slotY} {
    global gSlotSize gSlotList

    set slot [$w create rect \
		  [expr $gSlotSize*$slotX] \
		  [expr $gSlotSize*$slotY] \
		  [expr $gSlotSize*[expr $slotX+1]] \
		  [expr $gSlotSize*[expr $slotY+1]] \
        -outline black \
        -width 1 \
        -fill grey \
		  -tag tagSlot]

    ### Remember what the id of the slot was in our array

    set gSlotList($slotX,$slotY) $slot

    ### Add any slot embellishments are necessary (numbers, highlights, etc.)

    GS_EmbellishSlot $w $slotX $slotY $slot
}


#############################################################################
##
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishSlot { w slotX slotY slot } {

    $w addtag tagInitial withtag $slot
}


#############################################################################
##
## EnableSlot
##
## Here we need to Enable the slot because it's now able to be clicked upon
##
#############################################################################

proc EnableSlot { w theSlot } {
    global gSlotSize 

    $w dtag $theSlot tagDead
    $w addtag tagAlive withtag $theSlot
    
    $w bind tagAlive <1> { SendMove [GS_ConvertInteractionToMove [SinglePieceRemovalCoupleMove [expr %x/$gSlotSize] [expr %y/$gSlotSize]]] }

    $w bind tagAlive <Enter> "SlotEnter $w"
    $w bind tagAlive <Leave> "$w itemconf current -fill grey "
    $w bind tagDead  <1>     { }
    $w bind tagDead  <Enter> { }
    $w bind tagDead  <Leave> { }
}


proc SendMove { theMove } {
    ReturnFromHumanMove $theMove
}

#############################################################################
##
## SlotEnter
##
## This is what we do when we enter a live slot. (We fill it with black)
##
#############################################################################

proc SlotEnter { w } {
    $w itemconfig current -fill black
}


#############################################################################
##
## SinglePieceRemovalCoupleMove
##
## With SinglePieceRemoval, sometimes the move is in (x,y) and we have to
## convert it to a single move again.
##
#############################################################################

proc SinglePieceRemovalCoupleMove { slotX slotY } {
    global gSlotsX

    ### Row major format. The number of rows times the pieces per row + 
    ### the # of columns
    return [expr ($gSlotsX * $slotY) + $slotX]
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
proc GS_DrawPosition { w position } {
    global Blank o1 o2 o3 x1 x2 x3
    global BOARDSIZE
    global gSlotSize kLabelFont numberColor gSlotsX gSlotList
    global varXone varXtwo varXthree varOone varOtwo varOthree

    set varXone -1
    set varXtwo -1
    set varXthree -1
    set varOone -1
    set varOtwo -1
    set varOthree -1

    $w delete tagPiece
    $w delete tagText
    $w dtag tagDead
    $w dtag tagAlive
    $w addtag tagAlive withtag tagInitial

    set unhashedPosition [unhash $position]
    for {set i 1} { $i < $BOARDSIZE } {incr i} {
    	set whatInSlot [lindex $unhashedPosition $i]

	set internalSlot [expr $i - 1]
	set slotX [expr $internalSlot % $gSlotsX]
	set slotY [expr $internalSlot / $gSlotsX]
	set theSlot $gSlotList($slotX,$slotY)

	if { $whatInSlot == $o1 } { 
	    set varOone $internalSlot
	    DisableSlot $w $theSlot
	    
	    set textNum "1"

	    ### We add a numeric label to each slot
	    set theText [$w create text [expr ($slotX * $gSlotSize) + 4] [expr ($slotY + 1)*$gSlotSize - 4] \
			     -text $textNum \
			     -anchor sw \
			     -font $kLabelFont \
			     -fill  $numberColor \
			     -tag tagText]

	    $w addtag tagTextNumX$textNum withtag $theText
	    set thePiece "O" 

	    DrawPiece $slotX $slotY $thePiece $w


	} elseif { $whatInSlot == $o2 } { 
	    set varOtwo $internalSlot
	    DisableSlot $w $theSlot
	    
	    set textNum "2"

	    ### We add a numeric label to each slot
	    set theText [$w create text [expr ($slotX * $gSlotSize) + 4] [expr ($slotY + 1)*$gSlotSize - 4] \
			     -text $textNum \
			     -anchor sw \
			     -font $kLabelFont \
			     -fill  $numberColor \
			     -tag tagText]

	    $w addtag tagTextNumX$textNum withtag $theText
	    set thePiece "O" 

	    DrawPiece $slotX $slotY $thePiece $w
	    
	} elseif { $whatInSlot == $o3 } { 
	    set varOthree $internalSlot
	    DisableSlot $w $theSlot
	    
	    set textNum "3"

	    ### We add a numeric label to each slot
	    set theText [$w create text [expr ($slotX * $gSlotSize) + 4] [expr ($slotY + 1)*$gSlotSize - 4] \
			     -text $textNum \
			     -anchor sw \
			     -font $kLabelFont \
			     -fill  $numberColor \
			     -tag tagText]

	    $w addtag tagTextNumX$textNum withtag $theText
	    set thePiece "O" 

	    DrawPiece $slotX $slotY $thePiece $w

	} elseif { $whatInSlot == $x1 } { 
	    set varXone $internalSlot
	    DisableSlot $w $theSlot
	    
	    set textNum "1"

	    ### We add a numeric label to each slot
	    set theText [$w create text [expr ($slotX * $gSlotSize) + 4] [expr ($slotY + 1)*$gSlotSize - 4] \
			     -text $textNum \
			     -anchor sw \
			     -font $kLabelFont \
			     -fill  $numberColor \
			     -tag tagText]

	    $w addtag tagTextNumX$textNum withtag $theText
	    set thePiece "X" 

	    DrawPiece $slotX $slotY $thePiece $w

	} elseif { $whatInSlot == $x2 } { 
	    set varXtwo $internalSlot
	    DisableSlot $w $theSlot
	    
	    set textNum "2"

	    ### We add a numeric label to each slot
	    set theText [$w create text [expr ($slotX * $gSlotSize) + 4] [expr ($slotY + 1)*$gSlotSize - 4] \
			     -text $textNum \
			     -anchor sw \
			     -font $kLabelFont \
			     -fill  $numberColor \
			     -tag tagText]

	    $w addtag tagTextNumX$textNum withtag $theText
	    set thePiece "X" 

	    DrawPiece $slotX $slotY $thePiece $w

	} elseif { $whatInSlot == $x3 } { 
	    set varXthree $internalSlot
	    DisableSlot $w $theSlot
	    
	    set textNum "3"

	    ### We add a numeric label to each slot
	    set theText [$w create text [expr ($slotX * $gSlotSize) + 4] [expr ($slotY + 1)*$gSlotSize - 4] \
			     -text $textNum \
			     -anchor sw \
			     -font $kLabelFont \
			     -fill  $numberColor \
			     -tag tagText]

	    $w addtag tagTextNumX$textNum withtag $theText
	    set thePiece "X" 

	    DrawPiece $slotX $slotY $thePiece $w

	} elseif { $whatInSlot == 0 } { 
	    EnableSlot $w $theSlot


	} else {
	    BadElse DrawPosition
	}
	

    }
}

#############################################################################
# Unhash takes a given position and returns a board representation for 
# it.
#############################################################################
proc unhash { thePos } {
    global BOARDSIZE
    global Blank o1 o2 o3 x1 x2 x3

    set pieces [expr $thePos >> 1]

    set whoseTurn [expr $thePos & 1]

    set piece_o3 [expr $pieces % 10]
    set piece_o2 [expr [expr $pieces % 100] / 10]
    set piece_o1 [expr [expr $pieces % 1000] / 100]
    set piece_x3 [expr [expr $pieces % 10000] / 1000]
    set piece_x2 [expr [expr $pieces % 100000] / 10000]
    set piece_x1 [expr [expr $pieces % 1000000] / 100000]

    for {set i 0} { $i < $BOARDSIZE } {incr i} {
    	set theBlankOOOXXX($i) 0
    }

    set theBlankOOOXXX($piece_o3) $o3
    set theBlankOOOXXX($piece_o2) $o2
    set theBlankOOOXXX($piece_o1) $o1
  
    set theBlankOOOXXX($piece_x3) $x3
    set theBlankOOOXXX($piece_x2) $x2
    set theBlankOOOXXX($piece_x1) $x1
  
    set theBlankOOOXXX(0) $whoseTurn

    set board {}
    
    for {set i 0} {$i < $BOARDSIZE} {incr i} {
        lappend board $theBlankOOOXXX($i)
    }    

    return $board
}


#############################################################################
# GS_NewGame should start playing the game. 
# It's arguments are a canvas, c, where you should draw 
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.
#############################################################################
proc GS_NewGame { c position } {
    global gSlotsX gSlotsY
    global varXone varXtwo varXthree varOone varOtwo varOthree

    ### Delete all pieces that are left around

    $c delete tagPiece
    $c delete gameover

    set varXone -1
    set varXtwo -1
    set varXthree -1
    set varOone -1
    set varOtwo -1
    set varOthree -1

    $c delete tagText

    GS_DrawPosition $c $position

    global gPlayerOneTurn
    set gPlayerOneTurn 1

    $c dtag tagDead
    $c dtag tagAlive
    $c addtag tagAlive withtag tagInitial
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
    
    global gPlayerOneTurn gPosition gSlotSize gSlotList
    global gSlotsX gSlotsY

    set w $c

    set internalMove [expr $theMove - 1]
    set slotX [expr $internalMove % $gSlotsX]
    set slotY [expr $internalMove / $gSlotsX]
    set theSlot $gSlotList($slotX,$slotY)

    ### Enable or Disable slots as a result of the move
    GS_HandleEnablesDisables $w $theSlot $theMove

    if { $gPlayerOneTurn } { set thePiece "X" } { set thePiece "O" }

    DrawPiece $slotX $slotY $thePiece $c

    if { $gPlayerOneTurn == 1} { set gPlayerOneTurn 0 } { set gPlayerOneTurn 1}
}


#############################################################################
##
## GS_HandleEnablesDisables
##
## At this point a move has been registered and we have to handle the
## enabling and disabling of slots
##
#############################################################################
proc GS_HandleEnablesDisables { w theSlot theMove } {

    global varXone varXtwo varXthree varOone varOtwo varOthree 
    global gPlayerOneTurn gSlotsX gSlotSize kLabelFont gSlotList numberColor
    
    DisableSlot $w $theSlot
    set theBoardMove [expr $theMove - 1]

    set moveSlotX   [expr $theBoardMove % $gSlotsX]
    set moveSlotY   [expr $theBoardMove / $gSlotsX]
    set textNum "1"

    if { $gPlayerOneTurn == 0 } {

        # must have been o's turn

        if { $varOone == -1 } {
            set varOone $theBoardMove
            set textNum "1"
        } elseif { $varOtwo == -1 } {
            set varOtwo $theBoardMove
            set textNum "2"
        } elseif { $varOthree == -1 } {
            set varOthree $theBoardMove
            set textNum "3"

	} else {
            set slotX   [expr $varOone % $gSlotsX]
            set slotY   [expr $varOone / $gSlotsX]

            $w delete tagPieceOnCoord$slotX$slotY
            EnableSlot $w $gSlotList($slotX,$slotY)

            set varOone $varOtwo
            set varOtwo $varOthree
            set varOthree $theBoardMove

            set textNum "3"

            $w delete tagTextNumO1

            $w itemconfig tagTextNumO3 -text "2"
            $w addtag num3 withtag tagTextNumO3

            $w itemconfig tagTextNumO2 -text "1"
            $w addtag num2 withtag tagTextNumO2

            $w dtag tagTextNumO3
            $w dtag tagTextNumO2
            $w dtag tagTextNumO1

            $w addtag tagTextNumO2 withtag num3
            $w addtag tagTextNumO1 withtag num2

            $w dtag num3
            $w dtag num2
        }
        set theText [$w create text [expr ($moveSlotX * $gSlotSize) + 4] [expr ($moveSlotY + 1)*$gSlotSize - 4] \
                -text $textNum \
                -anchor sw \
                -font $kLabelFont \
                -fill  $numberColor \
			 -tag tagText]

        $w addtag tagTextNumO$textNum withtag $theText

        #$w itemconfig tagTextNumO1 -fill white
        #$w itemconfig tagTextNumO2 -fill white
        #$w itemconfig tagTextNumO3 -fill white

    } else {
        #must have been x's turn

        if { $varXone < 0 } {
            set varXone $theBoardMove
            set textNum "1"
        } elseif { $varXtwo < 0 } {
            set varXtwo $theBoardMove
            set textNum "2"
        } elseif { $varXthree < 0 } {
            set varXthree $theBoardMove
            set textNum "3"
	} else {
            set slotX   [expr $varXone % $gSlotsX]
            set slotY   [expr $varXone / $gSlotsX]

            $w delete tagPieceOnCoord$slotX$slotY
            EnableSlot $w $gSlotList($slotX,$slotY)

            set varXone $varXtwo
            set varXtwo $varXthree
            set varXthree $theBoardMove

            set textNum "3"

            $w delete tagTextNumX1

            $w itemconfig tagTextNumX3 -text "2"
            $w addtag num3 withtag tagTextNumX3

            $w itemconfig tagTextNumX2 -text "1"
            $w addtag num2 withtag tagTextNumX2

            $w dtag tagTextNumX3
            $w dtag tagTextNumX2
            $w dtag tagTextNumX1

            $w addtag tagTextNumX2 withtag num3
            $w addtag tagTextNumX1 withtag num2

            $w dtag num3
            $w dtag num2
        }
	
	### We add a numeric label to each slot
        set theText [$w create text [expr ($moveSlotX * $gSlotSize) + 4] [expr ($moveSlotY + 1)*$gSlotSize - 4] \
                -text $textNum \
                -anchor sw \
                -font $kLabelFont \
                -fill  $numberColor \
			 -tag tagText]

        $w addtag tagTextNumX$textNum withtag $theText
    }
}


#############################################################################
##
## GS_ConvertMoveToInteraction
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertMoveToInteraction { theMove } {

    ### In Lite-3, the move is just the slot itself.

    return [expr $theMove - 1]
}


#############################################################################
##
## GS_ConvertInteractionToMove
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################
proc GS_ConvertInteractionToMove { theMove } {

    ### In Lite-3, the move is just the slot itself.
    return [expr $theMove + 1]
}


#############################################################################
##
## DrawPiece
##
## Draw a piece at the slot specified by slotX and slotY
##
#############################################################################

proc DrawPiece { slotX slotY thePiece c} {
    global kBigPiece kLeftDrawProc kRightDrawProc 

    if     { $thePiece == "X" } {
        return [$kLeftDrawProc  $c $slotX $slotY tagPiece blue $kBigPiece]
    } elseif { $thePiece == "O" } {
        return [$kRightDrawProc $c $slotX $slotY tagPiece red $kBigPiece]
    } else {
        BadElse DrawPiece
    }
}


#############################################################################
##
## DrawCross
##
## Here we draw an X on (slotX,slotY) in window w with a tag of theTag
## If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################

proc DrawCross { w slotX slotY theTag theColor drawBig } {
    global gSlotSize

    set theSlotSize [expr $drawBig ? $gSlotSize : ($gSlotSize / 4.0)]

    set startCross [expr $theSlotSize/8.0]
    set halfCross  [expr $theSlotSize/2.0]
    set endCross   [expr $startCross*7.0]

    set cornerX    [expr $slotX*$theSlotSize]
    set cornerY    [expr $slotY*$theSlotSize]
    set thickness  [expr $theSlotSize/10.0]

    set x1 $startCross
    set x2 [expr $startCross + $thickness]
    set x3 [expr $halfCross - $thickness]
    set x4 $halfCross
    set x5 [expr $halfCross + $thickness]
    set x6 [expr $endCross - $thickness]
    set x7 $endCross

    set y1 $startCross
    set y2 [expr $startCross + $thickness]
    set y3 [expr $halfCross - $thickness]
    set y4 $halfCross
    set y5 [expr $halfCross + $thickness]
    set y6 [expr $endCross - $thickness]
    set y7 $endCross

    set theCross [$w create polygon \
                   $x2 $y1 $x4 $y3 $x6 $y1 $x7 $y2 $x5 $y4 \
                   $x7 $y6 $x6 $y7 \
                   $x4 $y5 $x2 $y7 $x1 $y6 $x3 $y4 $x1 $y2 \
                   -fill $theColor \
		      -tag $theTag]

    if { $drawBig } {
        $w move $theCross $cornerX $cornerY
    }

    $w addtag tagPieceOnCoord$slotX$slotY withtag $theCross

    return $theCross
}


#############################################################################
##
## DrawHollowCircle
##
## Here we draw a hollow circle on (slotX,slotY) in window w with a tag of
## theTag. If drawBig is false we don't move it to slotX,slotY.
##
#############################################################################

proc DrawHollowCircle { w slotX slotY theTag theColor drawBig } {
    global gSlotSize

    set theSlotSize [expr $drawBig ? $gSlotSize : ($gSlotSize / 4.0)]

    set circleWidth [expr $theSlotSize/10.0]
    set startCircle [expr $theSlotSize/8.0]
    set endCircle   [expr $startCircle*7.0]
    set cornerX     [expr $slotX*$theSlotSize]
    set cornerY     [expr $slotY*$theSlotSize]
    set theCircle [$w create oval $startCircle $startCircle \
                       $endCircle $endCircle \
                       -outline $theColor \
                       -width $circleWidth \
                       -tag $theTag]

    if { $drawBig } {
        $w move $theCircle $cornerX $cornerY
    }

    $w addtag tagPieceOnCoord$slotX$slotY withtag $theCircle

    return $theCircle
}


#############################################################################
##
## DisableSlot
##
## Disable the selected slot in the selected window by making it grey,
## adding a 'tagDead' tag and removing the 'tagAlive' tag
##
#############################################################################

proc DisableSlot { w theSlot } {

    ### Make it grey
    $w itemconfig $theSlot -fill grey

    ### Add a 'Dead' tag

    $w addtag tagDead withtag $theSlot

    ### Remove its 'Alive' tag

    $w dtag $theSlot tagAlive

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
# moveList = a list of available moves to the player.  These moves are represented as numbers (same as in C)
#
# The code snippet herein may be helpful but is not necessary to do it that way.
# We provide a procedure called MoveTypeToColor that takes in moveType and
# returns the correct color.
#############################################################################
proc GS_ShowMoves { c moveType position moveList } {
    global kBigPiece

    if {$moveType == "value"} {

	foreach item $moveList {
	    set theMove [lindex $item 0]
	    set value [lindex $item 1]
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} { 
		set color green
	    } else {
		set color red4
	    }

	    DrawMoveSinglePieceRemoval $c $theMove $color $kBigPiece
	}

    } 
    $c itemconfig tagText -fill white
    
    update idletasks
}


#############################################################################
##
## DrawMoveSinglePieceRemoval
##
## Here we show a user's move, graphically. The color might be generic or
## it might represent a value. If drawBig is true, we draw it the full size
## of the slot. Otherwise we draw it 1/5 of the size of the slot.
##
#############################################################################

proc DrawMoveSinglePieceRemoval { w theMoveArg color drawBig } {
    global gSlotSize gSlotList gSlotsX gSlotsY

    set theSlotSize  [expr $drawBig ? $gSlotSize : ($gSlotSize * .2)]

    set circleSize   [expr $gSlotSize*.2]
    set circleOffset [expr $gSlotSize*.4]

    set theMove [$w create oval 0 0 $circleSize $circleSize \
	    -outline $color \
	    -fill $color]
    
    if { $drawBig } {
	
	#Converting from move to slot-numbering
	set theMoveArg [GS_ConvertToAbsoluteMove $theMoveArg]
	
	### Figure out how far out to put the slot into the window.
	
	set slotX   [expr $theMoveArg % $gSlotsX]
	set slotY   [expr $theMoveArg / $gSlotsX]
	set cornerX [expr $slotX   * $gSlotSize + $circleOffset]
	set cornerY [expr $slotY   * $gSlotSize + $circleOffset]

	$w move $theMove $cornerX $cornerY
	$w addtag tagMoves withtag $theMove
	$w addtag tagMoveAlive withtag $theMove
	$w bind tagMoveAlive <1> { SendMove [GS_ConvertInteractionToMove [SinglePieceRemovalCoupleMove [expr %x/$gSlotSize] [expr %y/$gSlotSize]]] }
	#$w bind tagMoveAlive <Enter> { $w itemconfig $gSlotList([expr %x/$gSlotSize],[expr %y/$gSlotSize]) -fill black }
    }
}


#############################################################################
##
## GS_ConvertToAbsoluteMove
##
## Sometimes the move handed back by our C code is a relative move. We need
## to convert this to an absolute move to indicate on the board.
##
#############################################################################

proc GS_ConvertToAbsoluteMove { theMove } {
    global gPosition

    ### Fortunately, Lite-3's moves are already absolute.

    return [expr $theMove - 1]
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {
    
    ### TODO: Fill this in
    $c itemconfig tagText -fill grey
    $c delete tagMoves
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
    global gPlayerOneTurn
    
    ### TODO if needed
    GS_DrawPosition $c $positionAfterUndo
 
    if { $gPlayerOneTurn == 1} { set gPlayerOneTurn 0 } { set gPlayerOneTurn 1}
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
    global BOARDSIZE CANVAS_WIDTH
    global gSlotsY gSlotsX gSlotList
    
    for {set i 1} { $i < $BOARDSIZE } {incr i} {
	set internalSlot [expr $i - 1]
	set slotX [expr $internalSlot % $gSlotsX]
	set slotY [expr $internalSlot / $gSlotsX]
	set theSlot $gSlotList($slotX,$slotY)
	DisableSlot $c $theSlot
    }

    set size $CANVAS_WIDTH
    set fontsize [expr int($size / 20)]
    
    # Tell us it's "Game Over!" and announce and winner
    $c create rectangle 0 [expr $size/2 - 50] $size [expr $size/2 + 50] -fill gray -width 1 -outline black -tag "gameover"
    $c create text [expr $size/2] [expr $size/2] -text "Game Over! $nameOfWinner Wins" -font "Arial $fontsize" -fill black -tag "gameover"
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

    # Delete "Game Over!" text
    $c delete gameover
}
