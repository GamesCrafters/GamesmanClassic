############################################################################
##
## gamesman3.tcl
##
## LAST CHANGE: $Id: gamesman3.tcl,v 1.64 2008-11-29 11:09:07 koolswim88 Exp $
##
############################################################################

############################################################################
##
## Source The Utility Functions
##
##
##
#############################################################################

global command_line_args printing
# check first argument for --printing
# if it is set, then enable printing end of game stats
if {[llength $argv] > 0 && [lindex $argv 0] == "--printing"} {
	set printing true
	set argv [lrange $argv 1 [lindex [expr [llength $argv] - 1]]]
} else {
	set printing false
}

set command_line_args [concat $argv0 $argv]


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

    ### List of moves played
    global gMovesSoFar
    set gMovesSoFar [list]
    
    ### TIER-GAMESMAN: For Undoing Tier Moves
    global gTiersSoFar
    set gTiersSoFar [list]

    ### Automove when only 1 possible move
    global gSkipInputOnSingleMove
    global gJustUndone
    set gSkipInputOnSingleMove false
    set gJustUndone false
    
    ## Make sure we can ignore DriverLoop calls when waiting for a response
	global gWaitingForResponse
	set gWaitingForResponse 0
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

    global gGameSoFar gPosition gInitialPosition gMovesSoFar gTiersSoFar gComputerIsOnlinePlayer
    global gLeftName gRightName gWhoseTurn gPlaysFirst gUsingTiers
    .middle.f1.cMLeft itemconfigure LeftName \
	-text [format "Left:\n%s" $gLeftName]
    .middle.f3.cMRight itemconfigure RightName \
	-text [format "Right:\n%s" $gRightName]	
    .middle.f1.cMLeft raise LeftName
    .middle.f3.cMRight raise RightName
    update
    set gPosition $gInitialPosition
    # TIER-GAMESMAN
    if { $gUsingTiers == 1 } {
        set gPosition [C_InitHashWindow $gInitialPosition]
        set gTiersSoFar [list]
    }
    if { $gPlaysFirst == 0 } {
        set gWhoseTurn "Left"
    } else {
	set gWhoseTurn "Right"
    }
    set gGameSoFar [list $gInitialPosition]
    set gMovesSoFar [list]
    GS_NewGame .middle.f2.cMain $gPosition
    .cStatus raise undoD
    .cToolbar raise iDTB3
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
    global gMoveDelay gGameDelay gMoveType gGameSolved gReallyUnsolved gDeltaRemote
    global gWhoseTurn gLeftName gRightName
    global gameMenuToDriverLoop 
    global gWaitingForResponse
    
    global printing
    if { [expr !$gGameSolved] || $gWaitingForResponse == 1 } {
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
          set theMoves  [C_GetValueMoves $gPosition $gReallyUnsolved]
		    }
		    set lastMove      [peek $gMovesSoFar]
		    if {$gReallyUnsolved} {
          set theValue "lose"
          set theRemoteness 0
		    } else {
          set theValue      [C_GetValueOfPosition $gPosition]
          set theRemoteness [C_Remoteness $gPosition]
		    }
		    # capture position before move
		    # only do if printing is enabled
		    if { $printing == true } {
          doCapture .middle.f2.cMain $gMoveType $gPosition $theMoves true
          doCapture .middle.f2.cMain $gMoveType $gPosition $theMoves false
		    }
	
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
			global kGameName
      .middle.f3.cMRight itemconfigure WhoseTurn \
        -text [format "It's %s's Turn\n" [subst $[subst g[subst $gWhoseTurn]Name]]]
			update idletasks
	
      ## Handle Prediction
      global gPredString
      GetPredictions
      .middle.f3.cMRight itemconfigure Predictions \
        -text [format "Predictions: %s" $gPredString]
      update idletasks
	              
      if { [PlayerIsComputer] } {
	    if { $gDeltaRemote } {
		  set $gMoveType "rm"
		}
		
        GS_ShowMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition $gReallyUnsolved]
        after [expr int($gMoveDelay * 1000)]
        GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition $gReallyUnsolved]
        if { ![DoComputerMove] } {
          ## Encountered an error
          EndGame
          return
        }
        set gWaitingForHuman false
        update
      } else {
        global gSkipInputOnSingleMove gJustUndone
        if {$gSkipInputOnSingleMove && !$gJustUndone && [llength [C_GetValueMoves $gPosition $gReallyUnsolved]] == 1} {
            SwitchWhoseTurn
          if { ![DoComputerMove] } {
            ## Encountered an error
            EndGame
            return
          }
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

    global gPosition 
    global gGameSoFar 
    global gMovesSoFar 
    global gUsingTiers 
    global gTiersSoFar 
    global gRightHumanOrComputer 
    global gWhoseTurn
    global gGameInfo
    global gWaitingForResponse
	
	set onlinePlayer [PlayerIsOnline]
	if { !$onlinePlayer } {
	    set theMove [C_GetComputersMove $gPosition]    
	} elseif { $gGameInfo != "" } {
		if { [llength $gMovesSoFar] > 0 } {
			## Send the local player's last move
			set prevMove [peek $gMovesSoFar]
			if { ![SendLocalPlayersMove $gPosition $prevMove] } {
				## Exit play
				return 0
			}
		}

		## Get the remote players counter move
		set gWaitingForResponse 1
		set theMove [GetRemotePlayersMove $gPosition]
		set gWaitingForResponse 0
		if { $theMove == "" } {
			## Exit play
			return 0
		}
	}
    set oldPosition $gPosition

    set gPosition [C_DoMove $gPosition $theMove]

    set gGameSoFar [push $gGameSoFar $gPosition]

    set gMovesSoFar [push $gMovesSoFar $theMove]

    HandleComputersMove .middle.f2.cMain $oldPosition $theMove $gPosition

	if { !$onlinePlayer } {
	    .cStatus raise undoI
	    .cToolbar raise iITB3
	}

    if { [expr ![C_GoAgain $oldPosition $theMove]] } {
	SwitchWhoseTurn
    }
    
    # TIER-GAMESMAN
    if { $gUsingTiers == 1 } {
        set gTiersSoFar [push $gTiersSoFar [C_CurrentTier]]
        set gPosition [C_HashWindow $gPosition]
        set gGameSoFar [pop $gGameSoFar]
        set gGameSoFar [push $gGameSoFar $gPosition]
    }
    
    return 1
}

proc SendGameOver { } {
	global gUsername
	global gPassword
	global gSessionId
	global gGameInfo

    set result [C_SendGameOver $gUsername $gPassword $gSessionId [lindex $gGameInfo 0]]    
	scan $result "%d:%n" errCode num
	if { $errCode == 0 } {
		#Return true
		return 1
	} else {
		## Show the error to the user
		DisplayRuntimeOKModal "Error: [string range $result $num end]" "OK"
		## Return false
		return 0	
	}
}

proc SendResignGame { } {
	global gUsername
	global gPassword
	global gSessionId
	global gGameInfo

    set result [C_SendResign $gUsername $gPassword $gSessionId [lindex $gGameInfo 0]]    
	scan $result "%d:%n" errCode num
	if { $errCode == 0 } {
		#Return true
		return 1
	} else {
		## Return false
		return 0	
	}
}

proc SendLocalPlayersMove { position prevMove } {
	global gUsername
	global gPassword
	global gSessionId
	global gGameInfo

    set result [C_SendLocalPlayersMove $gUsername $gPassword $gSessionId [lindex $gGameInfo 0] $position $prevMove]    
	scan $result "%d:%n" errCode num
	if { $errCode == 0 } {
		#Return true
		return 1
	} else {	
		## Show the error to the user
		DisplayRuntimeOKModal "Error: [string range $result $num end]" "OK"
		## Return false
		return 0	
	}
}

proc GetRemotePlayersMove { position } {
	global gUsername
	global gPassword
	global gSessionId
	global gGameInfo
	global gGetRemotePlayersMoveRefreshPeriod
	global gGetRemotePlayersMoveMaxTries

	for {set x 0} {$x < $gGetRemotePlayersMoveMaxTries} {incr x} {
	    set result [C_GetRemotePlayersMove $gUsername $gPassword $gSessionId [lindex $gGameInfo 0] $position]
		scan $result "%d:%n" errCode num
		if { $errCode == 0 } {
			if { [string range $result $num end] != "" } {
				## Got the move
				return [scan [string range $result $num end] "%d"]
			} else {
				## No new move yet. Wait for a moment, let the event's process then try again
				update
				after $gGetRemotePlayersMoveRefreshPeriod set timesUp 1
				vwait timesUp	
			}
		} else {
			## Show the error to the user
			DisplayRuntimeOKModal "Error: [string range $result $num end]" "OK"
			## Return nothing (signals an error)
			return ""
		}
	
		## Provide a facility to continue waiting
		if {[expr $x + 1] >= $gGetRemotePlayersMoveMaxTries } {	
			## Show the error to the user
			set dec [DisplayRuntimeYesNoModal "Exceeded maximum retries for getting remote player's last move. Do you wish to continue (wait some more)?" "Wait a bit more" "End it"]
			if { $dec } {
				set x -1
			}
		}
	}
	
	## Return nothing (signals an error)
	return ""
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

    global gPosition gMoveType gReallyUnsolved gDeltaRemote
	
    if {$gDeltaRemote} {
	   set gMoveType "rm"
	}
	
    GS_ShowMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition $gReallyUnsolved]

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
    global gGamePlayable gRedoList
    .cStatus raise redoD
    set gRedoList [list]
    if {$gGamePlayable && ![PlayerIsComputer]} {
        ReturnFromHumanMoveHelper $theMove
    }
}

proc ReturnFromHumanMoveHelper { theMove } {
        
    global gPosition gGameSoFar gMovesSoFar gMoveType gReallyUnsolved gUsingTiers gTiersSoFar

    set primitive [C_Primitive $gPosition]

    set PositionValueList [C_GetValueMoves $gPosition $gReallyUnsolved]

    if { $primitive == "Undecided" &&
         [containskey $theMove $PositionValueList] } {
        
        GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition $gReallyUnsolved]
        
        set oldPosition $gPosition
                
        set gPosition [C_DoMove $gPosition $theMove]
                
        set gGameSoFar [push $gGameSoFar $gPosition]
        
        set gMovesSoFar [push $gMovesSoFar $theMove]
    
    	if { ![OpponentIsOnline] } {
			.cStatus raise undoI
			.cToolbar raise iITB3
		}
        
        GS_HandleMove .middle.f2.cMain $oldPosition $theMove $gPosition

		if { [expr ![C_GoAgain $oldPosition $theMove]] } {
		    SwitchWhoseTurn
		}
    
        # TIER-GAMESMAN
        if { $gUsingTiers == 1 } {
            set gTiersSoFar [push $gTiersSoFar [C_CurrentTier]]
            set gPosition [C_HashWindow $gPosition]
            set gGameSoFar [pop $gGameSoFar]
            set gGameSoFar [push $gGameSoFar $gPosition]
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
	global printing
	global gGameInfo

	if { $gGameInfo == "" && [expr [PlayerIsOnline] || [OpponentIsOnline]]} {
		return
	}

	if { [PlayerIsOnline] } {
		## Send the local player's final move
		if { ![SendLocalPlayersMove $position $lastMove] } {
			EndGame
			return
		}
		## Clear the global gGameInfo
		set gGameInfo ""
	} elseif { [OpponentIsOnline] } {
		## Send a game over notice
		if { ![SendGameOver] } {
			EndGame
			return
		}
		## Clear the global gGameInfo
		set gGameInfo ""
	}

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

	if { $printing == true } {
		doPrinting .middle.f2.cMain $gPosition $WhichPieceWon
	}
	
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
## EndGame
##
## This function is called from the driver loop to end a game due to 
## online game play error.
## 
## Args: none
##
## Requires: none
##
#############################################################################

proc EndGame { } {

    global gPosition gGameSoFar gWhoseTurn gLeftName gRightName
    global gLeftPiece gRightPiece
    global gGameDelay gLeftHumanOrComputer gRightHumanOrComputer
	global printing
	global gGameInfo

	if { $gGameInfo == "" && [expr [PlayerIsOnline] || [OpponentIsOnline]]} {
		return
	}
	
	if { [PlayerIsOnline] || [OpponentIsOnline] } {
		## Send a resign game notice and ignore any errors
		SendResignGame
		## Clear the global gGameInfo
		set gGameInfo ""
	}

    set message [concat GAME TERMINATED]
    SendMessage $message
	set loseMessage "Nobody wins!"

    .middle.f3.cMRight itemconfigure WhoseTurn \
	-text [format "%s" $message]
    
    .middle.f3.cMRight itemconfigure Predictions \
	-text [format "%s" $loseMessage] 
    update idletasks

    DisableMoves

	#after [expr int($gGameDelay * 1000)] TBaction1
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

    global gMoveType gPosition gReallyUnsolved

    ChangeMoveType $gMoveType $moveType $gPosition [C_GetValueMoves $gPosition $gReallyUnsolved]

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

    global gWaitingForHuman gDeltaRemote 
	
	if {$gDeltaRemote && [string equal $toMoveType "value"] } {
	   set toMoveType "rm"
	}

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
    # delete the printing canvas

    .printing delete PDF
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

proc UndoNMoves { n } {
    global gRightHumanOrComputer gLeftHumanOrComputer gWhoseTurn
    set tempLeft $gLeftHumanOrComputer
    set tempRight $gRightHumanOrComputer
    set gLeftHumanOrComputer "Human"
    set gRightHumanOrComputer "Human"
    for {set i 0} {$i<$n} {incr i} {
	Undo
    }
    set gLeftHumanOrComputer $tempLeft
    set gRightHumanOrComputer $tempRight
    if { [PlayerIsComputer] } {
	unplotMove 0
	DriverLoop
    }
}

proc UndoHelper { } {
    
    global gPosition gMovesSoFar gGameSoFar gMoveType gRedoList gReallyUnsolved gUsingTiers gTiersSoFar
    
    if { [llength $gGameSoFar] != 1 } {
        
        set primitive [C_Primitive $gPosition]
        
        if { $primitive == "Undecided" } {
            
            GS_HideMoves .middle.f2.cMain $gMoveType $gPosition [C_GetValueMoves $gPosition $gReallyUnsolved]
            
        } else {

            GS_UndoGameOver .middle.f2.cMain $gPosition

        }
        
        set undoOnce [pop $gGameSoFar]
        
        # TIER-GAMESMAN
        if { $gUsingTiers == 1 } {
            set gPosition [C_HashWindowUndo [peek $gTiersSoFar]]
            set gTiersSoFar [pop $gTiersSoFar]
        }        
        
        GS_HandleUndo .middle.f2.cMain [peek $gGameSoFar] [peek $gMovesSoFar] [peek $undoOnce]
        
        set gGameSoFar $undoOnce
        
        set gPosition [peek $gGameSoFar]
        
        
	set undoneMove [peek $gMovesSoFar]
        set gMovesSoFar [pop $gMovesSoFar]

	if { 0 == [llength $gMovesSoFar]} {
	    .cStatus raise undoD
	    .cToolbar raise iDTB3
	}

	if { [expr ![C_GoAgain $gPosition $undoneMove]] } {
	    SwitchWhoseTurn
	}
        
	set gRedoList [push $gRedoList $undoneMove]
	.cStatus raise redoI

        if { [PlayerIsComputer] } {

            unplotMove 0

            UndoHelper

	}

    }

}


proc Redo { n } {
    global gRedoList gRightHumanOrComputer gLeftHumanOrComputer gWhoseTurn
    set tempLeft $gLeftHumanOrComputer
    set tempRight $gRightHumanOrComputer

    set gLeftHumanOrComputer "Human"
    set gRightHumanOrComputer "Human"
    for {set i 0} {$i < $n} {incr i} {
	if { [llength $gRedoList] > 0 } {
	    set redoMove [peek $gRedoList]
	    ReturnFromHumanMoveHelper $redoMove
	    set gRedoList [pop $gRedoList]
	    if { $gWhoseTurn == "Left" && ($tempLeft == "Computer")} {
		set i [expr $i - 1]
	    } elseif {$gWhoseTurn == "Right" && ($tempRight == "Computer")} {
		set i [expr $i - 1]
	    }
	}
    }
    set gLeftHumanOrComputer $tempLeft
    set gRightHumanOrComputer $tempRight

}

#############################################################################
##
## PlayerIsOnline
##
## Returns true or false if the player whose turn it is a remote online player
## 
## Args: Nothing
##
## Requires: Nothing
##
#############################################################################

proc PlayerIsOnline { } {

    global gWhoseTurn gComputerIsOnlinePlayer

    if { $gWhoseTurn == "Left" } {
        return 0
    } else {
        return $gComputerIsOnlinePlayer
    }
}

#############################################################################
##
## OpponentIsOnline
##
## Returns true or false if the player whose turn it is not (i.e. the 
## opponent) is a remote online player
## 
## Args: Nothing
##
## Requires: Nothing
##
#############################################################################

proc OpponentIsOnline { } {

    global gWhoseTurn gRightHumanOrComputer gLeftHumanOrComputer gComputerIsOnlinePlayer

    if { $gWhoseTurn == "Left" } {
        return [expr { $gLeftHumanOrComputer == "Human" && $gRightHumanOrComputer == "Computer" && $gComputerIsOnlinePlayer }]
    } else {
        return 0
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

    global gPosition gPredString gWhoseTurn gReallyUnsolved
    global gLeftName gRightName

    if { [C_Primitive $gPosition] != "Undecided" } {
	set gPredString ""
    } else {

	if {$gReallyUnsolved} {
	    ### Get the value, the player and set the prediction
	    set theValue      "Lose"
	    set theRemoteness 0
	    set theMex        0
	} else {
	    ### Get the value, the player and set the prediction
	    set theValue      [C_GetValueOfPosition $gPosition]
	    set theRemoteness [C_Remoteness $gPosition]
	    set theMex        [C_Mex $gPosition]
	}
	
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


#########################################################
## Reusable GUI functions for in-game-play windowing
#########################################################

proc DisplayRuntimeStatusModal { msgText } {
	global kLabelFont

	set w [expr [winfo width .middle.f2.cMain] * 0.8]
	set h [expr [winfo height .middle.f2.cMain] * 0.8]
	frame .middle.f2.cMain.fModal -width $w -height $h -relief raised -bd 2
	pack propagate .middle.f2.cMain.fModal 0
		
	message .middle.f2.cMain.fModal.lMessage -text $msgText -font $kLabelFont -width $w -padx 5 -pady 5
	place .middle.f2.cMain.fModal -x [expr $w / 8] -y [expr $h / 8] -in .middle.f2.cMain
	pack .middle.f2.cMain.fModal.lMessage -side top -fill both
	update
	focus .middle.f2.cMain.fModal 
}

proc DisplayRuntimeOKModal { msgText okText } {
	global kLabelFont

	set w [expr [winfo width .middle.f2.cMain] * 0.8]
	set h [expr [winfo height .middle.f2.cMain] * 0.8]
	frame .middle.f2.cMain.fModal -width $w -height $h -relief raised -bd 2
	pack propagate .middle.f2.cMain.fModal 0

	message .middle.f2.cMain.fModal.lMessage -text $msgText -width $w -font $kLabelFont -padx 5 -pady 5
	button .middle.f2.cMain.fModal.bMessage -text $okText -width 10 -command { DestroyRuntimeModal }
	place .middle.f2.cMain.fModal -x [expr $w / 8] -y [expr $h / 8] -in .middle.f2.cMain
	pack .middle.f2.cMain.fModal.lMessage -side top -fill both
	pack .middle.f2.cMain.fModal.bMessage -side top
	update
	focus .middle.f2.cMain.fModal
	tkwait window .middle.f2.cMain.fModal
}

proc DisplayRuntimeYesNoModal { msgText yesText noText } {
	global kLabelFont
	global tmp
	
	set w [expr [winfo width .middle.f2.cMain] * 0.8]
	set h [expr [winfo height .middle.f2.cMain] * 0.8]
	frame .middle.f2.cMain.fModal -width $w -height $h -relief raised -bd 2
	pack propagate .middle.f2.cMain.fModal 0

	frame .middle.f2.cMain.fModal.fButtons
	message .middle.f2.cMain.fModal.lMessage -text $msgText -width $w -font $kLabelFont -padx 5 -pady 5
	button .middle.f2.cMain.fModal.fButtons.bYes -text $yesText -command { DestroyRuntimeModal; global tmp; set tmp 1; }
	button .middle.f2.cMain.fModal.fButtons.bNo -text $noText -command { DestroyRuntimeModal; global tmp; set tmp 0; }
	place .middle.f2.cMain.fModal -x [expr $w / 8] -y [expr $h / 8] -in .middle.f2.cMain
	pack .middle.f2.cMain.fModal.fButtons.bYes -side left
	pack .middle.f2.cMain.fModal.fButtons.bNo -side left
	pack .middle.f2.cMain.fModal.lMessage -side top -fill both
	pack .middle.f2.cMain.fModal.fButtons -side top
	update
	focus .middle.f2.cMain.fModal
	tkwait window .middle.f2.cMain.fModal
	return $tmp
}

proc DestroyRuntimeModal { } {
	place forget .middle.f2.cMain.fModal
	destroy .middle.f2.cMain.fModal
}


#commandline handler
#all commandline options starting with tcl are considered tcl options and removed
#from command_line_args that gets passed to c
#in the future this may result in some comical event where some student tries to 
#add a commandline arg with "tcl" in it and can't figure out why it doesn't work
#hopefully i won't be around then..
# - Yiding
proc CmdlineHandler {} {
  #handles all tcl related commands and returns a list with them removed
  global command_line_args argv0 argv
  global fixed_window_size printing
  set printing false
  
  set command_line_args $argv0
  
  set i 0
  
  while {$i < [llength $argv]} {
    set curarg [lindex $argv $i]
    if {[string range $curarg 0 4] == "--tcl"} {
      #this arg is for tcl
      switch [string range $curarg 5 [string length $curarg]] {
        "geom" {
          set i [expr {$i + 1}]
          set fixed_window_size [lindex $argv $i]
        }
        "printing" {
          set printing true
        }
      }
    } elseif { $curarg == "--printing" }  {
      #this used to be hacked in at the top fo this file
      puts "tcl: --printing is deprecated. Please use --tclprinting in the future"
      set printing true
    } elseif { $curarg == "--help" } {
      #this is a special argument that is both passed to C AND acted upon by tcl
      puts "Tcl GUI Args:"
      puts "  --tclgeom <geom>\tForce a specific size for the window."
      puts "\t<geom> is in the format of WxH, i.e. --tclgeom 800x600"
      #pass it on
      set command_line_args [concat $command_line_args $curarg]
    } else {
      #this is not a tcl arg so we give it to C
      #note that args with quotes in them are handled in correctly
      #i.e. --foo "bar" will turn into --foo bar
      #this is just how tcl handles commandline arguments and how it gets listified
      #hopefully this won't cause sleepless nights for future people
      set command_line_args [concat $command_line_args $curarg]
    }
    set i [expr {$i + 1}]
  }
}


# argv etc
proc main {kRootDir} {

  #handle tcl command lines, this must be called or expect death.
  CmdlineHandler

  # Initialize generic top-level window
  source "$kRootDir/../tcl/InitWindow.tcl"
  InitGlobals

	# source the printing files
	source "$kRootDir/../tcl/printing.tcl"

  # source the network code
  source "$kRootDir/../tcl/network.tcl"
    
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
    global kGameName kScriptName
    set kScriptName [info script]
    wm title . "$kGameName - GAMESMAN"

    # Generate game-specific About, Help, and Skins frames
    global gFrameWidth
    SetupAboutFrame .middle.f2.fAbout.content $gFrameWidth
    SetupHelpFrame .middle.f2.fHelp.content $gFrameWidth
    SetupSkinsFrame .middle.f2.fSkins.content $gFrameWidth

    # Initialize the C backend
    global command_line_args
    C_Initialize $command_line_args
    C_InitializeDatabases
}

