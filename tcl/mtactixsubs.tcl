#############################################################################
##
## NAME:         mtactixsubs.tcl
##
## DESCRIPTION:  The source code for the editor for the initial position of
##               the game tactix. Also known as subroutines for mtactix, so
##               the name is mtactixsubs.tcl
##
## AUTHOR:       Dan Garcia  -  University of California at Berkeley
##               Copyright (C) Dan Garcia, 1995. All rights reserved.
##
## DATE:         05-14-95
##
## UPDATE HIST:
##
## 05-15-95 1.0    : Final release code for M.S.
## 96-07-04 1.01   : Fix for 7.5 involving adding int() call to expr
##                   since in 7.3, expr 1/2 = 0 but in 7.5 1/2 = 0.5
##                   AND fixed a bug that meant 'cancel' actually didn't
##                   cancel the move, it actually remembered it.
##
#############################################################################

#############################################################################
##
## TagToColor
##
## Depending whether the tag is Alive or Dead, we return different colors
##
#############################################################################

proc TagToColor { theTag } {
    if { $theTag != "tagEditBoardAlive" } {
	return grey
    } else {
	return magenta
    }
}
    

############################################################################
##
## DrawBlankBoard
##
## Here we actually draw the board depending what the position already is.
## This allows us to have a position partially saved, then go in and load
## a new one up immediately.
##
############################################################################

proc DrawBlankBoard { w } {

    global gSlotSize gPosition gSlotsX gSlotsY

    $w config \
        -width  [expr $gSlotsX*$gSlotSize] \
        -height [expr $gSlotsY*$gSlotSize]

    for {set i 0} {$i < $gSlotsX} {incr i} {
	for {set j 0} {$j < $gSlotsY} {incr j} { 
	    set theIndex [SinglePieceRemovalCoupleMove $i $j]
	    if { $gPosition & int(pow(2,$theIndex)) } {
		CreateEditSlot $w $i $j tagEditBoardAlive
	    } else {
		CreateEditSlot $w $i $j tagEditBoardDead
	    }
	}
    }
}

############################################################################
##
## CreateEditSlot
##
## This creates one of the slots (pieces) which'll be either there or not
## there depending on whether the tag is Alive or dead.
##
############################################################################

proc CreateEditSlot { w slotX slotY theTag } {
    global gSlotSize

    set circleWidth [expr $gSlotSize/10]
    set startCircle [expr $gSlotSize/8]
    set endCircle   [expr $startCircle*7]
    set cornerX     [expr $slotX*$gSlotSize]
    set cornerY     [expr $slotY*$gSlotSize]

    set theBackground [$w create rect \
        [expr $gSlotSize*$slotX] \
        [expr $gSlotSize*$slotY] \
        [expr [expr $gSlotSize*[expr $slotX+1]]+1] \
        [expr [expr $gSlotSize*[expr $slotY+1]]+1] \
        -outline black \
        -width 1 \
        -fill grey]

    set slot [$w create oval $startCircle $startCircle \
		       $endCircle $endCircle \
		       -outline black \
		       -width 1 \
		       -fill [TagToColor $theTag] \
		       -tag tagSlot]

    $w move $slot $cornerX $cornerY
    $w raise $slot $theBackground

    $w addtag $theTag withtag $slot
    $w addtag [list $slotX $slotY] withtag $slot

    $w bind tagEditBoardDead <Double-Button-1> "HandleTacTixDoubleClick $w \[expr %x/$gSlotSize\] \[expr %y/$gSlotSize\] "
    $w bind tagEditBoardAlive <Double-Button-1> "HandleTacTixDoubleClick $w \[expr %x/$gSlotSize\] \[expr %y/$gSlotSize\] "
    $w bind tagEditBoardDead <1> " HandleTacTixClick $w \[expr %x/$gSlotSize\] \[expr %y/$gSlotSize\] "
    $w bind tagEditBoardAlive <1> " HandleTacTixClick $w \[expr %x/$gSlotSize\] \[expr %y/$gSlotSize\] "
#    $w bind tagEditBoardDead <3> { TheHelp "This is an unselected slot. Clicking on this in toggle-mode or dragging the cursor over it in drag-select-mode will select it and turn it gray." }
#    $w bind tagEditBoardAlive <3> { TheHelp "This is a selected slot. Clicking on this in toggle-mode or dragging the cursor over it in drag-unselect-mode will unselect it and turn it off-yellow." }
    $w bind tagEditBoardDead <Enter> "HandleTacTixEnter $w \[expr %x/$gSlotSize\] \[expr %y/$gSlotSize\] "
    $w bind tagEditBoardAlive <Enter> "HandleTacTixEnter $w \[expr %x/$gSlotSize\] \[expr %y/$gSlotSize\] "
}

############################################################################
##
## HandleTacTixEnter
##
## Here we handle when the cursor goes into one of the slots.
##
############################################################################

proc HandleTacTixEnter { w x y } {
    global ToggleMode

    set theTags [$w gettags current]
    if { $ToggleMode != "notTurning" } {
	if { ([lsearch $theTags tagEditBoardDead] != -1) && ($ToggleMode == "TurningOn") } {
	    $w dtag current tagEditBoardDead
	    $w addtag tagEditBoardAlive withtag current
	    $w itemconfig current -fill magenta
	} 
	if { ([lsearch $theTags tagEditBoardAlive] != -1) && ($ToggleMode == "TurningOff") } {
	    $w addtag tagEditBoardDead withtag current
	    $w dtag current tagEditBoardAlive
	    $w itemconfig current -fill grey
	}
    }
}


############################################################################
##
## HandleTacTixDoubleClick
##
## Here we handle when the cursor double-clicks inside one of the slots.
##
############################################################################

proc HandleTacTixDoubleClick { w x y } {
    global ToggleMode xbmCircleAdd xbmCircleSubtract

    set theTags [$w gettags current]
    if { $ToggleMode == "notTurning" } {
	if { [lsearch $theTags tagEditBoardDead] != -1 } {
	    set ToggleMode TurningOff
	    ## $w config -cursor "@$xbmCircleSubtract black" ## pre-Mac
	    $w config -cursor exchange
	    .winEditBoard.f1.mes config -text "You are now in drag-unselect-mode.\nDrag the cursor over any slot to unselect it.\nClick once to leave this mode."
	} else {
	    .winEditBoard.f1.mes config -text "You are now in drag-select-mode.\nDrag the cursor over any slot to select it.\nClick once to leave this mode."
	    set ToggleMode TurningOn 
	    ## $w config -cursor "@$xbmCircleAdd black" ## pre-Mac
	    $w config -cursor plus
	}
    } else {
	set ToggleMode notTurning
	$w config -cursor {}
	if { [lsearch $theTags tagEditBoardDead] != -1 } {
	    $w dtag current tagEditBoardDead
	    $w addtag tagEditBoardAlive withtag current
	    $w itemconfig current -fill grey
	} else {
	    $w addtag tagEditBoardDead withtag current
	    $w dtag current tagEditBoardAlive
	    $w itemconfig current -fill magenta
	}
    }
}

############################################################################
##
## HandleTacTixClick
##
## Here we handle when the cursor clicks inside one of the regions
##
############################################################################

proc HandleTacTixClick { w x y } {
    global ToggleMode

    set theTags [$w gettags current]
    if { $ToggleMode != "notTurning" } {
	set ToggleMode notTurning
	$w config -cursor {}
	.winEditBoard.f1.mes config -text "You are now in toggle-mode.\nClicking once toggles a board slot on or off.\nDouble-clicking changes to drag-mode."
    } else {
	if { [lsearch $theTags tagEditBoardDead] != -1 } {
	    $w dtag current tagEditBoardDead
	    $w addtag tagEditBoardAlive withtag current
	    $w itemconfig current -fill magenta
	} else {
	    $w addtag tagEditBoardDead withtag current
	    $w dtag current tagEditBoardAlive
	    $w itemconfig current -fill grey
	}
    }
}

############################################################################
##
## MakeBoardNumber
##
## Here we convert the physical board to a number which representas the
## board and is used internally to represent the board.
##
############################################################################

proc MakeBoardNumber { w theTag } {
    global gSlotSize
    set theBoard 0

    foreach slot [$w find withtag $theTag] {
        set thecoords [$w coords $slot]
	set slotX [expr int([lindex $thecoords 0]/$gSlotSize)]
	set slotY [expr int([lindex $thecoords 1]/$gSlotSize)]
	set theIndex [SinglePieceRemovalCoupleMove $slotX $slotY]
        incr theBoard [expr int(pow(2,$theIndex))]
    }
    return $theBoard
}

############################################################################
##
## DoEditInitialPosition
##
## The main procedure called by the "Edit Initial Positions" button. Set
## up the board, put it to the right of the current board, and then fill
## it with the current position.
##
############################################################################

proc DoEditInitialPosition { } {
    global ToggleMode
    global kLabelFont
    global gSlotsX gSlotsY
    global gPosition gInitialPosition
    global thePositionItCameInWith

    set ToggleMode notTurning
    set thePositionItCameInWith $gInitialPosition

    .f5.butInitialPosition config -state disabled

    toplevel .winEditBoard
    wm title .winEditBoard "Edit your own Tac Tix position"
    wm geometry .winEditBoard [GeometryRightOf .]

    canvas .winEditBoard.c -relief raised

    frame .winEditBoard.f0
    button .winEditBoard.f0.butCancel \
	    -text "Cancel" \
	    -font $kLabelFont \
	    -command {
	.f5.butInitialPosition config -state normal
	destroy .winEditBoard
        set gInitialPosition $thePositionItCameInWith
	set gPosition $gInitialPosition
    }
    button .winEditBoard.f0.butOk \
	    -text "OK" \
	    -font $kLabelFont \
	    -command {
	set gInitialPosition [MakeBoardNumber .winEditBoard.c tagEditBoardAlive]
	set gPosition $gInitialPosition
	.f5.butInitialPosition config -state normal
	destroy .winEditBoard
    }

    button .winEditBoard.f0.but33 \
	    -text "Full 3x3" \
	    -font $kLabelFont \
	    -command {
	set gInitialPosition 1911
	set gPosition $gInitialPosition
	.winEditBoard.c delete tagSlot
	DrawBlankBoard .winEditBoard.c
    }

    button .winEditBoard.f0.but44 \
	    -text "Full 4x4" \
	    -font $kLabelFont \
	    -command {
	set gInitialPosition 65535
	set gPosition $gInitialPosition
	.winEditBoard.c delete tagSlot
	DrawBlankBoard .winEditBoard.c
    }

    button .winEditBoard.f0.butRandom \
	    -text "Random" \
	    -font $kLabelFont \
	    -command {
	### 65536 = 2^4 (unfortunately hardcoded here)
	set gInitialPosition [C_Random 65536]
	set gPosition $gInitialPosition
	.winEditBoard.c delete tagSlot
	DrawBlankBoard .winEditBoard.c
    }

    frame .winEditBoard.f1 \
        -borderwidth 2 \
        -relief raised

    label .winEditBoard.f1.labValue \
        -font $kLabelFont \
        -anchor w \
        -text "" \
        -foreground red

    message .winEditBoard.f1.mes \
	-justify center \
        -font $kLabelFont \
        -text "You are now in toggle mode.\nClicking once toggles a board slot on or off.\nDouble-clicking changes to drag-mode." \
	-width 700 \
	-foreground black

    pack append .winEditBoard.f1 \
	    .winEditBoard.f1.mes { left fill expand }

    pack append .winEditBoard.f0 \
	    .winEditBoard.f0.butOk { left expand fill } \
	    .winEditBoard.f0.but33 { left expand fill } \
	    .winEditBoard.f0.but44 { left expand fill } \
	    .winEditBoard.f0.butRandom { left expand fill } \
	    .winEditBoard.f0.butCancel {left expand fill}

    pack append .winEditBoard .winEditBoard.f1 { top fill expand } \
	.winEditBoard.c top .winEditBoard.f0 { bottom fill expand }

    DrawBlankBoard .winEditBoard.c

#    bind .winEditBoard.f1.mes <3> { TheHelp "This is your status window, telling you which editing mode you are in, either toggle-mode, drag-select-mode, or drag-unselect-mode (or pie ala mode)."}
#    bind .winEditBoard.f0.butOk <3> { TheHelp "This button will accept the current edited position and change the \"Domineering\" window to contain this position."}
#    bind .winEditBoard.f0.butCancel <3> { TheHelp "This button cancels the \"Edit...\" request, closes this window and pretends the user never clicked the \"Edit...\" button. " }
}   

global xbmCircleAdd xbmCircleSubtract kRootDir
set xbmCircleAdd      "$kRootDir/../bitmaps/circleadd.xbm"
set xbmCircleSubtract "$kRootDir/../bitmaps/circlesubtract.xbm"