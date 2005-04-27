#############################################################################
##
## NAME:         mdodgem.tcl
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
    set kGameName "Dodgem"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 100
    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "Moving your pieces home first WINS"
    set kMisereString "Moving your pieces home first LOSES"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "The pieces are intially arranged at a right angle. Players alternate turns moving their pieces to an adjacent open space. The player with pieces lined up vertically may move north, east, or south. The player with his or her pieces lind up horizontally may move north, east or west. Click on the arrow that moves the piece in the direction you wish to move. Note that you can only move to an adjacent UNOCCUPIED square and you may never move away from your goal as shown by the \"Legend\" in the upper-right."
    set kToWinStandard  "Move both of your pieces off of the board into your goal area first or trap your opponent so that he may not move."
    set kToWinMisere  "Force your opponent to move both of his pieces off the board first or trap yourself so you can't move."

    ### Set the size of the board

    global gSlotsX gSlotsY 
    set gSlotsX 4
    set gSlotsY 4
    
    ### Set the initial position of the board in our representation

    ### O - -
    ### O - -
    ### - X X

    global gInitialPosition gPosition

    set gInitialPosition 17524
    set gPosition $gInitialPosition

    ### Set what the cursors will look like. These can be the same because
    ### they will be color-coded.

    global xbmLeft xbmRight macLeft macRight kRootDir
    set xbmLeft  "$kRootDir/../bitmaps/circle.xbm"
    set xbmRight "$kRootDir/../bitmaps/circle.xbm"
    set macLeft  dot
    set macRight dot

    ### Authors Names and Photo (by AP)
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Dan Garcia"
    set kTclAuthors "Dan Garcia"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

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
    set kEditInitialPosition 0

    ### What are the default game-specific options, sir?
    global varGameSpecificOption1
    set varGameSpecificOption1 butYes
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
    
    message $w.f1.labMoves \
	    -font $kLabelFont \
	    -text "What would you like the trapping condition to be:" \
	    -width 200 \
	    -foreground $kLabelColor
    
    radiobutton $w.f1.butYes \
	    -text "Trapping your opponent WINS (you want to do it)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value butYes
    
    radiobutton $w.f1.butNo \
	    -text "Trapping your opponent LOSES (you want to avoid it)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value butNo

    ### Pack it all in
    
    pack append $w.f1 \
	    $w.f1.labMoves {left} \
	    $w.f1.butYes {top expand fill} \
	    $w.f1.butNo {top expand fill} 
    
    pack append $w \
	    $w.f1 {top expand fill}
}

#############################################################################
##
## GS_AddGameSpecificGUIOnTheFlyOptions
##
## This creates new frame(s) for GUI options that can be changed on the fly.
##
#############################################################################

proc GS_AddGameSpecificGUIOnTheFlyOptions { w } {
    ### Do nothing because there are no game-specific on-the-fly options
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
    return [list [expr {$varGameSpecificOption1 == "butYes"}] \
	    [expr {$varGameSpecificOption1 == "butYes"}]]
}

#############################################################################
##
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishSlot { w slotX slotY slot } {

    global gPosition gSlotsX gSlotsY xbmLightGrey
    global gAgainstComputer gHumanGoesFirst

    ### Now we convert the position and put pieces there

    if { $slotX == ($gSlotsX - 1) && $slotY > 0 } {
	if { $gAgainstComputer && !$gHumanGoesFirst } {
	    ### Swap
	    set thePiece [DrawPiece $slotX $slotY "O"]
	} else {
	    ### Normal
	    set thePiece [DrawPiece $slotX $slotY "X"]
	}
	$w dtag $thePiece tagPiece
	$w addtag tagPieceLabel withtag $thePiece
	$w itemconfig $thePiece -stipple @$xbmLightGrey

	### Stipple doesn't work on macs, so add light color

	global tcl_platform
	if { $tcl_platform(platform) == "macintosh" } {
	    if { $gAgainstComputer && !$gHumanGoesFirst } {
		### Swap
		$w itemconfig $thePiece -fill #FF7F7F
	    } else {
		### Normal
		$w itemconfig $thePiece -fill #7F7FFF
	    }
	}

    } elseif { $slotX != ($gSlotsX - 1) && $slotY == 0 } {
	if { $gAgainstComputer && !$gHumanGoesFirst } {
	    ### Swap
	    set thePiece [DrawPiece $slotX $slotY "X"]
	} else {
	    ### Normal
	    set thePiece [DrawPiece $slotX $slotY "O"]
	}
	$w dtag $thePiece tagPiece
	$w addtag tagPieceLabel withtag $thePiece
	$w itemconfig $thePiece -stipple @$xbmLightGrey

	### Stipple doesn't work on macs, so add light color

	global tcl_platform
	if { $tcl_platform(platform) == "macintosh" } {
	    if { $gAgainstComputer && !$gHumanGoesFirst } {
		### Swap
		$w itemconfig $thePiece -fill #7F7FFF
	    } else {
		### Normal
		$w itemconfig $thePiece -fill #FF7F7F
	    }
	}

    } elseif { $slotX == ($gSlotsX - 1) && $slotY == 0 } {
	$w itemconfig $slot -fill grey50
    } else {

	### If the position is > this number, it's one person's turn. Otherwise
	### the others.
	
	set theHalfPositions 19683
	
	set thePosition $gPosition
	
	incr thePosition [expr ($gPosition > $theHalfPositions) ? -$theHalfPositions : 0]

	set trueX $slotX
	set trueY [expr $slotY - 1]

	### Now we put the pieces on the board

	set i [expr (($gSlotsX - 1) * $trueY) + $trueX]

	set thePiece [expr int($thePosition % int(pow(3,$i+1)) / int(pow(3,$i)))]

	if { $thePiece == 1 } {
	    if { $gAgainstComputer && !$gHumanGoesFirst } {
		### Swap
		DrawPiece $slotX $slotY "O"
	    } else {
		### Normal
		DrawPiece $slotX $slotY "X"
	    }
	} elseif { $thePiece == 2 } {
	    if { $gAgainstComputer && !$gHumanGoesFirst } {
		### Swap
		DrawPiece $slotX $slotY "X"
	    } else {
		### Normal
		DrawPiece $slotX $slotY "O"
	    }
	} elseif { $thePiece == 0 } {
	    ### Do nothing
	} else {
	    BadElse "GS_EmbellishSlot"
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

    global gSlotsX gSlotsY

    ### Off the Board = 9 (from tcldodgem.c)

    set OFFTHEBOARD 9

    ### In TacTix, the move is just the slot itself.

    set fromSlot4  [lindex $theMove 0]
    set toSlot4    [lindex $theMove 1]
    set fromSlot3  [expr $fromSlot4 - ($gSlotsY-1) - ($fromSlot4/4)]

    set offTheBoard 0
    foreach i { 0 1 2 7 11 15 } {
	if { $toSlot4 == $i } {
	    set offTheBoard 1
	}
    }

    if { $offTheBoard } {
	set toSlot3 $OFFTHEBOARD
    } else {
	set toSlot3  [expr $toSlot4 - ($gSlotsY-1) - ($toSlot4/4)]
    }

    ### Does this look like SlotsToMove from tcldodgem.c or what?!
    set theBoardSize [expr ($gSlotsX - 1) * ($gSlotsY - 1) + 1]
    return [expr ($toSlot3 * $theBoardSize) + $fromSlot3]
}


#############################################################################
##
## GS_ConvertMoveToInteraction
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertMoveToInteraction { theMove } {

    ### In Dodgem, the move is just the slot itself.

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

    global gSlotSize kLabelFont gSlotsX gHumanGoesFirst gAgainstComputer

    ### Here we need to raise the text items because they need raising,
    ### otherwise they'll be under the slots the whole time...
    
    $w raise tagPiece tagSlot

    ### First I put labels there.

    set theSlotX { 0 1 2 3 3 3 }
    set theSlotY { 0 0 0 1 2 3 }

    if { $gAgainstComputer && !$gHumanGoesFirst } {
	### Swap
	set theWords { "The" "Blue" "Goal" "The" "Red" "Goal" }
	set leftColor  red
	set rightColor blue
    } else {
	### Normal
	set theWords { "The" "Red" "Goal" "The" "Blue" "Goal" }
	set leftColor  blue
	set rightColor red
    }

    set theSlotSizeHalf [expr $gSlotSize >> 1]

    for {set i 0} { $i < 6 } {incr i} {
	set theText [$w create text \
		[expr $gSlotSize * [lindex $theSlotX $i] + $theSlotSizeHalf] \
		[expr $gSlotSize * [lindex $theSlotY $i] + $theSlotSizeHalf] \
		-text [lindex $theWords $i] \
		-font $kLabelFont \
		-fill white \
		-tag tagText]
    }

    ### Let's put a "LEGEND" up there

    $w create text [expr $gSlotSize * ($gSlotsX - .4)] [expr $gSlotSize * .25] \
	    -text "Legend" \
	    -font $kLabelFont \
	    -fill white    

    ### First the pictures of the pieces

    set theSlotSize   [expr $gSlotSize / 3]
    set theOffsetX    [expr $gSlotSize * 3]
    set circleWidth   [expr $theSlotSize/10]
    set startCircle   [expr $theSlotSize/8]
    set endCircle     [expr $startCircle*7]

    set theCircle [$w create oval $startCircle $startCircle \
	    $endCircle $endCircle \
	    -outline $rightColor \
	    -fill $rightColor]
    $w move $theCircle [expr $gSlotSize * 3 + $theSlotSize] [expr $theSlotSize * 2]
    set theCircle [$w create oval $startCircle $startCircle \
	    $endCircle $endCircle \
	    -outline $leftColor \
	    -fill $leftColor]
    $w move $theCircle [expr $gSlotSize * 3] $theSlotSize
	    

    ### Then I put pictures there. First the lines.

    set theArrowList { { 3 0 } { 3 4 } { 3 6 } { 7 6 } { 7 4 } { 7 8 } }

    for {set i 0} { $i < 6 } {incr i} {
	set theMove [CreateArrow $w [lindex $theArrowList $i] $theSlotSize 3 3 cyan]
	$w move $theMove $theOffsetX 0
    }

    ### Then I order then in 2.5-D so that things are above/behind correctly

    $w raise tagText tagPiece
    $w raise tagText tagPieceLabel

    
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
    global gPosition gSlotsX gSlotsY

    ### Off the Board = 9 (from tcldodgem.c)

    set OFFTHEBOARD 9

    ### Fortunately, Dodgem's moves are already absolute.

    set theBoardSize [expr ($gSlotsX - 1) * ($gSlotsY - 1) + 1]
    set fromSlot3    [expr ($theMove % $theBoardSize)]
    set fromSlot4    [expr $fromSlot3 + $gSlotsY + ($fromSlot3 / 3)]
    set toSlot3      [expr ($theMove / $theBoardSize)]
    set toSlot4      [expr $toSlot3 + $gSlotsY + ($toSlot3 / 3)]
    
    ### Off the board, what a bad representation for a move!
    if { $toSlot3 == $OFFTHEBOARD } {
	set theHalfPositions 19683

	### X's turn
	if { $gPosition < $theHalfPositions } {
	    ### Off the right edge
	    set toSlot4 [expr $fromSlot4 + 1]
	} else {
	    ### Off the top edge
	    set toSlot4 [expr $fromSlot4 - $gSlotsX]
	}
	return [list $fromSlot4 $toSlot4]
    } else {
	return [list $fromSlot4 $toSlot4]
    }
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

    ### Check whether the move puts the piece off of the board or not

    if { $toSlotX == 3 || $toSlotY == 0 } {
	ShrinkDeleteTag $w tagPieceOnCoord$fromSlotX$fromSlotY
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

    ### No slot embellishents for the Dodgem meister.
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

    ### There has to be a better way to do this. But I'm feeling lazy

    $w delete tagPiece
    DrawPiece 0 1 "X"
    DrawPiece 0 2 "X"
    DrawPiece 1 3 "O"
    DrawPiece 2 3 "O"
}

