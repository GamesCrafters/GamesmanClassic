#############################################################################
##
## NAME:         mdnb.tcl
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
    set kGameName "DotsAndBoxes"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 25
    
    global dotPercent
    set dotPercent .2

    global gGSscore
    set gGSscores 0

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "n/a"
    set kMisereString "n/a"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "Click with the left button\non the line you want to draw"
    set kToWinStandard  "Score more boxes than your opponent"
    set kToWinMisere  ""

    ### Set the size of the board

    global gBoardSizeX gBoardSizeY
    set gBoardSizeX 1
    set gBoardSizeY 1
    
    global gSlotsX gSlotsY 
    set gSlotsX [expr $gBoardSizeX*2+1]
    set gSlotsY [expr $gBoardSizeY*2+1]
    
    ### Set the initial position of the board in our representation

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set what the cursors will look like. These can be the same because
    ### they will be color-coded.

    global xbmLeft xbmRight macLeft macRight kRootDir
    set xbmLeft  "$kRootDir/../bitmaps/plus.xbm"
    set xbmRight "$kRootDir/../bitmaps/plus.xbm"
#    set macLeft  X_cursor
#    set macRight circle

    # Authors Info
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Unsung Hero(s)"
    set kTclAuthors "Unsung Hero(s)"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    ### Set the procedures that will draw the pieces

    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DoNothing
    set kRightDrawProc DoNothing
    set kBothDrawProc  DoNothing

    ### What type of interaction will it be, sir?

    global kInteractionType
    set kInteractionType SinglePieceRemoval

    ### Will you be needing moves to be on all the time, sir?
    
    global kMovesOnAllTheTime
    set kMovesOnAllTheTime 0

    ### Do you support editing of the initial position, sir?

    global kEditInitialPosition
    set kEditInitialPosition 0

    global varGameSpecificOption1
    set varGameSpecificOption1 0x22
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
    set varGameSpecificOption1 0x22

    pack forget .gameSpecificOptions.f0

    frame $w.f1 \
	    -borderwidth 2 \
	    -relief raised
    
    message $w.f1.labMoves \
	    -font $kLabelFont \
	    -text "What type of Dots & Boxes would you like to play:" \
	    -width 200 \
	    -foreground $kLabelColor
    
    radiobutton $w.f1.r11 \
	    -text "1x1" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x11
    
    radiobutton $w.f1.r12 \
	    -text "1x2" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x12

    radiobutton $w.f1.r22 \
	    -text "2x2" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x22

    radiobutton $w.f1.r23 \
	    -text "2x3" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x23

    radiobutton $w.f1.r33 \
	    -text "3x3" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x33

    radiobutton $w.f1.c11 \
	    -text "Closed 1x1" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x1f
    radiobutton $w.f1.c12 \
	    -text "Closed 1x2" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x2f
    radiobutton $w.f1.c13 \
	    -text "Closed 1x3" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x3f
    radiobutton $w.f1.c14 \
	    -text "Closed 1x4" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x4f
    radiobutton $w.f1.c15 \
	    -text "Closed 1x5" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x5f
    radiobutton $w.f1.c16 \
	    -text "Closed 1x6" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x6f
    radiobutton $w.f1.c17 \
	    -text "Closed 1x7" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x7f
    radiobutton $w.f1.c18 \
	    -text "Closed 1x8" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x8f

    ### Pack it all in
    
    pack append $w.f1 \
	    $w.f1.labMoves {left} \
	    $w.f1.r11 {top expand fill} \
	    $w.f1.r12 {top expand fill} \
	    $w.f1.r22 {top expand fill} \
	    $w.f1.r23 {top expand fill} \
	    $w.f1.r33 {top expand fill} \
	    $w.f1.c11 {top expand fill} \
	    $w.f1.c12 {top expand fill} \
	    $w.f1.c13 {top expand fill} \
	    $w.f1.c14 {top expand fill} \
	    $w.f1.c15 {top expand fill} \
	    $w.f1.c16 {top expand fill} \
	    $w.f1.c17 {top expand fill} \
	    $w.f1.c18 {top expand fill} 
   
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
    global varGameSpecificOption1 gClosed
    global gInitialPosition gPosition

    global gBoardSizeX gBoardSizeY
    set gBoardSizeX [expr $varGameSpecificOption1 >> 4]
    set gBoardSizeY [expr $varGameSpecificOption1 & 0xf]
    
    if {$gBoardSizeY==0xf} {
	set gBoardSizeY 1
	set gClosed true
	set gInitialPosition [expr ((1<<$gBoardSizeX)-1)| \
		(1<<($gBoardSizeX*($gBoardSizeY+1)))| \
		(1<<($gBoardSizeX*($gBoardSizeY+2)))]
    } else {
	set gInitialPosition 0
    }

    set gPosition $gInitialPosition

    global gSlotsX gSlotsY 
    set gSlotsX [expr $gBoardSizeX*2+1]
    set gSlotsY [expr $gBoardSizeY*2+1]

    return [list $varGameSpecificOption1]
}

#############################################################################
##
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc HandleOddShapedSlotClick { x y } {
    global gSlotSize
    set w .winBoard.c
    set bbox [$w bbox current]
    set x [expr ([lindex $bbox 2] + [lindex $bbox 0])/2]
    set y [expr ([lindex $bbox 3] + [lindex $bbox 1])/2]
    set x [expr $x/$gSlotSize]
    set y [expr $y/$gSlotSize]
    HandleMove [SinglePieceRemovalCoupleMove $x $y]
}

proc GS_EmbellishSlot { w slotX slotY slot } {
    global gSlotSize
    global dotPercent

    $w itemconfig $slot -width 0
    if {($slotX&1)^($slotY&1) } { 
	# Edges
	EnableSlot $w $slot
	$w addtag tagInitial withtag $slot
	if {$slotX&1} {
	    $w scale $slot [expr ($slotX+0.5)*$gSlotSize] [expr ($slotY+0.5)*$gSlotSize] [expr 1.75 - $dotPercent] $dotPercent
	} else {
	    $w scale $slot [expr ($slotX+0.5)*$gSlotSize] [expr ($slotY+0.5)*$gSlotSize] $dotPercent [expr 1.75 - $dotPercent]
	}
    } elseif {($slotX&1) || ($slotY&1)} {
	# Box centers
	$w delete $slot
    } else {
	# Dots
	$w scale $slot [expr ($slotX+0.5)*$gSlotSize] \
		[expr ($slotY+0.5)*$gSlotSize] \
		$dotPercent $dotPercent
	$w itemconfig $slot -fill black
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
    global gSlotsX gSlotsY gBoardSizeX gBoardSizeY

    set x [expr $theMove % $gSlotsX]
    set y [expr $theMove / $gSlotsX]

    
    set vert [expr $y&1]
    set x [expr $x / 2]
    set y [expr $y / 2]

    if { !$vert} {
	set ret [expr $x+$y*$gBoardSizeX]
    } else {
	set ret [expr $gBoardSizeX*($gBoardSizeY+1)+$x*$gBoardSizeY+$y]
    }
#    puts "Player moved to I:$theMove which is now M:$ret"
    return $ret
}

#############################################################################
##
## GS_ConvertMoveToInteraction
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertMoveToInteraction { theMove } {
    global gSlotsX gSlotsY gBoardSizeX gBoardSizeY

    set dx 0
    set dy 0
    if {$theMove >= $gBoardSizeX*($gBoardSizeY+1)} {
	set move [expr $theMove - $gBoardSizeX*($gBoardSizeY+1)]
	set x [expr $move / $gBoardSizeY]
	set y [expr $move % $gBoardSizeY]
	set dy 1
    } else {
	set x [expr $theMove % $gBoardSizeX]
	set y [expr $theMove / $gBoardSizeX]
	set dx 1
    }

    set x [expr $x * 2 + $dx]
    set y [expr $y * 2 + $dy]

    set ret [expr $x+$y*$gSlotsX]
#    puts "Computer moved to M:$theMove which is now I:$ret"
    return $ret
}

#############################################################################
##
## GS_PostProcessBoard
##
## This allows us to post-process the board in case we need something
##
#############################################################################

proc GS_PostProcessBoard { w } {
    $w bind tagAlive <1> {HandleOddShapedSlotClick %x %y}
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

    
    return [GS_ConvertMoveToInteraction $theMove]
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
    global gSlotList gPosition gHumanGoesFirst
    global gBoardSizeX gBoardSizeY gPlayerOneTurn

    DisableSlot $w $theSlot
    
    set pos [C_DoMove $gPosition $theMove]


    for {set j 0;} {$j < $gBoardSizeY} {incr j} {
	for {set i 0;} {$i < $gBoardSizeX} {incr i} {
	    set newval [C_Scored $pos $i $j]
	    if {$newval==-1} {continue}
	    set oldval [C_Scored $gPosition $i $j]
	    if {$newval==$oldval} {continue}
	    if {$newval==2} {
		# our C backend isn't tracking who scored what for us
		set newval [expr !$gPlayerOneTurn]
	    } elseif {!$gHumanGoesFirst} {
		set newval [expr !$newval]
	    }
	    switch -- $newval {
		0 {DrawCross $w [expr $i*2+1] [expr $j*2+1] tagPiece blue 1}
		1 {DrawHollowCircle $w [expr $i*2+1] [expr $j*2+1] tagPiece red 1}
	    }
	    
	}
    }

    
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

    ### No slot embellishents for the TicTacToe meister.
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

    $w itemconfig $theSlot -fill black
}

#############################################################################
##
## GS_NewGame
##
## "New Game" has just been clicked. We need to reset the slots
##
#############################################################################

proc GS_NewGame { w } {

    ### Delete all pieces that are left around

    $w delete tagPiece
    $w itemconfig tagInitial -fill grey

    global gGSscore
    set gGSscores 0
}

proc DoNothing { w slotX slotY theTag theColor drawBig } {
}
