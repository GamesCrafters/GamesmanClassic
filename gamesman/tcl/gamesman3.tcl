############################################################################
##
## gamesman3.tcl
##
## LAST CHANGE: $Id: gamesman3.tcl,v 1.46 2006-04-17 08:52:47 scarr2508 Exp $
##
############################################################################

############################################################################
##
## Source The Utility Functions
##
##
##
#############################################################################

proc stack {} {
    return [list]
}

proc push {l a} {
    return [linsert $l 0 $a]
}

proc pop {l} {
    return [lrange $l 1 [expr [llength $l] - 1]]
}

proc peek {l} {
    return [lindex $l 0]
}

## returns whether or not the list l contains the key a
proc containskey {a l} {
    set retval false

    foreach item $l {
        if {[lindex $item 0] == $a} {  
            set retval true
        } 
    }
    return $retval
}

##### other utility function

proc MoveValueToColor { moveType value } {
    set color cyan
    if {$moveType == "value"} {
	if {$value == "Tie"} {
	    set color yellow
	} elseif {$value == "Lose"} {
	    set color green
	} else {
	    set color red4
	}
    } elseif {$moveType == "all"} {
	set color cyan
    }
    return $color
}

##### animation utility functions
###
 # This function takes a number associated with a function and adjusts it
 # mathematically, taking into account the value of the animationSpeed
 # scrollbar.
 ##
proc ScaleDownAnimation { norm } {
    #the exponential base
    set base 2
    #what value of gAnimationSpeed causes norm to be returned unaltered?
    #this is dependent on what value is at the middle of the animation speed
    #scrollbar.  
    set median 0
    global gAnimationSpeed
    return [expr $norm * pow($base, [expr $median - $gAnimationSpeed])]
}

###
 # This function takes a number associated with a function and adjusts it
 # mathematically, taking into account the value of the animationSpeed
 # scrollbar.  It's basic contract is that it is the inverse operation of 
 # whatever ScaleDownAnimation did.  So, for example, if an animation should
 # occur in a certain amount of time, then the animator's choice of using
 # ScaleDownAnimation to decrease the amount of time between frames or using
 # ScaleUpAnimation to increase the number of frames MUST be equivalent.
 ##
proc ScaleUpAnimation { norm } {
    set base 2
    set median 0
    global gAnimationSpeed
    return [expr $norm * pow($base, [expr $gAnimationSpeed - $median])]
}

#############################################################################
##
## DrawArrow
##
## This procedure is used to abstract away the drawing of polygonal arrows
## even more.
##
## Input: canvas, starting coordinate, ending coordinate, arrow width
##
## Gamesman's arrow properties are defined in a 2:2:1 ratio to the width.
##
#############################################################################

proc DrawArrow {canvas startx starty endx endy width} {
    
    set arrowTip [expr 2*$width]
    set arrowBase [expr 2*$width]
    set arrowSides [expr 1*$width]

    return [PolyArrow $canvas $startx $starty $endx $endy $width $arrowTip $arrowBase $arrowSides]

}

#############################################################################
##
## PolyArrow
##
## This procedure is used to draw arrows using polygons.
## Arrows have to be polygons since arrowshape does not have a border
## and borders are needed for delta-remoteness
##
## Assumes all points are in floating-point coordinates 
## (to allow arithmetic operations)
##
## Arrow starts at (startx, starty) and points to (endx, endy)
##
## Arrows point in 8 cardinal directions with 45o angles between them
## (i.e., the absolute value of the slopes for the dagonal arrows are 1)       
##
## arrowWidth == -width w
## {arrowTip arrowBase arrowSides} == -arrowShape {x y z}
##
## $path create line x1 y1 ... xn yn ?option value ...
##  -arrow
##  -arrowshape
##  -tags
##
## $path create polygon x1 y1 ... xn yn ?option value ...
##  -outline color
##  -width outlineWidth
##  -tags
##
## Tk coordinates: 
## "Larger y-coordinates refer to points lower on the screen;
##  larger x-coordinates refer to points farther to the right"
##
## (0,0) -> (n,0)
##   |
##   v
## (0, n)
##
## Tk does not allow rotations on canvas objects, therefore we need 8 helpers
#############################################################################

proc PolyArrow {canvas startx starty endx endy arrowWidth arrowTip arrowBase arrowSides} {

    global pi
    set pi 3.14159265359
    #3.1415926535897932384626433832795028841971693993751058209749445923078164 

    if {($startx < $endx) && ($starty == $endy)} {
	return [EPolyArrow $canvas $startx $starty $endx $endy $arrowWidth $arrowTip $arrowBase $arrowSides]
    } elseif {($startx > $endx) && ($starty == $endy)} {
	return [WPolyArrow $canvas $startx $starty $endx $endy $arrowWidth $arrowTip $arrowBase $arrowSides]
    } elseif {($startx == $endx) && ($starty < $endy)} {
	return [SPolyArrow $canvas $startx $starty $endx $endy $arrowWidth $arrowTip $arrowBase $arrowSides]	
    } elseif {($startx == $endx) && ($starty > $endy)} {
	return [NPolyArrow $canvas $startx $starty $endx $endy $arrowWidth $arrowTip $arrowBase $arrowSides]
    } elseif {($startx < $endx) && ($starty < $endy)} {
	return [SEPolyArrow $canvas $startx $starty $endx $endy $arrowWidth $arrowTip $arrowBase $arrowSides]
    } elseif {($startx < $endx) && ($starty > $endy)} {
	return [NEPolyArrow $canvas $startx $starty $endx $endy $arrowWidth $arrowTip $arrowBase $arrowSides]
    } elseif {($startx > $endx) && ($starty < $endy)} {
	return [SWPolyArrow $canvas $startx $starty $endx $endy $arrowWidth $arrowTip $arrowBase $arrowSides]
    } elseif {($startx > $endx) && ($starty > $endy)} {
	return [NWPolyArrow $canvas $startx $starty $endx $endy $arrowWidth $arrowTip $arrowBase $arrowSides]
    } else {
	[BadElse "PolyArrow" "unknown arrow type"]
    }

}

## East Horizontal Arrow points:
##
##          3 
## 1        2  
##              4
## 7        6
##          5
proc EPolyArrow {canvas startx starty endx endy arrowWidth arrowTip arrowBase arrowSides} {

    set halfWidth [expr $arrowWidth / 2]
    
    set x1 $startx
    set y1 [expr $starty - $halfWidth]

    set x2 [expr $endx - $arrowBase]
    set y2 $y1

    set x3 $x2
    set y3 [expr $y2 - $arrowSides]

    set x4 $endx
    set y4 $endy

    set x7 $startx
    set y7 [expr $starty + $halfWidth]

    set x6 $x2
    set y6 $y7

    set x5 $x2
    set y5 [expr $y6 + $arrowSides]

    set epArrow [$canvas create polygon $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $x5 $y5 $x6 $y6 $x7 $y7]

    return $epArrow

}


## West Arrow points:
##      3 
##      2         1
##  4        
##      6         7
##      5
proc WPolyArrow {canvas startx starty endx endy arrowWidth arrowTip arrowBase arrowSides} {

    set halfWidth [expr $arrowWidth / 2]
    
    set x1 $startx
    set y1 [expr $starty - $halfWidth]

    set x7 $x1
    set y7 [expr $starty + $halfWidth]

    set x2 [expr $endx + $arrowBase]
    set y2 $y1

    set x3 $x2
    set y3 [expr $y2 - $arrowSides]

    set x4 $endx
    set y4 $endy

    set x6 $x2
    set y6 $y7

    set x5 $x2
    set y5 [expr $y6 + $arrowSides]

    set wpArrow [$canvas create polygon $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $x5 $y5 $x6 $y6 $x7 $y7]

    return $wpArrow

}


## North Arrow points:
##
##      4
##
##  3 2   6 5
##
##
##    1   7
proc NPolyArrow {canvas startx starty endx endy arrowWidth arrowTip arrowBase arrowSides} {

    set halfWidth [expr $arrowWidth / 2]
    
    set x1 [expr $startx - $halfWidth]
    set y1 $starty

    set x7 [expr $startx + $halfWidth]
    set y7 $starty

    set x2 $x1
    set y2 [expr $endy + $arrowBase]

    set x3 [expr $x2 - $arrowSides]
    set y3 $y2

    set x4 $endx
    set y4 $endy

    set x6 $x7
    set y6 $y2

    set x5 [expr $x6 + $arrowSides]
    set y5 $y2
    
    set npArrow [$canvas create polygon $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $x5 $y5 $x6 $y6 $x7 $y7]

    return $npArrow

}


## South Arrow points:
##
##    1   7
##
##          
##  3 2   6 5
##              
##      4  
proc SPolyArrow {canvas startx starty endx endy arrowWidth arrowTip arrowBase arrowSides} {

    set halfWidth [expr $arrowWidth / 2]
    
    set x1 [expr $startx - $halfWidth]
    set y1 $starty

    set x7 [expr $startx + $halfWidth]
    set y7 $starty

    set x2 $x1
    set y2 [expr $endy - $arrowBase]

    set x3 [expr $x2 - $arrowSides]
    set y3 $y2

    set x4 $endx
    set y4 $endy

    set x6 $x7
    set y6 $y2

    set x5 [expr $x6 + $arrowSides]
    set y5 $y2

    set spArrow [$canvas create polygon $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $x5 $y5 $x6 $y6 $x7 $y7]

    return $spArrow

}
    

## North-East Arrow points:
##
##     3    4
##      2
##        6 
##   1     5
##     7    
proc NEPolyArrow {canvas startx starty endx endy arrowWidth arrowTip arrowBase arrowSides} {

    global pi

    set halfWidth [expr $arrowWidth / 2]
    set length [expr hypot(abs($endx - $startx), abs($endy - $starty))]
    set tailLength [expr $length - $arrowTip]

    set angle [expr atan(abs($endy - $starty) / abs($endx - $startx))]
    set compAngle [expr ($pi / 2) - $angle]
    
    set x1 [expr $startx - ($halfWidth * cos($compAngle))]
    set y1 [expr $starty - ($halfWidth * sin($compAngle))]

    set x7 [expr $startx + ($halfWidth * cos($compAngle))]
    set y7 [expr $starty + ($halfWidth * sin($compAngle))]

    set x2 [expr $x1 + ($tailLength * cos($angle))]
    set y2 [expr $y1 - ($tailLength * sin($angle))]

    set x3 [expr $x2 - ($arrowSides * cos($compAngle))]
    set y3 [expr $y2 - ($arrowSides * sin($compAngle))]

    set x4 $endx
    set y4 $endy

    set x6 [expr $x7 + ($tailLength * cos($angle))]
    set y6 [expr $y7 - ($tailLength * sin($angle))] 

    set x5 [expr $x6 + ($arrowSides * cos($compAngle))]
    set y5 [expr $y6 + ($arrowSides * sin($compAngle))]
    
    set nepArrow [$canvas create polygon $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $x5 $y5 $x6 $y6 $x7 $y7]

    return $nepArrow

}

## North-West Arrow points:
##
## 4   3
##    2  
##   6    1   
##  5    7   
##         
proc NWPolyArrow {canvas startx starty endx endy arrowWidth arrowTip arrowBase arrowSides} {

    global pi

    set halfWidth [expr $arrowWidth / 2]
    set length [expr hypot(abs($endx - $startx), abs($endy - $starty))]
    set tailLength [expr $length - $arrowTip]

    set angle [expr atan(abs($endy - $starty) / abs($endx - $startx))]
    set compAngle [expr ($pi / 2) - $angle]
    
    set x1 [expr $startx + ($halfWidth * cos($compAngle))]
    set y1 [expr $starty - ($halfWidth * sin($compAngle))]

    set x7 [expr $startx - ($halfWidth * cos($compAngle))]
    set y7 [expr $starty + ($halfWidth * sin($compAngle))]

    set x2 [expr $x1 - ($tailLength * cos($angle))]
    set y2 [expr $y1 - ($tailLength * sin($angle))]

    set x3 [expr $x2 + ($arrowSides * cos($compAngle))]
    set y3 [expr $y2 - ($arrowSides * sin($compAngle))]

    set x4 $endx
    set y4 $endy

    set x6 [expr $x7 - ($tailLength * cos($angle))]
    set y6 [expr $y7 - ($tailLength * sin($angle))] 

    set x5 [expr $x6 - ($arrowSides * cos($compAngle))]
    set y5 [expr $y6 + ($arrowSides * sin($compAngle))]
    
    set nwpArrow [$canvas create polygon $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $x5 $y5 $x6 $y6 $x7 $y7]

    return $nwpArrow

}

## South-East Arrow points:
##
##    1    3 
##   7    2
##       6 
##      5  4
##         
proc SEPolyArrow {canvas startx starty endx endy arrowWidth arrowTip arrowBase arrowSides} {

    global pi

    set halfWidth [expr $arrowWidth / 2]
    set length [expr hypot(abs($endx - $startx), abs($endy - $starty))]
    set tailLength [expr $length - $arrowTip]

    set angle [expr atan(abs($endy - $starty) / abs($endx - $startx))]
    set compAngle [expr ($pi / 2) - $angle]
    
    set x1 [expr $startx + ($halfWidth * cos($compAngle))]
    set y1 [expr $starty - ($halfWidth * sin($compAngle))]

    set x7 [expr $startx - ($halfWidth * cos($compAngle))]
    set y7 [expr $starty + ($halfWidth * sin($compAngle))]

    set x2 [expr $x1 + ($tailLength * cos($angle))]
    set y2 [expr $y1 + ($tailLength * sin($angle))]

    set x3 [expr $x2 + ($arrowSides * cos($compAngle))]
    set y3 [expr $y2 - ($arrowSides * sin($compAngle))]

    set x4 $endx
    set y4 $endy

    set x6 [expr $x7 + ($tailLength * cos($angle))]
    set y6 [expr $y7 + ($tailLength * sin($angle))] 

    set x5 [expr $x6 - ($arrowSides * cos($compAngle))]
    set y5 [expr $y6 + ($arrowSides * sin($compAngle))]

    set sepArrow [$canvas create polygon $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $x5 $y5 $x6 $y6 $x7 $y7]

    return $sepArrow

}

## South-West Arrow points:
##
##        1  
##   3      7
##    2      
##     6    
##  4   5     
proc SWPolyArrow {canvas startx starty endx endy arrowWidth arrowTip arrowBase arrowSides} {

    global pi

    set halfWidth [expr $arrowWidth / 2]
    set length [expr hypot(abs($endx - $startx), abs($endy - $starty))]
    set tailLength [expr $length - $arrowTip]

    set angle [expr atan(abs($endy - $starty) / abs($endx - $startx))]
    set compAngle [expr ($pi / 2) - $angle]
    
    set x1 [expr $startx - ($halfWidth * cos($compAngle))]
    set y1 [expr $starty - ($halfWidth * sin($compAngle))]

    set x7 [expr $startx + ($halfWidth * cos($compAngle))]
    set y7 [expr $starty + ($halfWidth * sin($compAngle))]

    set x2 [expr $x1 - ($tailLength * cos($angle))]
    set y2 [expr $y1 + ($tailLength * sin($angle))]

    set x3 [expr $x2 - ($arrowSides * cos($compAngle))]
    set y3 [expr $y2 - ($arrowSides * sin($compAngle))]

    set x4 $endx
    set y4 $endy

    set x6 [expr $x7 - ($tailLength * cos($compAngle))]
    set y6 [expr $y7 + ($tailLength * sin($compAngle))]

    set x5 [expr $x6 + ($arrowSides * cos($angle))]
    set y5 [expr $y6 + ($arrowSides * sin($angle))]


    set swpArrow [$canvas create polygon $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $x5 $y5 $x6 $y6 $x7 $y7]

    return $swpArrow

}

#############################################################################
##
## InitGlobals
##
## This procedure is used to reserve some keywords. Constants begin with
## 'k' to remember they're constants to make the code easier to read. 
## Constants never change. Nice to know somethings don't change around here.
##
#############################################################################

proc InitGlobals {} {

    ### These are used in the routine that draws the pieces - we want a small
    ### piece used as a label but a larger piece used in the playing board.

    global kBigPiece
    set kBigPiece 1
    
    global kSmallPiece
    set kSmallPiece 0

    ### Set the color and font of the labels (moved to InitWindow.tcl)

    global kLabelColor
    set kLabelColor grey40

    ### Set the animation speed.  determined exponentially, so middle value is
    ### 0.

    global gAnimationSpeed
    set gAnimationSpeed 0
    
    global gMoveType
    set gMoveType all
    
    global gMoveRadioStation
    set gMoveRadioStation all

    global gGameSolved 
    set gGameSolved false

    ### Smarter Computer

    global gSmartness gSmartnessScale
    set gSmartness Perfectly
    set gSmartnessScale 100

    ### ToMove & ToWin
    global gToMove gToWin
    set gToMove "To Move:"
    set gToWin "To Win:"

    ### Automove when only 1 possible move
    global gSkipInputOnSingleMove
    global gJustUndone
    set gSkipInputOnSingleMove false
    set gJustUndone false
}


proc SetupGamePieces {} {

    global gLeftPiece gRightPiece gLeftHumanOrComputer gRightHumanOrComputer
    
    global gLeftName gRightName
    global gPiecesPlayersName
    set alist [GS_NameOfPieces]
    
    set gLeftPiece [lindex $alist 0]
    set gRightPiece [lindex $alist 1]
    set gLeftHumanOrComputer Human
    set gRightHumanOrComputer Computer

    if {[catch {set fileptr [open playername.txt r]}]} {
	set gLeftName Player
	set gRightName Hal9000
    } else {
	gets $fileptr gLeftName
	gets $fileptr gRightName
	close $fileptr
    }
    
    set gPiecesPlayersName($gLeftPiece) $gLeftName
    set gPiecesPlayersName($gRightPiece) $gRightName

    global gLeftColor gRightColor
    set alist [GS_ColorOfPlayers]
    set gLeftColor [lindex $alist 0]
    set gRightColor [lindex $alist 1]

}



#############################################################################
##
## NewGame
##
## This is what we do when the user clicks the 'New Game' button.
##
## Here we set up the globals for the new game and call the game-specific 
## function GS_NewGame to draw the initial screen
##
## Args: None
## 
## Requires: GS_Initialize has been called
##
#############################################################################

proc NewGame { } {

    global gGameSoFar gPosition gInitialPosition gMovesSoFar
    global gLeftName gRightName gWhoseTurn gPlaysFirst
    .middle.f1.cMLeft itemconfigure LeftName \
	-text [format "Left:\n%s" $gLeftName]
    .middle.f3.cMRight itemconfigure RightName \
	-text [format "Right:\n%s" $gRightName]	
    .middle.f1.cMLeft raise LeftName
    .middle.f3.cMRight raise RightName
    update
    set gPosition $gInitialPosition
    if { $gPlaysFirst == 0 } {
        set gWhoseTurn "Left"
    } else {
	set gWhoseTurn "Right"
    }
    set gGameSoFar [list $gInitialPosition]
    set gMovesSoFar [list]
    GS_NewGame .middle.f2.cMain $gPosition
    .cStatus raise undoD
    EnableMoves
    
    clearMoveHistory

    DriverLoop
}

#############################################################################
##
## DriverLoop
##
## Here is where we decide whether to get a move from the database or give
## up control to the user and allow for a human move.  
##
## Args: None
##
## Requires: New Game has been called 
##
#############################################################################

proc DriverLoop { } {
   
    ## retrieve global variables
    global gGameSoFar gMovesSoFar gPosition gWaitingForHuman
    global gMoveDelay gGameDelay gMoveType gGameSolved
    global gWhoseTurn gLeftName gRightName
    global gameMenuToDriverLoop

    if { !$gGameSolved } {
	return
    }

    set gWaitingForHuman false

    while { [expr !$gWaitingForHuman] } {

	set primitive [C_Primitive $gPosition]

	if {!$gameMenuToDriverLoop} {
	    ## Move History
	    if { $primitive != "Undecided" } {
		set theMoves  [list]
	    } else {
		set theMoves  [C_GetValueMoves $gPosition]
	    }
	    set lastMove      [peek $gMovesSoFar]
	    set theValue      [C_GetValueOfPosition $gPosition]
	    set theRemoteness [C_Remoteness $gPosition]
	    
	    plotMove $gWhoseTurn $theValue $theRemoteness $theMoves $lastMove
	    update idletasks
	    ##
	}
	set gameMenuToDriverLoop false

	## Game's over if the position is primitive
	if { $primitive != "Undecided" } {
	
	    set gWaitingForHuman true
	    GameOver $gPosition $primitive [peek $gMovesSoFar]
	    
	} else {

	    .middle.f3.cMRight itemconfigure WhoseTurn \
		-text [format "It's %s's Turn" [subst $[subst g[subst $gWhoseTurn]Name]]]
		update idletasks

	    ## Handle Prediction
	    global gPredString
	    GetPredictions
	    .middle.f3.cMRight itemconfigure Predictions \
		-text [format "Predictions: %s" $gPredString]
	    update idletasks
       	    
	    if { [PlayerIsComputer] } {
		GS_ShowMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]
		after [expr int($gMoveDelay * 1000)]
		GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]
		DoComputerMove
		set gWaitingForHuman false
		update
	    } else {
		global gSkipInputOnSingleMove gJustUndone
		if {$gSkipInputOnSingleMove && !$gJustUndone && [llength [C_GetValueMoves $gPosition]] == 1} {
		    SwitchWhoseTurn
		    DoComputerMove
		    SwitchWhoseTurn
		} else {
		    DoHumanMove
		    set gWaitingForHuman true
		}
		set gJustUndone false

	    }
	}
    }
}

#############################################################################
##
## SwitchWhoseTurn
##
## Switches from the left player's turn to the right player's and vice-versa
## 
## Args: none
##
## Requires: NewGame has been called
##
#############################################################################

proc SwitchWhoseTurn {} {
    global gWhoseTurn

    if { $gWhoseTurn == "Left" } {
	set gWhoseTurn "Right"
    } else {
	set gWhoseTurn "Left"
    }
}
    
#############################################################################
##
## DoComputerMove
##
## This function gets the computer's move from the database and makes it
## 
## Args: none
##
## Requires: The state of the board is such that there are no moves being
##           shown at this time
##
#############################################################################

proc DoComputerMove { } {

    global gPosition gGameSoFar gMovesSoFar

    set theMove [C_GetComputersMove $gPosition]    

    set oldPosition $gPosition

    set gPosition [C_DoMove $gPosition $theMove]

    set gGameSoFar [push $gGameSoFar $gPosition]

    set gMovesSoFar [push $gMovesSoFar $theMove]

    HandleComputersMove .middle.f2.cMain $oldPosition $theMove $gPosition

    .cStatus raise undoI

    if { [expr ![C_GoAgain $oldPosition $theMove]] } {
	SwitchWhoseTurn
    }

}

#############################################################################
##
## HandleComputersMove
##
## This function handles the  move for a computer player
## This should be overwritten for modules which support n-phase moves,
## with n > 1
##
#############################################################################

proc HandleComputersMove { c oldPos theMove Position } {

  GS_HandleMove $c $oldPos $theMove $Position

}


#############################################################################
##
## DoHumanMove
##
## This function shows all the moves and relinquishes control so that the
## user can input the move
## 
## Args: none
##
## Requires: The state of the board is such that there are no moves being
##           shown at this time
##
#############################################################################

proc DoHumanMove { } {

    global gPosition gMoveType

    GS_ShowMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]

}

#############################################################################
##
## ReturnFromHumanMove
##
## This function is called from the Game Specific Functions and gives
## control back to gamesman
## 
## Args: the Move
##
## Requires: The Moves on the Game Board are still being shown
##
#############################################################################

proc ReturnFromHumanMove { theMove } {
    global gGamePlayable
    if {$gGamePlayable && ![PlayerIsComputer]} {
        ReturnFromHumanMoveHelper $theMove
    }
}

proc ReturnFromHumanMoveHelper { theMove } {
        
    global gPosition gGameSoFar gMovesSoFar gMoveType

    set primitive [C_Primitive $gPosition]

    set PositionValueList [C_GetValueMoves $gPosition]

    if { $primitive == "Undecided" &&
         [containskey $theMove $PositionValueList] } {
        
        GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]
        
        set oldPosition $gPosition
                
        set gPosition [C_DoMove $gPosition $theMove]
                
        set gGameSoFar [push $gGameSoFar $gPosition]
        
        set gMovesSoFar [push $gMovesSoFar $theMove]
        
	.cStatus raise undoI
        
        GS_HandleMove .middle.f2.cMain $oldPosition $theMove $gPosition

	if { [expr ![C_GoAgain $oldPosition $theMove]] } {
	    SwitchWhoseTurn
	}

        DriverLoop

    }

}


#############################################################################
##
## GameOver
##
## This function is called from the driver loop to signify a game is over.
## 
## Args: the position, the Game Value (win, lose or tie)
##
## Requires: The Moves on the Game Board are not shown
##
#############################################################################

proc GameOver { position gameValue lastMove } {

    global gPosition gGameSoFar gWhoseTurn gLeftName gRightName
    global gLeftPiece gRightPiece
    global gGameDelay gLeftHumanOrComputer gRightHumanOrComputer

    set previousPos [peek [pop $gGameSoFar]]

    set WhoWon Nobody

    set WhichPieceWon Nobody

    if { $gWhoseTurn == "Right" } {

        if { $gameValue == "Win" } {
            
            set WhoWon $gRightName
	    set WhoLost $gLeftName
            set WhichPieceWon $gRightPiece

        } elseif { $gameValue == "Lose" } {

            set WhoWon $gLeftName
	    set WhoLost $gRightName
            set WhichPieceWon $gLeftPiece

        }

    } else {
        
        if { $gameValue == "Win" } {
            
            set WhoWon $gLeftName
	    set WhoLost $gRightName
            set WhichPieceWon $gLeftPiece

        } elseif { $gameValue == "Lose" } {

            set WhoWon $gRightName
	    set WhoLost $gLeftName
            set WhichPieceWon $gRightPiece

        }
    }

    if { $gameValue == "Tie" } {
        set message [concat GAME OVER: It's a TIE!]
        SendMessage $message
	set loseMessage "Nobody wins!"
    } else {
        set message [concat GAME OVER: $WhoWon wins!]
	SendMessage $message
	set loseMessage [format "%s loses!" $WhoLost]
    }

    .middle.f3.cMRight itemconfigure WhoseTurn \
	-text [format "%s" $message]
    
    .middle.f3.cMRight itemconfigure Predictions \
	-text [format "%s" $loseMessage] 
    update idletasks

    GS_GameOver .middle.f2.cMain $gPosition $gameValue $WhichPieceWon $WhoWon $lastMove
    update idletasks

    DisableMoves

    if { $gLeftHumanOrComputer == "Computer" && $gRightHumanOrComputer == "Computer" } {
	after [expr int($gGameDelay * 1000)]
	TBaction1
    }
}


#############################################################################
##
## DisableMoves/EnableMoves
##
## Disables or Enables the radiobuttons which show All/Value/Best Moves
## 
## Args: None
##
## Requires: Nothing
##
#############################################################################

proc DisableMoves {} {

#   .winPlayOptions.fRadio.butAllMoves configure -state disabled
#   .winPlayOptions.fRadio.butValueMoves configure -state disabled
#   .winPlayOptions.fRadio.butBestMoves configure -state disabled

}

proc EnableMoves {} {

#    .winPlayOptions.fRadio.butAllMoves configure -state normal
#    .winPlayOptions.fRadio.butValueMoves configure -state normal
#    .winPlayOptions.fRadio.butBestMoves configure -state normal

}

#############################################################################
##
## ToggleMoves
##
## Called When Changing Between All/Value/Best Moves
##
## Args:  A string corresponding to all, value, or best
##
## Requires: Moves are currently shown and Game is not over
##
#############################################################################

proc ToggleMoves { moveType } {

    global gMoveType gPosition

    ChangeMoveType $gMoveType $moveType $gPosition [C_GetValueMoves $gPosition]

    set gMoveType $moveType

}

#############################################################################
##
## ChangeMoveType
##
## Called when Changing Between All/Value/Best Moves
## 
## Args: 
##
## Requires: Moves are currently shown
##
#############################################################################

proc ChangeMoveType { fromMoveType toMoveType position moveList } {

    global gWaitingForHuman

    GS_HideMoves .middle.f2.cMain $fromMoveType $position $moveList

    GS_ShowMoves .middle.f2.cMain $toMoveType $position $moveList

}

#############################################################################
##
## Undo
##
## Calls Game Specific Undo to undo the last move
## 
## Args: none
##
## Requires: Moves are currently shown
##
#############################################################################

proc Undo { } {
    
    UndoHelper
    global gWhoseTurn gLeftName gRightName
    .middle.f3.cMRight itemconfigure WhoseTurn \
	-text [format "It's %s's Turn" [subst $[subst g[subst $gWhoseTurn]Name]]]
    GetPredictions
    global gPredString
    .middle.f3.cMRight itemconfigure Predictions \
	    -text [format "Predictions: %s" $gPredString] 

    unplotMove 1

    update idletasks
    global gJustUndone
    set gJustUndone true

    DriverLoop
}

proc UndoHelper { } {
    
    global gPosition gMovesSoFar gGameSoFar gMoveType
    
    if { [llength $gGameSoFar] != 1 } {
        
        set primitive [C_Primitive $gPosition]
        
        if { $primitive == "Undecided" } {
            
            GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition]
            
        } else {

            GS_UndoGameOver .middle.f2.cMain $gPosition

        }
        
        set undoOnce [pop $gGameSoFar]
        
        GS_HandleUndo .middle.f2.cMain [peek $gGameSoFar] [peek $gMovesSoFar] [peek $undoOnce]
        
        set gGameSoFar $undoOnce
        
        set gPosition [peek $gGameSoFar]
        
	set undoneMove [peek $gMovesSoFar]
        set gMovesSoFar [pop $gMovesSoFar]

	if { 0 == [llength $gMovesSoFar]} {
	    .cStatus raise undoD
	}

	if { [expr ![C_GoAgain $gPosition $undoneMove]] } {
	    SwitchWhoseTurn
	}
        
        if { [PlayerIsComputer] } {

            unplotMove 0

            UndoHelper

	}
        
    }

}

#############################################################################
##
## PlayerIsComputer
##
## Returns true or false if the player whose turn it is currently is controlled
## by a computer
## 
## Args: Nothing
##
## Requires: Nothing
##
#############################################################################

proc PlayerIsComputer { } {

    global gWhoseTurn gRightHumanOrComputer gLeftHumanOrComputer

    if { $gWhoseTurn == "Left" } {
        return [expr { $gLeftHumanOrComputer == "Computer"}]
    } else {
        return [expr { $gRightHumanOrComputer == "Computer"}]
    }
}

proc ReturnToGameSpecificOptions {} {
    GS_Deinitialize .middle.f2.cMain
    global gGameSolved
    set gGameSolved false
}

#############################################################################
##
## SendMessage
##
## Displays whose turn it is or who's won
## 
## Args: string
##
## Requires: Nothing
##
#############################################################################

proc SendMessage { arg } {
    
}

#############################################################################
##
## GetPredictions
##
## Get predictions from the C code
## 
## Args: Nothing
##
## Requires: Nothing
##
#############################################################################

proc GetPredictions {} {

    global gPosition gPredString gWhoseTurn
    global gLeftName gRightName

    if { [C_Primitive $gPosition] != "Undecided" } {
	set gPredString ""
    } else {

	### Get the value, the player and set the prediction
	set theValue      [C_GetValueOfPosition $gPosition]
	set theRemoteness [C_Remoteness $gPosition]
	set theMex        [C_Mex $gPosition]
	
	if { $gWhoseTurn == "Left" } {
	    set playersName $gLeftName
	} else {
	    set playersName $gRightName
	}
	
	set prediction [format "%s should " $playersName]
	
	if { $theValue == "Tie" && $theRemoteness == 255 } {
	    set prediction [concat $prediction "Draw"]
	} else {
	    set prediction [concat $prediction [format "%s in %s" $theValue $theRemoteness]]
	}

	set prediction [concat $prediction " " $theMex]

	### And place it in the field if the button is on.
	set gPredString $prediction
    }
}

#############################################################################
##
## BadElse
##
## This is for if-then-elseif-then-else statements to guarantee correctness.
##
#############################################################################

proc BadElse { theFunction theMsg } {
    puts "Error: $theFunction\{\} just reached an else clause it shouldn't have: $theMsg"
}





# argv etc
proc main {kRootDir} {

    # Initialize generic top-level window
    source "$kRootDir/../tcl/InitWindow.tcl"
    InitGlobals

    global gSkinsDir
    if {[catch {set fileptr [open skin.txt r]}]} {
	set gSkinsDir OxySkin_HiRes/
    } else {
	gets $fileptr gSkinsDir
	if {![file isdirectory "$kRootDir/../tcl/skins/$gSkinsDir"]} {
	    set gSkinsDir OxySkin_HiRes/
	}
	close $fileptr
    }

    InitWindow $kRootDir ppm

    # Initialize game-specific globals and frame
    GS_InitGameSpecific

    # This line tags the background of the blank canvas before sending it away to
    # each game's Tcl code. This is because when changing rules, GS_Deinitialize is run
    # which deletes everything, including the background. This can result in a border
    # forming around the board. Hence, we should do $c delete {!background} in GS_Deinitialize
    # now, but as a temporary easier fix I'll change the line in InitWindow.tcl, too.
    .middle.f2.cMain addtag background all

    GS_Initialize .middle.f2.cMain

    # Set the window title
    global kGameName
    wm title . "$kGameName - GAMESMAN"

    # Generate game-specific About, Help, and Skins frames
    global gFrameWidth
    SetupAboutFrame .middle.f2.fAbout.content $gFrameWidth
    SetupHelpFrame .middle.f2.fHelp.content $gFrameWidth
    SetupSkinsFrame .middle.f2.fSkins.content $gFrameWidth

    # Initialize the C backend
    C_Initialize
    C_InitializeDatabases
}

