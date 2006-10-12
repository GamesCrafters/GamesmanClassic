#############################################################################
##
## NAME:         mctoi.tcl
##
## DESCRIPTION:  The source code for the Tcl component of Chung-Toi
##               for the Master's project GAMESMAN
##
## AUTHOR:       Dan Garcia, Farzad Eskafi, Erwin Vedar
##               University of California at Berkeley
##               Copyright (C) Dan Garcia, 1995. All rights reserved.
##
## DATE:         05-12-95
##
## UPDATE HIST:
##
## 05-15-95 1.0    : Final release code for M.S.
##
#############################################################################
# This is just a test -Jeffrey
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
    set kGameName "Chung-Toi"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80, but 100 is ok
    
    global gSlotSize
    set gSlotSize 240
    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "3-in-a-row WINS"
    set kMisereString "3-in-a-row LOSES"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "The players begin by taking turns placing their game pieces on the board until all the pieces are played. If neither player succeeds in getting three pieces in a row then the players continue by taking turns moving their pieces to other open spaces on the board. A piece can be moved to any open space as long as an arrow on that piece points in the direction of the desired space. A player may also change the direction of the arrows while moving. Additionally, a piece can just rotate in place or “pass” (by spinning back to the original position)."
    set kToWinStandard  "Be the first player to get three of your pieces\nin a row (horizontally, vertically or diagonally)"
    set kToWinMisere  "Force your opponent into getting three of his pieces\nin a row (horizontally, vertically or diagonally) first"

    ### Set the size of the board

    global gSlotsX gSlotsY 
    set gSlotsX 3
    set gSlotsY 3
    
    ### Set the initial position of the board in our representation

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set what the cursors will look like. These can be the same because
    ### they will be color-coded.

    global xbmLeft xbmRight macLeft macRight kRootDir
    set xbmLeft  "$kRootDir/../bitmaps/XPlus.xbm"
    set xbmRight "$kRootDir/../bitmaps/XPlus.xbm"
    set macLeft  X_cursor
    set macRight circle

    # Authors Info
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Farzad Eskafi, Erwin Vedar"
    set kTclAuthors "Farzad Eskafi, Erwin Vedar"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    ### Set the procedures that will draw the pieces

    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  DrawCross
    set kRightDrawProc DrawHollowCircle
    set kBothDrawProc  DrawCircle

    global kLeftCrossDrawProc kLeftPlusDrawProc kRightCrossDrawProc kRightPlusDrawProc 

    set kLeftCrossDrawProc  DrawCross
    set kLeftPlusDrawProc   DrawPlus
    set kRightCrossDrawProc DrawCross
    set kRightPlusDrawProc  DrawPlus

    ### What type of interaction will it be, sir?

    global kInteractionType
    set kInteractionType ChungToi

    ### Will you be needing moves to be on all the time, sir?
    
    global kMovesOnAllTheTime
    set kMovesOnAllTheTime 1

    ### Do you support editing of the initial position, sir?

    global kEditInitialPosition
    set kEditInitialPosition 0

    
    ######################
    #### added this part by Farzad and Erwin ######
    ######################
    ### What are the default game-specific options, sir?
    global varGameSpecificOption1  \
	varGameSpecificOption2 \
	varGameSpecificOption3 \
	varGameSpecificOption4 \
	varGameSpecificOption5 
    

    set varGameSpecificOption1 butYes1
    set varGameSpecificOption2 butYes2
    set varGameSpecificOption3 butYes3
    set varGameSpecificOption4 butYes4
    set varGameSpecificOption5 butYes5
    
    
    
    

}

#############################################################################
##
## GS_AddGameSpecificGUIOptions
##
## This initializes the game-specific variables.
##
#############################################################################

proc GS_AddGameSpecificGUIOptions { w } {

    ### we need 5 game specific options, since our game has 
    ### 5 main options. 
    global kLabelFont kLabelColor \
	    varGameSpecificOption1 \
	    varGameSpecificOption2 \
	    varGameSpecificOption3 \
	    varGameSpecificOption4 \
	    varGameSpecificOption5 
    
    ## We need 5 different frames for 5 different options we have.
    frame $w.f1 \
	    -borderwidth 2 \
	    -relief raised
    
    frame $w.f2 \
	    -borderwidth 2 \
	    -relief raised 
    
    frame $w.f3 \
	    -borderwidth 2 \
	    -relief raised 
    
    frame $w.f4 \
	    -borderwidth 2 \
	    -relief raised 
    
    frame $w.f5 \
	    -borderwidth 2 \
	    -relief raised 
    


    
    
    # spin in place 
    message $w.f1.spininplace \
	    -font $kLabelFont \
	    -text "Would like to allow rotation in place?" \
	    -width 200 \
	    -foreground $kLabelColor
    
    radiobutton $w.f1.butYes1 \
	    -text "Yes (Allow rotation in place)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value butYes1
    
    radiobutton $w.f1.butNo1 \
	    -text "No (Do not allow rotation in place)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption1 \
	    -value butNo1
    
    ## one space jump 
    message $w.f2.onespacejump \
	    -font $kLabelFont \
	    -text "Would like to allow single-space jumps?" \
	    -width 200 \
	    -foreground $kLabelColor
    
    radiobutton $w.f2.butYes2 \
	    -text "Yes (Allow single-space jumps)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption2 \
	    -value butYes2
    
    radiobutton $w.f2.butNo2 \
	    -text "No (Do not allow single-space jumps)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption2 \
	    -value butNo2
    
    ## two spaces jump
    message $w.f3.twospacejump \
	    -font $kLabelFont \
	    -text "Would you like to allow double-space jumps?" \
	    -width 200 \
	    -foreground $kLabelColor
    
    radiobutton $w.f3.butYes3 \
	    -text "Yes (Allow double-space jumps)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption3 \
	    -value butYes3
    
    radiobutton $w.f3.butNo3 \
	    -text "No (Do not allow double-space jumps)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption3 \
	    -value butNo3
    
    ## landing in different orientation difforientation
    message $w.f4.difforientation \
	    -font $kLabelFont \
	    -text "Would you like to allow rotation upon landing?" \
	    -width 200 \
	    -foreground $kLabelColor
    
    radiobutton $w.f4.butYes4 \
	    -text "Yes (Allow rotation upon landing)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption4 \
	    -value butYes4
    
    radiobutton $w.f4.butNo4 \
	    -text "No (Do not allow rotation upon landing)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption4 \
	    -value butNo4

    ## trapped player losing or winning  trappedplayer
    message $w.f5.trappedplayer \
	    -font $kLabelFont \
	    -text "Does the trapped player win?" \
	    -width 200 \
	    -foreground $kLabelColor
    
    radiobutton $w.f5.butYes5 \
	    -text "Yes (The trapped player wins)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption5 \
	    -value butYes5
    
    radiobutton $w.f5.butNo5 \
	    -text "No (The trapped player loses)" \
	    -font $kLabelFont \
	    -variable varGameSpecificOption5 \
	    -value butNo5
    
    ### Pack it all in
    
    ### Since we wanted them all to be aligned, they needed to be
    ### contained in different frames.  
    
    pack append $w.f1 \
	    $w.f1.spininplace {left} \
	    $w.f1.butYes1 {top expand fill} \
	    $w.f1.butNo1 {top expand fill}
    pack append $w \
	    $w.f1 {top expand fill}

    pack append $w.f2 \
	    $w.f2.onespacejump {left} \
	    $w.f2.butYes2 {top expand fill} \
	    $w.f2.butNo2 {top expand fill}
    pack append $w \
	    $w.f2 {top expand fill}
    
    pack append $w.f3 \
	    $w.f3.twospacejump {left} \
	    $w.f3.butYes3 { top expand fill} \
	    $w.f3.butNo3 {top expand fill} 
    pack append $w \
	    $w.f3 {top expand fill}

    pack append $w.f4 \
	    $w.f4.difforientation {left} \
	    $w.f4.butYes4 { top expand fill} \
	    $w.f4.butNo4 {top expand fill} 
    pack append $w \
	    $w.f4 {top expand fill}


    pack append $w.f5 \
	    $w.f5.trappedplayer {left} \
	    $w.f5.butYes5 { top expand fill} \
	    $w.f5.butNo5 {top expand fill} 
    pack append $w \
	    $w.f5 {top expand fill}
    
    
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

    global varGameSpecificOption1 \
	    varGameSpecificOption2 \
	    varGameSpecificOption3 \
	    varGameSpecificOption4 \
	    varGameSpecificOption5 
	    
    return [list \
	    [expr {$varGameSpecificOption1 == "butYes1"}] \
	    [expr {$varGameSpecificOption1 == "butYes1"}] \
	    [expr {$varGameSpecificOption2 == "butYes2"}] \
	    [expr {$varGameSpecificOption2 == "butYes2"}] \
	    [expr {$varGameSpecificOption3 == "butYes3"}] \
	    [expr {$varGameSpecificOption3 == "butYes3"}] \
	    [expr {$varGameSpecificOption4 == "butYes4"}] \
	    [expr {$varGameSpecificOption4 == "butYes4"}] \
	    [expr {$varGameSpecificOption5 == "butYes5"}] \
	    [expr {$varGameSpecificOption5 == "butYes5"}] 
	   ]
}

#############################################################################
##
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishSlot { w slotX slotY slot } {

    #### Does nothing :P

}

#############################################################################
##
## GS_ConvertInteractionToMove
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertInteractionToMove { theMove } {

    ### In TicTacToe, the move is just the slot itself.

    return $theMove
}

#############################################################################
##
## GS_ConvertMoveToInteraction
##
## This converts the user's interaction to a move to be passed to the C code.
##
#############################################################################

proc GS_ConvertMoveToInteraction { theMove } {

    ### In TicTacToe, the move is just the slot itself.

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

    ### In TicTacToe, we need no post-processing
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

    if { $theMove == "041" } {
	set theMove "41"
    }

    if { $theMove == "040"} {
	set theMove "40"
    }


    set fromSlot    [expr ($theMove /100)]
    set toSlot      [expr (($theMove % 100) / 10)]
    set orientation [expr ($theMove - ($fromSlot * 100) - ($toSlot * 10))] 

    set theMove [list $fromSlot $toSlot $orientation]

    return $theMove
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

    #### Does nothing

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

    ### No slot embellishents for the TicTacToe meister.
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

}

proc GS_AddGameSpecificGUIOnTheFlyOptions { w } {
    ### Do nothing because there are no game-specific on-the-fly options
}

