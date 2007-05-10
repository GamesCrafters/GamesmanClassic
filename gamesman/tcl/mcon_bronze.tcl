#############################################################################
##
## NAME:         mcon.tcl
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
    set kGameName "Connections"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 25
    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "n/a"
    set kMisereString "n/a"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "Players alternate turns connecting the open spots. To place a piece, select an open spot."
    set kToWinStandard  "Get your colored pieces to connect across the board or surround your opponent."
    set kToWinMisere  "Force your opponent to connect his colored pieces across the board."

    ### Set the size of the board

    global gBoardSize
    global gBoardSizeX gBoardSizeY
    set gBoardSizeX 1
    set gBoardSizeY 2
    set gBoardSize 4
    
    global gSlotsX gSlotsY 
    set gSlotsX [expr $gBoardSizeX+1]
    set gSlotsY [expr $gBoardSizeY+1]
    
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
    set kCAuthors "Brian Carnes"
    set kTclAuthors "Brian Carnes"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    ### Set the procedures that will draw the pieces

    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawMyPiece
    set kRightDrawProc DrawMyPiece
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
    return 0;

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

    radiobutton $w.f1.c11 \
	    -text "Closed 1x1" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x81
    radiobutton $w.f1.c12 \
	    -text "Closed 1x2" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x82
    radiobutton $w.f1.c13 \
	    -text "Closed 1x3" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x83
    radiobutton $w.f1.c14 \
	    -text "Closed 1x4" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value 0x84

    ### Pack it all in
    
    pack append $w.f1 \
	    $w.f1.labMoves {left} \
	    $w.f1.r11 {top expand fill} \
	    $w.f1.r12 {top expand fill} \
	    $w.f1.r22 {top expand fill} \
	    $w.f1.r23 {top expand fill} 
#	    $w.f1.c11 {top expand fill} \
#	    $w.f1.c12 {top expand fill} \
#	    $w.f1.c13 {top expand fill} \
#	    $w.f1.c14 {top expand fill} \
    
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

    global gBoardSizeX gBoardSizeY
    set gBoardSizeX [expr $varGameSpecificOption1 >> 4]
    set gBoardSizeY [expr $varGameSpecificOption1 & 0xf]
    
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

proc HandleEdges { w enable disable} {
    global gBoardSize gSlotList
    for {set j 0} {$j <= $gBoardSize} {incr j} {
	for {set i 0} {$i <= $gBoardSize} {incr i} {
	    set slot $gSlotList($i,$j)

	    if {[lsearch -exact [$w gettags $slot] $disable]!=-1} {
		$w dtag $slot tagAlive 
		$w addtag tagDead withtag $slot
	    }
	    if {[lsearch -exact [$w gettags $slot] $enable]!=-1} {
		$w dtag $slot tagDead
		$w addtag tagAlive withtag $slot
	    }
	}
    }
}

proc GS_EmbellishSlot { w slotX slotY slot } {
    global gSlotSize gBoardSize
    
    $w itemconfig $slot -width 0
    set pos [C_XYtoPos $slotX $slotY]
    switch -- $pos {
	-1 {$w addtag tagV with $slot}
	-2 {$w addtag tagH with $slot}
	-99 {}
	default {
	    if {$slotX==0 || $slotX==$gBoardSize} {
		$w addtag tagInitialLR with $slot
	    } else {
		EnableSlot $w $slot
		$w addtag tagInitial with $slot

		if {$slotY==0 || $slotY==$gBoardSize} {
		    $w addtag tagInitialTB with $slot
		} 
	    }
	}
    }
    
    if {$pos == -1 || $pos == -2} {
	$w scale $slot [expr ($slotX+0.5)*$gSlotSize] [expr ($slotY+0.5)*$gSlotSize] .4 .4
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
    set slotX   [expr $theMove % $gSlotsX]
    set slotY   [expr $theMove / $gSlotsX]

    set move [C_XYtoPos $slotX $slotY]
    return $move
}

#############################################################################
##
## GS_ConvertMoveToInteraction
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertMoveToInteraction { theMove } {
    
    return [C_MoveToInter $theMove]
}

#############################################################################
##
## GS_PostProcessBoard
##
## This allows us to post-process the board in case we need something
##
#############################################################################

proc GS_PostProcessBoard { w } {
    global gVcolor gHcolor gHumanGoesFirst

    if {$gHumanGoesFirst} {
	set gVcolor blue
	set gHcolor red
    } else {
	set gVcolor red
	set gHcolor blue
    }

    $w itemconfig tagV -fill $gVcolor
    $w itemconfig tagH -fill $gHcolor
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
    global gSlotList gPosition
    global gBoardSizeX gBoardSizeY

    DisableSlot $w $theSlot
    
    set pos [C_DoMove $gPosition $theMove]
    
    set turn [C_GetTurn $gPosition]

    if {$turn} {
	HandleEdges $w tagInitialTB tagInitialLR
    } else {
	HandleEdges $w tagInitialLR tagInitialTB
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
    HandleEdges $w tagInitialTB tagInitialLR
}

proc DoNothing { w slotX slotY theTag theColor drawBig } {
}

proc DrawMyPiece { w slotX slotY theTag theColor drawBig } {
    global gSlotSize
    global gVcolor 

    set Width [expr $gSlotSize]
    set Height [expr $gSlotSize]
    set thePiece [$w create rect 0 0 $Height $Width \
		       -width 0 \
		       -fill $theColor \
		       -tag $theTag]

    if {(($slotX&1)&&($slotY&1))^($gVcolor == $theColor)} {
	$w scale $thePiece [expr $gSlotSize*.5] [expr $gSlotSize*.5] 1.6 .2
    } else {
	$w scale $thePiece [expr $gSlotSize*.5] [expr $gSlotSize*.5] .2 1.6
    }
    $w move $thePiece [expr $slotX*$gSlotSize] [expr $slotY*$gSlotSize]
    $w addtag tagPieceCoord$slotX$slotY withtag $thePiece
    $w addtag tagPieceOnCoord$slotX$slotY withtag $thePiece

    return $thePiece
}
