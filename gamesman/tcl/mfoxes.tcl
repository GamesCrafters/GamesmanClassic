#!/usr/bin/wish -f
#
#############################################################################
##
## NAME:         XGfoxes
##
## DESCRIPTION:  The source code for the Tcl component of Dodgem
##               for the Master's project GAMESMAN
##
## AUTHOR:       Dan Garcia  -  University of California at Berkeley
##               Copyright (C) Dan Garcia, 1995. All rights reserved.
##
## DATE:         05-14-95
##
## UPDATE HIST:
##
## 05-15-95 1.0    : Final release code for M.S.
##
#############################################################################

### Load the Gamesman solver
#load "../so/foxes.so" Gamesman

#############################################################################
##
## GS_InitGameSpecific
##
## This initializes the game-specific variables.
##
#############################################################################

proc GS_InitGameSpecific {} {

    ### Set the name of the game

    global kGameName
    set kGameName "Foxes and Geese"

    # Authors Info
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Sergey Kirshner, James Chung"
    set kTclAuthors "Sergey Kirshner, James Chung"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 80
    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "If you're the geese, trapping the foxes first WINS. If you're the foxes, getting by the geese first WINS"
    set kMisereString "If you're the geese, letting the foxes by first WINS. If you're the foxes, trapping your pieces first WINS"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "Click on the arrow that moves the piece in the direction you wish to move. Note that you can only move to an adjacent diagonal UNOCCUPIED square one row above."
    set kToWinStandard  "Trap your opponent, i.e. leave your opponent no legal moves."
    set kToWinMisere  "Leave your pieces no legal moves first, or trap your piece(s)."

    ### Set the size of the board

    global gSlotsX gSlotsY 
    global gTrueSlotsX gTrueSlotsY
    global gBoardSize
    set gSlotsX 8
    set gTrueSlotsX 4
    set gSlotsY 8
    set gTrueSlotsY 8
    set gBoardSize [expr $gSlotsX*$gSlotsY]
    
    ### Set the initial position of the board in our representation    

    ###  - - G -
    ### - - - -
    ###  - - - -
    ### F F F F

    ### In further version, the initial position would be uploaded
    ### from a file

    global gInitialPosition gPosition

    set gInitialPosition 82620
    set gPosition $gInitialPosition

    ### Set what the cursors will look like. These can be the same because
    ### they will be color-coded.

    global xbmLeft xbmRight
    set xbmLeft  "../bitmaps/circle.xbm"
    set xbmRight "../bitmaps/circle.xbm"

    ### Set the procedures that will draw the pieces

    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawCircle
    set kRightDrawProc DrawCircle
    set kBothDrawProc  DrawCircle

    ### What type of interaction will it be, sir?

    global kInteractionType
    set kInteractionType Rearranger

    ### Will you be needing moves to be on all the time, sir?
    
    global kMovesOnAllTheTime
    set kMovesOnAllTheTime 1

    ### Do you support editing of the initial position, sir?

    global kEditInitialPosition
    set kEditInitialPosition 1

    global gInsideArray
    set gInsideArray {}

    global gRealBoardSize
    set gRealBoardSize 32

    global gNumberPieces
    set gNumberPieces 5

    global gNumberFoxes
    set gNumberFoxes 1

    ### What are the default game-specific options, sir?
    global varGameSpecificOption1
    set varGameSpecificOption1 butGeese
    
}


#############################################################################
##
## GS_AddGameSpecificGUIOptions
##
## This initializes the game-specific variables.
##
#############################################################################

proc GS_AddGameSpecificGUIOptions { w } {

    global kLabelFont kLabelColor varGameSpecificOption1
    
    frame $w.f1 \
	    -borderwidth 2 \
	    -relief raised
    
    message $w.f1.labTurn \
	    -font $kLabelFont \
	    -text "Who do you want to go first:" \
	    -width 200 \
	    -foreground $kLabelColor
    
    radiobutton $w.f1.butFoxes \
	    -text "Foxes" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value butFoxes
    
    radiobutton $w.f1.butGeese \
	    -text "Geese" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value butGeese

    ### Pack it all in
    
    pack append $w.f1 \
	    $w.f1.labTurn {left} \
	    $w.f1.butFoxes {top expand fill} \
	    $w.f1.butGeese {top expand fill} 
    
    pack append $w \
	    $w.f1 {top expand fill}
}

#############################################################################
##
## GS_GetGameSpecificOptions
##
## If you don't have any cool game specific options, then just return 
## an empty list. Otherwise, return a list of 1s or 0s...
##
#############################################################################

proc GS_GetGameSpecificOptions {} {
    global varGameSpecificOption1
    global gInitialPosition
    global gNumberPieces
    global gNumberFoxes
    global gRealBoardSize
    
    set position_offset [expr [C_ComputeC $gRealBoardSize $gNumberPieces]*[C_ComputeC $gNumberPieces [expr $gNumberPieces - $gNumberFoxes]]]

    if { $varGameSpecificOption1 == "butFoxes" } {
	set gInitialPosition [expr $gInitialPosition + $position_offset]
    }

    return [list [expr {$varGameSpecificOption1 == "butFoxes"}] \
	    $gNumberPieces \
	    $gNumberFoxes \
	    $gRealBoardSize \
	    $gInitialPosition]
}

#############################################################################
##
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishSlot { w slotX slotY slot } {
    
    global gPosition gSlotsX gSlotsY gTrueSlotsX gTrueSlotsY xbmLightGrey
    global gAgainstComputer gHumanGoesFirst
    global gInsideArray gRealBoardSize gNumberPieces gNumberFoxes
    
    ### Now we convert the position and put pieces there
    if {[expr ($slotX+$slotY)%2==0]} {
	$w itemconfig $slot -fill #a88
    } else {
	$w itemconfig $slot -fill #111
    }

    if {[llength $gInsideArray] == 0} {

	set theHalfPositions [expr [C_ComputeC $gRealBoardSize $gNumberPieces]*\
		[C_ComputeC $gNumberPieces $gNumberFoxes]]
	set thePosition $gPosition
	incr thePosition [expr ($gPosition > $theHalfPositions) ? -$theHalfPositions : 0]

	set piecesPosition [expr $thePosition%[C_ComputeC $gRealBoardSize $gNumberPieces]]
	set insidePosition [expr $thePosition/[C_ComputeC $gRealBoardSize $gNumberPieces]]
	
	set placesLeft $gRealBoardSize
	set piecesLeft $gNumberPieces

	for {set i 0} {$i < $gRealBoardSize} {incr i} {
	    if { $placesLeft == 1 } {
		if { $piecesLeft == 1 } {
		    lappend gInsideArray 1
		} else {
		    lappend gInsideArray 0
		}
	    } elseif { $piecesLeft == 0 } {
		lappend gInsideArray 0
		incr placesLeft -1
	    } elseif { $piecesPosition < \
		    [C_ComputeC [expr $placesLeft-1] [expr $piecesLeft-1]]} {
		lappend gInsideArray 1
		incr placesLeft -1
		incr piecesLeft -1
	    } else {
		lappend gInsideArray 0
		incr placesLeft -1
		incr piecesPosition -[C_ComputeC $placesLeft [expr $piecesLeft-1]]
	    }
	}

	set tempArray {}
	set placesLeft $gNumberPieces
	set piecesLeft $gNumberFoxes
	
	for {set i 0} {$i < $gNumberPieces} {incr i} {
	    if { $placesLeft == 1 } {
		if { $piecesLeft == 1 } {
		    lappend tempArray 1
		} else {
		    lappend tempArray 0
		}
	    } elseif { $piecesLeft == 0 } {
		lappend tempArray 0
		incr placesLeft -1
	    } elseif { $insidePosition < \
		    [C_ComputeC [expr $placesLeft-1] [expr $piecesLeft-1]]} {
		lappend tempArray 1
		incr placesLeft -1
		incr piecesLeft -1
	    } else {
		lappend tempArray 0
		incr placesLeft -1
		incr insidePosition -[C_ComputeC $placesLeft [expr $piecesLeft-1]]
	    }
	}
	
	set j 0
	for { set i 0 } {$i < $gRealBoardSize} {incr i} {
	    if { [lindex $gInsideArray $i] == 1 } {
		if { [lindex $tempArray $j] == 1 } {
		    set gInsideArray [lreplace $gInsideArray $i $i 2]
		}	    
		incr j
	    }
	}
    }
    ### Now we put the pieces on the board

    if { [expr ($slotX==$gSlotsX-1) && ($slotY==$gSlotsY-1)] } {
	for {set i 0} {$i < $gSlotsX} {incr i} {
	    for {set j 0} {$j < $gSlotsY} {incr j} {
		if { [expr (($i+$j) % 2) == 1] } {
		    set trueX [expr $i / 2]
		    set trueY $j
		    set trueSlot [expr ($gTrueSlotsX*$trueY)+$trueX]
		    set temp [lindex $gInsideArray $trueSlot]
		    switch $temp {
			0       {} ;# the space on the board is empty
	  
			1       {
			    if { $gAgainstComputer && !$gHumanGoesFirst } {
				### Swap
				DrawPiece $i $j "O"
			    } else {
				### Normal
				DrawPiece $i $j "X"
			    }
			}
			
			2       {
			    if { $gAgainstComputer && !$gHumanGoesFirst } {
				### Swap
				DrawPiece $i $j "X"
			    } else {
				### Normal
				DrawPiece $i $j "O"
			    }
			}
	    
			default {BadElse "GS_EmbellishSlot"}
		    }
		}
	    }
	}
    }
}    
    

#############################################################################
##
## GS_ConvertInteractionToMove
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertInteractionToMove { theMove } {

    global gSlotsX gSlotsY gTrueSlotsX gRealBoardSize
#    global testMove
#    set testMove $theMove
    
    #puts stdout "gSlotsX: $gSlotsX   gSlotsY: $gSlotsY   gTrueSlotsX:$gTrueSlotsX" ;#tests

    set fromSlot [lindex $theMove 0]
    set fromX    [expr ($fromSlot % $gSlotsX)]
    set fromY    [expr ($fromSlot / $gSlotsX)] 
    #puts stdout "fromSlot: $fromSlot   fromX: $fromX   fromY: $fromY"  ;#tests 

    # set trueFromX $fromX
    set trueFromX [expr $fromX / 2]
    # set trueFromY [expr ($gSlotsY-1)-$fromY]
    set trueFromY $fromY
    set trueFromSlot [expr ($gTrueSlotsX*$trueFromY)+$trueFromX]

    #puts stdout "trueFromX: $trueFromX   trueFromY: $trueFromY"  ;#tests 

    set toSlot    [lindex $theMove 1]
    set toX [expr $toSlot % $gSlotsX]
    set toY [expr $toSlot / $gSlotsX]

    #puts stdout "toSlot: $toSlot   toX: $toX   toY: $toY" ;# test

    # set trueToX $toX
    set trueToX [expr $toX / 2]
    # set trueToY [expr ($gSlotsY-1)-$toY]
    set trueToY $toY
    set trueToSlot [expr ($gTrueSlotsX*$trueToY)+$trueToX]

    #puts stdout "trueToSlot: $trueToSlot   trueToX: $trueToX   trueToY: $trueToY" ;# test
    

    #puts stdout "The move for the C proc: [expr ($trueToSlot * $gRealBoardSize) + $trueFromSlot]" ;# test

    return [expr ($trueToSlot * $gRealBoardSize) + $trueFromSlot]
}


#############################################################################
##
## GS_ConvertMoveToInteraction
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertMoveToInteraction { theMove } {
#    global gRealBoardSize gSlotsX gSlotsY gBoardSize gTrueSlotsX

#    set fromSlot [expr $theMove % $gRealBoardSize]
#    set toSlot   [expr $theMove / $gRealBoardSize] 

#    set fromX [expr $fromSlot % $gTrueSlotsX]
#    set fromY [expr $fromSlot / $gTrueSlotsX]

#    set trueFromX [expr 2*$fromX]

#    set trueFromSlot [expr ($fromY*$gSlotsX)+$trueFromX]
    
#    set toX [expr $toSlot % $gTrueSlotsX]
#    set toY [expr $toSlot / $gTrueSlotsX]

#    set trueToX [expr 2*$toX]

#    set trueToSlot [expr ($toY*$gSlotsX)+$trueToX]

#    return [expr ($trueToSlot*$gBoardSize)+$trueFromSlot]

    return $theMove
}

#############################################################################
##
## GS_PostProcessBoard
##
## This allows us to post-process the board in case we need something
##
#############################################################################

proc GS_PostProcessBoard { w } {

#    global gSlotSize kLabelFont gSlotsX gHumanGoesFirst gAgainstComputer

    ### Here we need to raise the text items because they need raising,
    ### otherwise they'll be under the slots the whole time...
    
#    $w raise tagPiece tagSlot

    ### First I put labels there.

#    set theSlotX { 0 1 2 3 3 3 }
#    set theSlotY { 0 0 0 1 2 3 }

#    if { $gAgainstComputer && !$gHumanGoesFirst } {
	### Swap
#	set theWords { "The" "Blue" "Goal" "The" "Red" "Goal" }
#	set leftColor  red
#	set rightColor blue
#    } else {
	### Normal
#	set theWords { "The" "Red" "Goal" "The" "Blue" "Goal" }
#	set leftColor  blue
#	set rightColor red
#    }

#    set theSlotSizeHalf [expr $gSlotSize >> 1]

#    for {set i 0} { $i < 6 } {incr i} {
#	set theText [$w create text \
#		[expr $gSlotSize * [lindex $theSlotX $i] + $theSlotSizeHalf] \
#		[expr $gSlotSize * [lindex $theSlotY $i] + $theSlotSizeHalf] \
#		-text [lindex $theWords $i] \
#		-font $kLabelFont \
#		-fill white \
#		-tag tagText]
#    }

    ### Let's put a "LEGEND" up there

#    $w create text [expr $gSlotSize * ($gSlotsX - .4)] [expr $gSlotSize * .25] \
#	    -text "Legend" \
#	    -font $kLabelFont \
#	    -fill white    

    ### First the pictures of the pieces

#    set theSlotSize   [expr $gSlotSize / 3]
#    set theOffsetX    [expr $gSlotSize * 3]
#    set circleWidth   [expr $theSlotSize/10]
#    set startCircle   [expr $theSlotSize/8]
#    set endCircle     [expr $startCircle*7]

#    set theCircle [$w create oval $startCircle $startCircle \
#	    $endCircle $endCircle \
#	    -outline $rightColor \
#	    -fill $rightColor]
#    $w move $theCircle [expr $gSlotSize * 3 + $theSlotSize] [expr $theSlotSize * 2]
#    set theCircle [$w create oval $startCircle $startCircle \
#	    $endCircle $endCircle \
#	    -outline $leftColor \
#	    -fill $leftColor]
#    $w move $theCircle [expr $gSlotSize * 3] $theSlotSize
	    

    ### Then I put pictures there. First the lines.

#    set theArrowList { { 3 0 } { 3 4 } { 3 6 } { 7 6 } { 7 4 } { 7 8 } }

#    for {set i 0} { $i < 6 } {incr i} {
#	set theMove [CreateArrow $w [lindex $theArrowList $i] $theSlotSize 3 3 cyan]
#	$w move $theMove $theOffsetX 0
#    }

    ### Then I order then in 2.5-D so that things are above/behind correctly

#    $w raise tagText tagPiece
#    $w raise tagText tagPieceLabel

    
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
    
    global gSlotsX gSlotsY gRealBoardSize gTrueSlotsX gTrueSlotsY

    #puts stdout "GS_ConvertToAbsoluteMove - theMove:$theMove" ;#test

    set fromSlot    [expr $theMove % $gRealBoardSize]
    set toSlot      [expr $theMove / $gRealBoardSize]

    # find the trueFromSlot
    set fromX    [expr $fromSlot % $gTrueSlotsX]
    set fromY    [expr ($fromSlot / $gTrueSlotsX)]

    #puts stdout "--- fromX:$fromX --- fromY:$fromY" ;#test

    if { [expr ($fromY%2)==0] } {
	set trueFromX [expr (2*$fromX)+1]
    } else {
	# set trueFromX $fromX
	set trueFromX [expr 2 * $fromX]
    }
    # set trueFromY [expr ($gSlotsY-1)-$fromY]
    set trueFromY $fromY

    #puts stdout "--- trueFromX:$trueFromX --- trueFromY:$trueFromY" ;#test

    set trueFromSlot [expr ($gSlotsX*$trueFromY)+$trueFromX]
    
    #puts stdout "--- trueFromSlot:$trueFromSlot" ;#test

    # find the trueToSlot
    set toX [expr $toSlot % $gTrueSlotsX]
    set toY [expr $toSlot / $gTrueSlotsX]

    #puts stdout "--- toX:$toX  toY:$toY" ;#test

    if { [expr ($toY%2)==0] } {
	set trueToX [expr (2*$toX)+1]
    } else {
	# set trueToX $toX
	set trueToX [expr 2 * $toX]
    }
    # set trueToY [expr ($gSlotsY-1)-$toY]
    set trueToY $toY
    
    #puts stdout "--- trueToX:$trueToX --- trueToY:$trueToY" ;#test

    set trueToSlot [expr ($gSlotsX*$trueToY)+$trueToX]

    #puts stdout "--- trueToSlot:$trueToSlot" ;#test

    return [list $trueFromSlot $trueToSlot]
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
    global gSlotsX gSlotSize gAnimationSpeed

    ### Here we remove all the old markers for the last move

    $w delete tagLastMove

    set theAbsoluteMove [GS_ConvertToAbsoluteMove $theMove]

    set fromSlot  [lindex $theAbsoluteMove 0]
    set toSlot    [lindex $theAbsoluteMove 1]
    set fromSlotX [expr $fromSlot % $gSlotsX]
    set fromSlotY [expr $fromSlot / $gSlotsX]
    set toSlotX   [expr $toSlot % $gSlotsX]
    set toSlotY   [expr $toSlot / $gSlotsX]

    ### Try a little animation

    set timeSlices [expr int(($gSlotSize + 0.0) / ($gAnimationSpeed + 0.0))]

    set theIncrementalMoveX [expr ($toSlotX - $fromSlotX) * (($gSlotSize + 0.0) / $timeSlices)]
    set theIncrementalMoveY [expr ($toSlotY - $fromSlotY) * (($gSlotSize + 0.0) / $timeSlices)]
    set theOverallMoveX [expr $timeSlices * $theIncrementalMoveX]
    set theOverallMoveY [expr $timeSlices * $theIncrementalMoveY]

    DeleteMoves

    for {set i 1} { $i <= $timeSlices } {incr i} {
	$w move tagPieceOnCoord$fromSlotX$fromSlotY \
		$theIncrementalMoveX $theIncrementalMoveY
	update idletasks
    }

    ### Update our internal markers

    $w addtag tagPieceOnCoord$toSlotX$toSlotY withtag tagPieceOnCoord$fromSlotX$fromSlotY
    $w dtag tagPieceOnCoord$fromSlotX$fromSlotY
}

#############################################################################
##
## GS_EnableSlotEmbellishments
##
## If there are any slot embellishments that need to be enabled, now is the
## time to do it.
##
#############################################################################

proc GS_EnableSlotEmbellishments { w theSlot } {
    ### No slot embellishents for the TacTix meister.
}

#############################################################################
##
## GS_DisbleSlotEmbellishments
##
## If there are any slot embellishments that need to be enabled, now is the
## time to do it.
##
#############################################################################

proc GS_DisableSlotEmbellishments { w theSlot } {

    ### No slot embellishents for the TacTix meister.
}

#############################################################################
##
## GS_NewGame
##
## "New Game" has just been clicked. We need to reset the slots
##
#############################################################################

proc GS_NewGame { w } {
    global gAgainstComputer gHumanGoesFirst gSlotsX gSlotsY gTrueSlotsX gInsideArray

    $w delete tagPiece

    for {set i 0} {$i < $gSlotsX} {incr i} {
	for {set j 0} {$j < $gSlotsY} {incr j} {
	    if { [expr (($i+$j) % 2) == 1] } {
		set trueX [expr $i / 2]
		set trueY $j
		set trueSlot [expr ($gTrueSlotsX*$trueY)+$trueX]
		set temp [lindex $gInsideArray $trueSlot]
		switch $temp {
		    0       {} ;# the space on the board is empty
		    
		    1       {
			if { $gAgainstComputer && !$gHumanGoesFirst } {
			    ### Swap
			    DrawPiece $i $j "O"
			} else {
			    ### Normal
			    DrawPiece $i $j "X"
			}
		    }
		    
		    2       {
			if { $gAgainstComputer && !$gHumanGoesFirst } {
			    ### Swap
			    DrawPiece $i $j "X"
			} else {
			    ### Normal
			    DrawPiece $i $j "O"
			}
		    }
		    
		    default {BadElse "GS_NewGame"}
		}
	    }
	}
    }
}

### Now source the rest of the game-playing interface code.

#source "../tcl/gamesman.tcl.foxes"







