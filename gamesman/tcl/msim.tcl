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
## UPDATE HIST:  03-17-06 David Chan
##                 - converted to gamesman3 gui, code from mttt3.tcl adopted
##                   for this game
##
#############################################################################


# global variables
set g3Array [list 1 3 9 27 81 243 729 2187 6561 19683 59049 177147 531441 1594323 4782969]

set edges 15
set bgColor black
set bgColor2  grey
set lineColor cyan
set p1Color blue
set p2Color red
set vertices 6

#############################################################################
proc unhash {pos array} {
    global edges
    global g3Array
    upvar $array a

    for { set i [expr $edges - 1] } { $i >= 0 } { set i [expr $i - 1] } {
	if { $pos >= [expr 2 * [lindex $g3Array $i]] } {
	   set a($i) x
	    set pos [expr $pos - [expr 2 * [lindex $g3Array $i]]]
	} elseif { $pos >= [expr 1 * [lindex $g3Array $i]] } {
	    set a($i) o
	    set pos [expr $pos - [expr 1 * [lindex $g3Array $i]]]
	} elseif { $pos >= [expr 0 * [lindex $g3Array $i]] } {
	    set a($i) -
	    set pos [expr $pos - [expr 0 * [lindex $g3Array $i]]]
	}
    }
	    
}

#############################################################################
proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "Sim"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to complete a triangle  LOSES"
    set kMisereString "First player to complete a triangle wins"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: Avoid making a triangle of your color."
    } else {
	SetToWinString "To Win: Complete a triangle of your color before your opponent."
    }
    SetToMoveString "To Move: Players take turns connecting 2 of the 6 corners with a line. A line cannot be drawn twice and lines of the same color cannot form a triangle if the end points are part of the outer corners. Click on the stick you want to color."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Dan Garcia & Sunil Ramesh, Peter Trethewey"
    set kTclAuthors "Peter Trethewey, David Chan"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

}

#############################################################################
proc GS_NameOfPieces {} {

    return [list x o]

}

#############################################################################
proc GS_ColorOfPlayers {} {

    return [list blue red]
    
}

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
proc GS_GetOption { } {
    # TODO: Needs to change with more variants
    global gMisereGame
    set option 1
    set option [expr $option + (1-$gMisereGame)]
    return $option
}

#############################################################################
proc GS_SetOption { option } {
    # TODO: Needs to change with more variants
    global gMisereGame
    set option [expr $option - 1]
    set gMisereGame [expr 1-($option%2)]
}

#############################################################################
proc GS_Initialize { c } {

    # you may want to start by setting the size of the canvas; this line isn't cecessary
    #$c configure -width 500 -height 500
    global bgColor
    global bgColor2
    global lineColor
    global edges
    global vertices

    global gFrameWidth gFrameHeight

    $c create rect 0 0 $gFrameWidth $gFrameWidth -fill $bgColor -tag bkgnd
    $c create oval 0 0 $gFrameWidth $gFrameWidth -fill $bgColor2 -tag bkgnd2
    
    set i 0
    for {set x 0} {$x < $vertices} {incr x} {
	for {set y [expr {$x+1}]} {$y < $vertices} {incr y} {
	    set p 1.0
	    set q 1.0
	    set p [expr {$gFrameWidth/2.0}]
	    set q [expr {($gFrameWidth*0.9)/2.0}]
	    set pi 3.14159265

	    $c create line \
		[expr {$p+$q*cos($x*$pi/3)}] \
		[expr {$p+$q*sin($x*$pi/3)}] \
		[expr {$p+$q*cos($y*$pi/3)}] \
		[expr {$p+$q*sin($y*$pi/3)}] \
		-width 15 -fill $lineColor -capstyle round \
		-tag [list base base$i]

	    $c create line \
		[expr {$p+$q*cos($x*$pi/3)}] \
		[expr {$p+$q*sin($x*$pi/3)}] \
		[expr {$p+$q*cos($y*$pi/3)}] \
		[expr {$p+$q*sin($y*$pi/3)}] \
		-width 15 -fill $lineColor -capstyle round \
		-tag [list vm vm-$i]

	    $c bind base$i <ButtonRelease-1> "ReturnFromHumanMove $i"
	    $c bind vm-$i <ButtonRelease-1> "ReturnFromHumanMove $i"
	    incr i
	}
    }
} 

#############################################################################
proc GS_Deinitialize { c } {
    $c delete all
}

#############################################################################
proc GS_DrawPosition { c position } {
    global edges
    global lineColor
    global p1Color
    global p2Color
    global valueMoves

    set a(0) (0)

    unhash $position a

    for {set i 0} {$i < $edges} {incr i} {
	if {$a($i) == "x"} {
	    $c itemconfigure base$i -fill $p1Color
	} elseif {$a($i) == "o"} {
	    $c itemconfigure base$i -fill $p2Color
	} else {
	    $c itemconfigure base$i -fill $lineColor
	}
	$c raise base$i
    }
}

#############################################################################
proc GS_NewGame { c position } {
    GS_DrawPosition $c $position
}

#############################################################################
proc GS_WhoseMove { position } {
    # code copied from mttt3.tcl
    global edges
    set a(0) 0

    unhash $position a
    set counter 0
    for {set i 0} {$i < $edges} {incr i} {
	if {$a($i) != "-"} {
	    incr counter
	}

	set val o
	if ( [expr $counter % 2] == 0 } {
	    set val x
	}

	return $val
}

#############################################################################
proc GS_HandleMove { c oldPosition theMove newPosition } {
    GS_DrawPosition $c $newPosition
}

#############################################################################
proc GS_ShowMoves { c moveType position moveList } {
    foreach item $moveList {
	set move [lindex $item 0]
	set value [lindex $item 1]
	set color cyan

	$c raise vm-$move base
	if {$moveType == "value"} {
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} {
		set color green
	    } else {
		set color red4
	    }
	}
	$c itemconfigure vm-$move -fill $color
    }
}

#############################################################################
proc GS_HideMoves { c moveType position moveList} {
    $c lower vm base
}

#############################################################################
proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {
    GS_DrawPosition $c $positionAfterUndo
}

#############################################################################
proc GS_GetGameSpecificOptions { } {
}

#############################################################################
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove} {
}

#############################################################################
proc GS_UndoGameOver { c position } {
}
