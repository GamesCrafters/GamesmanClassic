#########################################################################
##
## NAME:         msimtcl
##
## DESCRIPTION:  The source code for the Tcl component of sim
##               for the Master's project GAMESMAN
##
## AUTHOR:       Peterson Trethewey
##               Copyright (C) Peterson Trethewey, 2002. All rights reserved.
##
## DATE:         05-13-02
##
## UPDATE HIST:
##
#############################################################################

#############################################################################
##
## GS_InitGameSpecific
##
## This initializes the game-specific variables.
##
#############################################################################



#values to massage...

set grandwidth 500



proc GS_InitGameSpecific {} {
    
    global grandwidth
    global altcolor
    global p1Color
    global p2Color
    global gHumanGoesFirst
    
    
    ### Set the name of the game

    global kGameName
    set kGameName "Sim"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80 (Why's that?)
    
    global gSlotSize
    set gSlotSize $grandwidth
    
    ### Set the strings to be used in the Edit Rules
    
    global kStandardString kMisereString
    set kStandardString "Completing a triangle loses"
    set kMisereString "Completing a triangle wins"
    
    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "Click on the stick you want to color."
    set kToWinStandard  "Avoid making a triangle of your color."
    set kToWinMisere  "Complete a triangle before your opponent."
    
    ### Set the size of the board
    
    global gSlotsX gSlotsY
    set gSlotsX 1
    set gSlotsY 1
    
    ### Set the initial position of the board in our representation
    
    global gInitialPosition gPosition
    set gInitialPosition 0
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
    set kCAuthors "Dan Garcia, Sunil Ramesh, Peter Trethewey"
    set kTclAuthors "Peter Trethewey"
    set kGifAuthors "$kRootDir/../bitmaps/lite3team.gif"
    
    ### Set the procedures that will draw the pieces
    ### (I prefer to do this myself)
    
    global kLeftDrawProc kRightDrawProc kBothDrawProc
    set kLeftDrawProc  empty
    set kRightDrawProc empty
    set kBothDrawProc  empty
    
    ### What type of interaction will it be, sir?
    
    global kInteractionType
    set kInteractionType Custom
    
    ### Will you be needing moves to be on all the time, sir?
    
    global kMovesOnAllTheTime
    set kMovesOnAllTheTime 0
    
    ### Do you support editing of the initial position, sir?
    
    global kEditInitialPosition
    set kEditInitialPosition 0
    
    ### What are the default game-specific options, sir?
    ## none at the moment
    
    
}

#############################################################################
##
## GS_AddGameSpecificGUIOptions
##
## This initializes the game-specific variables.
##
#############################################################################

proc GS_AddGameSpecificGUIOptions { w } {
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
}

set bgColor #A0F0F0
set chanColor #FFFFFF
set volColor  #E0E0E0
set dullColor #F7F7F7
set navyColor #000350
set greenColor #006010
set peachColor #FFE0D0
set redColor #701010
set p1Color blue
set p2Color red
set highlightColor #E0E000


#############################################################################
##
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishSlot { w slotX slotY slot } {
    
    global bgColor
    global chanColor
    global volColor
    global dullColor
    global navyColor
    global greenColor
    global peachColor
    global redColor
    global p1Color
    global p2Color
    global highlightColor
    global grandwidth
    
    
    set base $w
    
    ## background
    
    $base create rect 0 0 [expr {$grandwidth}] [expr {$grandwidth}] -fill $navyColor
    $base create oval 0 0 [expr {$grandwidth}] [expr {$grandwidth}] -fill $peachColor
    
    
    ## six lineserother
    
    set i 0
    
    
    for {set x 0} {$x < 6} {incr x} {
	for {set y [expr {$x+1}]} {$y < 6} {incr y} {
	    set c 1.0
	    set r 1.0
	    set c [expr {$grandwidth/2.0}]
	    set r [expr {($grandwidth*0.9)/2.0}]
	    set pi 3.14159265
	    
	    ## one line between each pair of points x, y that are x*pi/3 radians around
	    
	    
	    ## set item [$base create line [expr {$c+$r*cos($x*$pi/3)}] [expr {$c+$r*sin($x*$pi/3)}] [expr {$c+$r*cos($y*$pi/3)}] [expr {$c+$r*sin($y*$pi/3)}]	-width 30 -fill $chanColor]
	    
	    ## $base addtag tagClickable-$i withtag $item
	    ## $base addtag tagIndicator-$i withtag $item
	    ## $base addtag tagIndicator withtag $item
	    
	    set item [$base create line [expr {$c+$r*cos($x*$pi/3)}] [expr {$c+$r*sin($x*$pi/3)}] [expr {$c+$r*cos($y*$pi/3)}] [expr {$c+$r*sin($y*$pi/3)}]	-width 15 -fill $volColor -capstyle round]
	    
	    $base addtag tagClickable-$i withtag $item
	    $base addtag tagWasMove-$i withtag $item
	    $base addtag tagMove-$i withtag $item
	    $base addtag tagMove withtag $item
	    
	    $base bind tagClickable-$i <Any-1> "takeClick $w %x %y $i"
	    $base bind tagClickable-$i <Any-Enter> "enter $w %x %y $i"
	    $base bind tagClickable-$i <ButtonRelease-1> "letUp $w %x %y $i"
	    $base bind tagClickable-$i <Any-Leave> "Leave $w %x %y $i"
	    $base addtag tagIndicator-$i withtag $item
	    $base addtag tagIndicator withtag $item
	    
	    incr i
	}
    }
    
    
    
}





###################################
## My own event handling follows ##
###################################


set squareisdown false
set buttonisdown false
set numberOfPaths 0
set paths(0) 0
set burstingPlayer 1



proc enter {w x y i} {
    $w raise current
}


proc takeClick {w x y i} {
    global highlightColor

    SetColorByTag $w tagMove-$i $highlightColor
 
    set buttonisdown true
}


proc Leave {w x y i} {
    global buttonisdown
    global volColor
    ## SetColorByTag $w tagMove-$i $volColor
    set buttonisdown false
}

proc letUp {w x y i} {
    global volColor
    global buttonisdown
    global available
    
    SetColorByTag $w tagMove-$i $volColor
    
    if { $available($i) } {
	HandleMove $i
    }
    set buttonisdown true
}




set altcolor $p1Color
for {set i 0} {$i < 16} {incr i} {
    set available($i) true
}


proc forceMove {w theMove color} {
    global altcolor
    global p1Color
    global p2Color
    global available
    
    SetColorByTag $w tagMove-$theMove $altcolor
    $w dtag tagMove-$theMove
    set available($theMove) false
    
    if {$altcolor == $p1Color} {
	set altcolor $p2Color
    } else {
	set altcolor $p1Color
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

    ### Fortunately, TicTacToe's moves are already absolute.

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
    global chanColor
    global p1Color
    global gSlotsX
    global gSlotsY
    
    SetColorByTag $w tagMove-$theMove $chanColor
    forceMove $w $theMove $p1Color
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
    
    global bgColor
    global chanColor
    global volColor
    global dullColor
    global navyColor
    global greenColor
    global peachColor
    global redColor
    global p1Color
    global p2Color
    global altcolor
    global available
    global gHumanGoesFirst
    
    if {$gHumanGoesFirst} {
	set altcolor $p1Color
    } else {
	set altcolor $p2Color
    }
    
    SetColorByTag $w tagIndicator $chanColor
    SetColorByTag $w tagMove $volColor
    for {set i 0} {$i < 16} {incr i} {
	$w addtag tagMove-$i withtag tagWasMove-$i
	set available($i) true
    }
    
}


proc SetColorByTag { w tag color } {
    foreach item [$w find withtag $tag] {
	$w itemconfig $item -fill $color
    }
}

proc SetColorAndOutlineByTag { w tag color } {
    foreach item [$w find withtag $tag] {
	$w itemconfig $item -fill $color
	$w itemconfig $item -outline $color
    }
}



proc empty {w a b c d e} {
}














