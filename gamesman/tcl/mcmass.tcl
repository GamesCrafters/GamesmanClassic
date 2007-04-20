########################################################################
##
## NAME:         mcmass.tcl
##
## DESCRIPTION:  The source code for the Tcl component of Critical Mass
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

proc GS_InitGameSpecific {} {
    
    ### Set the name of the game

    global kGameName
    set kGameName "Critical Mass"

    ### Set the size of the slot in the window
    ### This should be a multiple of 80 (Why's that?)
    
    global gSlotSize
    set gSlotSize 160
    
    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "Occupation WINS"
    set kMisereString "Occupation LOSES"

    ### Set the strings to tell the user how to move and what the goal is.

    global kToMove kToWinStandard kToWinMisere
    set kToMove "Players alternate turns placing pieces into any box that does not contain the opponent's color. If the box fills up, then it will explode and all the pieces, except for one,  are distributed to the surrounding boxes. If one of the surrounding boxes is occupied by the opponent, it is captured and taken over by your color. Multiple explosions may occur if the previous explosion caused other spaces to reach critical mass. Select an empty square to place a piece or  to add a square of your color."
    set kToWinStandard  "Be the first player to occupy the board with your color."
    set kToWinMisere  "Force your opponent to capture every square."
    
    ### Set the size of the board
    
    global gSlotsX gSlotsY 
    set gSlotsX 2
    set gSlotsY 2
    
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
    set kCAuthors "Peterson Trethewey"
    set kTclAuthors "Peterson Trethewey"
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
    global varGameSpecificOption1
    set varGameSpecificOption1 2x2
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
	    -text "How big would you like the board?:" \
	    -width 200 \
	    -foreground $kLabelColor
    
    radiobutton $w.f1.2x2 -text "2x2" -font $kLabelFont -variable varGameSpecificOption1 -value 2x2
    radiobutton $w.f1.2x3 -text "2x3" -font $kLabelFont -variable varGameSpecificOption1 -value 2x3
    radiobutton $w.f1.2x4 -text "2x4" -font $kLabelFont -variable varGameSpecificOption1 -value 2x4
    radiobutton $w.f1.3x3 -text "3x3" -font $kLabelFont -variable varGameSpecificOption1 -value 3x3
    
    ### Pack it all in
    
    pack append $w.f1 \
	    $w.f1.labMoves {left} \
	    $w.f1.2x2 {top expand fill} \
	    $w.f1.2x3 {top expand fill} \
	    $w.f1.2x4 {top expand fill} \
	    $w.f1.3x3 {top expand fill} 
    
    pack append $w $w.f1 {top expand fill}

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
    global gSlotsX
    global gSlotsY
    
    if {$varGameSpecificOption1 == "2x2"} {
	set gSlotsX 2
	set gSlotsY 2
    } elseif {$varGameSpecificOption1 == "2x3"} {
	set gSlotsX 2
	set gSlotsY 3
    } elseif {$varGameSpecificOption1 == "2x4"} {
	set gSlotsX 2
	set gSlotsY 4
    } elseif {$varGameSpecificOption1 == "3x3"} {
	set gSlotsX 3
	set gSlotsY 3
    } else {
	set gSlotsX 2
	set gSlotsY 2
    }
    
    return [list \
	    [expr {$varGameSpecificOption1 == "2x2"}] \
	    [expr {$varGameSpecificOption1 == "2x3"}] \
	    [expr {$varGameSpecificOption1 == "2x4"}] \
	    [expr {$varGameSpecificOption1 == "3x3"}]]
}



## grandwidth and grandheight get set in EmbellishSlot
set grandwidth 0
set grandheight 0

set width 80
set margin 3

set bgColor #A0F0F0
set chanColor #FFFFFF
set volColor  #E0E0E0
set dullColor #F7F7F7
set p1Color blue
set p2Color red
set highlightColor #E0E000

set animSteps 50

set altPlayer 1



#############################################################################
##
## GS_EmbellishSlot
##
## This is where we embellish a slot if its necessary
##
#############################################################################

proc GS_EmbellishSlot { w slotX slotY slot } {
    
    global gSlotSize
    global gSlotsX
    global gSlotsY
    
    global width
    
    global margin 3
    
    global  bgColor
    global  chanColor
    global  volColor
    global  dullColor
    global  p1Color
    global  p2Color
    global  highlightColor

    global grandwidth
    global grandheight
    
    global bkgndsquares
    
    global owner
    global filled
    global slotcount
    global availslots
    global bursts
    global pieces
    global paths

    global altPlayer

    global gHumanGoesFirst
    
    
    DisableSlot $w $slot
    
    $w itemconfig $slot -fill $bgColor
    $w itemconfig $slot -outline $bgColor
    
    ## well, when I wrote this code I used different variable names so...
    set grandwidth [expr {$gSlotSize*$gSlotsX}]
    set grandheight [expr {$gSlotSize*$gSlotsY}]
    
    set X $slotX
    set Y $slotY
    
    set base $w
    
    set urcx [expr {(($X+.5)*$grandwidth/$gSlotsX)-$width/2}]
    set urcy [expr {(($Y+.5)*$grandheight/$gSlotsY)-$width/2}]
    
    ## just for making the array of coordinates of slots
    set slotcount($X,$Y,0) 0
    set slotcount($X,$Y,1) 0
    set filled($X,$Y) 0
    set owner($X,$Y) 0
    
    ## for the move indicator make a square slightly larger than the bkgnd square
    set indicsquare [$base create rectangle [expr {$urcx-2*$margin}] [expr {$urcy-2*$margin}] [expr {$urcx+$width+2*$margin}] [expr {$urcy+$width+2*$margin}] -fill $chanColor -outline $chanColor]
    set movenumber [expr {$X+$gSlotsX*$Y}]
    
    $base addtag tagIndicator-$movenumber withtag $indicsquare
    
    ## this code snippet makes the pipe
    for {set dx 0} {$dx < 3} {incr dx} {
	for {set dy 0} {$dy < 3} {incr dy} {
	    if { ($dx==1 || $dy==1) && $X+$dx<=$gSlotsX && $X+$dx>0 && $Y+$dy<=$gSlotsY && $Y+$dy>0 } {
		set DX [expr {($dx-1)*$width/2}]
		set DY [expr {($dy-1)*$width/2}]
		set item [$base create rectangle [expr {$urcx+($width/3)+$DX}] [expr {$urcy+($width/3)+$DY}] [expr {$urcx+(2*$width/3)+$DX}] [expr {$urcy+(2*$width/3)+$DY}] -fill $chanColor -outline $chanColor]
		$base addtag clickable withtag $item
	    }
	}
    }
    
    ## make the bkgnd square
    set bkgndsquare [$base create rectangle [expr {$urcx-$margin}] [expr {$urcy-$margin}] [expr {$urcx+$width+$margin}] [expr {$urcy+$width+$margin}] -fill $chanColor -outline $chanColor]
    $base addtag clickable withtag $bkgndsquare
    set bkgndsquares($X,$Y) $bkgndsquare
    
    ## build an array of squares
    set bkgndsquares($X,$Y) $bkgndsquare
    
    ## cycle through each direction...
    for {set dx 0} {$dx < 3} {incr dx} {
	for {set dy 0} {$dy < 3} {incr dy} {
	    
	    ##figure out what color the slot should be
	    set thiscolor $dullColor
	    set primeslot 0
	    
	    if { ($dx==1 || $dy==1) && $X+$dx<=$gSlotsX && $X+$dx>0 && $Y+$dy<=$gSlotsY && $Y+$dy>0 } {
		set thiscolor $volColor
		set primeslot 1
	    }
	    
	    ## in the array set the colors of the slots
	    set a [expr {$urcx+($dx*$width/3)}]
	    set b [expr {$urcy+($dy*$width/3)}]
	    set c [expr {$urcx+(($dx+1)*$width/3)}]
	    set d [expr {$urcy+(($dy+1)*$width/3)}]
	    
	    ## build array of next available slots
	    set availslots($X,$Y,$primeslot,$slotcount($X,$Y,$primeslot),x1) $a
	    set availslots($X,$Y,$primeslot,$slotcount($X,$Y,$primeslot),y1) $b
	    set availslots($X,$Y,$primeslot,$slotcount($X,$Y,$primeslot),x2) $c
	    set availslots($X,$Y,$primeslot,$slotcount($X,$Y,$primeslot),y2) $d
	    incr slotcount($X,$Y,$primeslot)
	    
	    set item [$base create oval $a $b $c $d -width 0 -fill $thiscolor -outline $chanColor]
	    $base addtag clickable withtag $item
	}
    }
    
    ## take that array and put it into a new array combining low and high priority pieces in one
    for {set i 0} {$i < $slotcount($X,$Y,1)} {incr i} {
	set availslots($X,$Y,$i,x1) $availslots($X,$Y,1,$i,x1)
	set availslots($X,$Y,$i,y1) $availslots($X,$Y,1,$i,y1)
	set availslots($X,$Y,$i,x2) $availslots($X,$Y,1,$i,x2)
	set availslots($X,$Y,$i,y2) $availslots($X,$Y,1,$i,y2)
    }
    for {set i 0} {$i < $slotcount($X,$Y,0)} {incr i} {
	set availslots($X,$Y,[expr {$i+$slotcount($X,$Y,1)}],x1) $availslots($X,$Y,0,$i,x1)
	set availslots($X,$Y,[expr {$i+$slotcount($X,$Y,1)}],y1) $availslots($X,$Y,0,$i,y1)
	set availslots($X,$Y,[expr {$i+$slotcount($X,$Y,1)}],x2) $availslots($X,$Y,0,$i,x2)
	set availslots($X,$Y,[expr {$i+$slotcount($X,$Y,1)}],y2) $availslots($X,$Y,0,$i,y2)
    }
        
    
    ## if the computer goes first, change stuff around
    if {!$gHumanGoesFirst} {
    	set altPlayer 2
    } else {
	set altPlayer 1
    }
    
    
    ## setting variables before the idle stage
    
    set bursts($slotX,$slotY) false
    set pieces($slotX,$slotY,0) 0
    set piecesToAdd($slotX,$slotY) 0
    
    
    ## so what if we do this more than necessary
    
    $base bind clickable <Any-1> "takeClick $base %x %y"
    ##$base bind clickable <Any-Enter> "takeEnter $base %x %y"
    $base bind clickable <ButtonRelease-1> "letUp $base %x %y"
    $base bind clickable <Any-Leave> "Leave $base %x %y"
    
    $w addtag tagInitial withtag $slot
}




###################################
## My own event handling follows ##
###################################


set squareisdown false
set buttonisdown false
set numberOfPaths 0
set paths(0) 0
set burstingPlayer 1


proc takeClick {w x y} {
    global squaredown
    global highlightColor
    global bkgndsquares
    global grandwidth 
    global grandheight
    global squareisdown
    global buttonisdown
    global owner
    global altPlayer
    global gSlotsX
    global gSlotsY
    
    set lX [expr {$x/($grandwidth/$gSlotsX)}]
    set lY [expr {$y/($grandheight/$gSlotsY)}]
    
    if { $altPlayer==$owner($lX,$lY) || $owner($lX,$lY)==0 } {
	set squaredown $bkgndsquares($lX,$lY)
	set squareisdown true
	$w itemconfig $squaredown -fill $highlightColor
    }
    
    set buttonisdown true
}


proc takeEnter {w x y} {
    global squaredown
    global highlightColor
    global bkgndsquares
    global grandwidth 
    global grandheight
    global squareisdown
    global buttonisdown
    global owner
    global altPlayer
    global gSlotsX
    global gSlotsY
    
    set lX [expr {$x/($grandwidth/$gSlotsX)}]
    set lY [expr {$y/($grandheight/$gSlotsY)}]
    
    if { $buttonisdown } {
	if { $altPlayer==$owner($lX,$lY) || $owner($lX,$lY)==0 } {
	    set squaredown $bkgndsquares($lX,$lY)
	    set squareisdown true
	    $w itemconfig $squaredown -fill $highlightColor
	}
    }
}


proc Leave {w x y} {
    global squaredown
    global squareisdown
    global buttonisdown
    global chanColor
    global bkgndsquares
    
    if {$squareisdown} {
	$w itemconfig $squaredown -fill $chanColor
	set squareisdown false
    }
}


proc letUp {w x y} {
    global squaredown
    global squareisdown
    global chanColor
    global bkgndsquares
    global grandwidth
    global grandheight
    global p1Color
    global p2Color
    global bursts
    global altPlayer
    global contents
    global gSlotsX
    global gSlotsY
    
    if {$squareisdown} {
	$w itemconfig $squaredown -fill $chanColor
	set squareisdown false
	
	set lX [expr {$x/($grandwidth/$gSlotsX)}]
	set lY [expr {$y/($grandheight/$gSlotsY)}]
	
	set movenumber [expr {$lX+$gSlotsX*$lY}]
	HandleMove $movenumber
    }
    
    set buttonisdown false
}



proc forceMove {w theMove} {
    
    global squaredown
    global squareisdown
    global chanColor
    global bkgndsquares
    global grandwidth
    global grandheight
    global p1Color
    global p2Color
    global bursts
    global altPlayer
    global contents
    global gSlotsX
    global gSlotsY
    
    
    ## FIE thee, TCL!
    set lY [expr {$theMove/$gSlotsX}]
    set lX [expr {$theMove-$lY*$gSlotsX}]
    
    set color $p1Color    
    if {$altPlayer==1} {
	set color $p1Color
    } else {
	set color $p2Color
    }
    
    
    set movenumber [expr {$lX+$gSlotsY*$lY}]
    placePiece $w $lX $lY $color
    
    clearBursts
    while {[checkForBursts] && ![boardIsOccupied]} {
	burstIfNeeded $w
	clearBursts
    }
    
    if {$altPlayer==1} {
	set altPlayer 2
    } else {
	set altPlayer 1
    }
}



proc placePiece {w X Y color} {

    global availslots
    global grandwidth
    global filled
    global p1Color
    global p2Color
    global owner
    global pieces
    global altPlayer
    
    set a $availslots($X,$Y,$filled($X,$Y),x1)
    set b $availslots($X,$Y,$filled($X,$Y),y1)
    set c $availslots($X,$Y,$filled($X,$Y),x2)
    set d $availslots($X,$Y,$filled($X,$Y),y2)
    
    set item [$w create oval $a $b $c $d -width 0 -fill $color -outline $color]
    $w addtag clickable withtag $item
    $w addtag tagPiece withtag $item
    set pieces($X,$Y,$filled($X,$Y)) $item
    incr filled($X,$Y)
    
    if {$color==$p1Color} {
	set owner($X,$Y) 1
    } else {
	set owner($X,$Y) 2
    }
}




proc checkForBursts {} {
    global availslots
    global grandwidth
    global filled
    global pieces
    global p1Color
    global p2Color
    global owner
    global bursts
    global slotcount
    global altPlayer
    global gSlotsX
    global gSlotsY
    
    set foundone false
    
    for {set X 0} {$X < $gSlotsX} {incr X} {
	for {set Y 0} {$Y < $gSlotsY} {incr Y} {
	    	    
	    if {$filled($X,$Y)>=$slotcount($X,$Y,1)} {
		set foundone true
		set bursts($X,$Y) true
	    } else {
		set bursts($X,$Y) false
	    }
	}
    }
    
    return $foundone 
}



proc boardIsOccupied {} {
    global availslots
    global grandwidth
    global filled
    global pieces
    global p1Color
    global p2Color
    global owner
    global bursts
    global slotcount
    global altPlayer
    global gSlotsX
    global gSlotsY
    global owner
    
    set foundone 1
    
    for {set X 0} {$X < $gSlotsX} {incr X} {
	for {set Y 0} {$Y < $gSlotsY} {incr Y} {
	    if {$owner($X,$Y)==0 || $owner($X,$Y)!=$owner(0,0)} {
		set foundone 0
	    }
	}
    }
    
    return $foundone 
}


proc burstIfNeeded {w} {
    global availslots
    global grandwidth
    global filled
    global p1Color
    global p2Color
    global owner
    global bursts
    global slotcount
    global paths
    global numberOfPaths
    global bkgndsquares
    global animSteps
    global piecesToAdd
    global burstingPlayer
    global width
    global pieces
    global altPlayer
    global gSlotsX
    global gSlotsY
    global highlightColor
    global chanColor
    
    
    for {set X 0} {$X < $gSlotsX} {incr X} {
	for {set Y 0} {$Y < $gSlotsY} {incr Y} {
	    if {$bursts($X,$Y)} {
		makeBurstPaths $X $Y
	    }
	}
    }
    
    
    ## here's where we actually animate
    
    
    set lastRed false
    
    for {set t 0.0} {$t < 1.0} {set t [expr {$t+(1.0/$animSteps)}]} {
	for {set i 0} {$i < $numberOfPaths} {incr i} {
	    
	    set arc [expr {$width*3*($t)*(1-$t)}]
	    
	    set x1 [expr {$paths($i,source,x1)*(1-$t) + $paths($i,destination,x1)*$t}]
	    set y1 [expr {$paths($i,source,y1)*(1-$t) + $paths($i,destination,y1)*$t} - $arc]
	    set x2 [expr {$paths($i,source,x2)*(1-$t) + $paths($i,destination,x2)*$t}]
	    set y2 [expr {$paths($i,source,y2)*(1-$t) + $paths($i,destination,y2)*$t} - $arc]
	    
	    $w coords $paths($i,piece) $x1 $y1 $x2 $y2
	    update idletasks
	}
	
	for {set X 0} {$X < $gSlotsX} {incr X} {
	    for {set Y 0} {$Y < $gSlotsY} {incr Y} {
		if {$bursts($X,$Y)} {
		    if { $t<.25 } {
			if { $lastRed } {
			    $w itemconfig $bkgndsquares($X,$Y) -fill $chanColor
			    $w itemconfig $bkgndsquares($X,$Y) -outline $chanColor
			    set lastRed false
			} else {
			    $w itemconfig $bkgndsquares($X,$Y) -fill $highlightColor
			    $w itemconfig $bkgndsquares($X,$Y) -outline $highlightColor
			    set lastRed true
			}
		    } else {
			$w itemconfig $bkgndsquares($X,$Y) -fill $chanColor
			$w itemconfig $bkgndsquares($X,$Y) -outline $chanColor
		    }
		}
	    }
	}
    }
    
    
    if {$burstingPlayer==1} {
	set color $p1Color
    } else {
	set color $p2Color
    }
    
    
    ## in the end make sure everything is in the right place
    for {set i 0} {$i < $numberOfPaths} {incr i} {
	set x $paths($i,destinationsquare,x)
	set y $paths($i,destinationsquare,y)
	set x1 $paths($i,destination,x1)
	set y1 $paths($i,destination,y1)
	set x2 $paths($i,destination,x2)
	set y2 $paths($i,destination,y2)
	$w coords $paths($i,piece) $x1 $y1 $x2 $y2
	set pieces($x,$y,$filled($x,$y)) $paths($i,piece)
	incr filled($x,$y)
	
	## occupy colors
	for {set a 0} {$a < $filled($x,$y)} {incr a} {
	    $w itemconfig $pieces($x,$y,$a) -fill $color
	    $w itemconfig $pieces($x,$y,$a) -outline $color
	}
	
	## abstractly as well as actually
	set owner($x,$y) $burstingPlayer
    }   
    
    
}



proc clearBursts {} {
    global numberOfPaths
    global brusts
    global piecesToAdd
    global altPlayer
    global gSlotsX
    global gSlotsY
    
    set numberOfPaths 0
    ## setting variables before the idle stage of pieces in each square
    for {set X 0} {$X < $gSlotsX} {incr X} {
	for {set Y 0} {$Y < $gSlotsY} {incr Y} {
	    set bursts($X,$Y) false
	    set piecesToAdd($X,$Y) 0
	}
    }
}



proc makeBurstPaths {X Y} {
    global numberOfPaths
    global paths
    global availslots
    global filled 
    global owner
    global pieces
    global piecesToAdd
    global burstingPlayer
    global altPlayer
    global gSlotsX
    global gSlotsY
    
    
    set burstingPlayer $altPlayer
    
    for {set dx 0} {$dx < 3} {incr dx} {
	for {set dy 0} {$dy < 3} {incr dy} {
	    ## lower case is the destination believe it or not
	    set x [expr {$dx+$X-1}]
	    set y [expr {$dy+$Y-1}]
	    
	    if { ($dx==1 || $dy==1) && !($dx==1 && $dy==1) && $x<$gSlotsX && $x>=0 && $y<$gSlotsY && $y>=0 } {
		
		## determine the paths
		set i [expr {$filled($X,$Y)-1}]
		set paths($numberOfPaths,source,x1) $availslots($X,$Y,$i,x1)
		set paths($numberOfPaths,source,y1) $availslots($X,$Y,$i,y1)
		set paths($numberOfPaths,source,x2) $availslots($X,$Y,$i,x2)
		set paths($numberOfPaths,source,y2) $availslots($X,$Y,$i,y2)
		set i [expr {$filled($x,$y)+$piecesToAdd($x,$y)}]
		set paths($numberOfPaths,destination,x1) $availslots($x,$y,$i,x1)
		set paths($numberOfPaths,destination,y1) $availslots($x,$y,$i,y1)
		set paths($numberOfPaths,destination,x2) $availslots($x,$y,$i,x2)
		set paths($numberOfPaths,destination,y2) $availslots($x,$y,$i,y2)
		set paths($numberOfPaths,destinationsquare,x) $x
		set paths($numberOfPaths,destinationsquare,y) $y
		
		## decrement number of pieces in square 
		set filled($X,$Y) [expr {$filled($X,$Y)-1}]
		set paths($numberOfPaths,piece) $pieces($X,$Y,$filled($X,$Y))
		
		## increment number of pieces to add
		incr piecesToAdd($x,$y)
				
		incr numberOfPaths
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
    global gSlotsX
    global gSlotsY
    
    for {set i 0} {$i < $gSlotsX*$gSlotsY} {incr i} {
	$w itemconfig tagIndicator-$i -fill $chanColor
	$w itemconfig tagIndicator-$i -outline $chanColor
    }
    forceMove $w $theMove
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
    ### Delete all pieces that are left around
        
    global pieces
    global owner
    global availslots
    global filled
    global paths
    global numberOfPaths
    global gSlotsX
    global gSlotsY
    global altPlayer
    global gHumanGoesFirst
    
    
    for {set X 0} {$X < $gSlotsX} {incr X} {
	for {set Y 0} {$Y < $gSlotsY} {incr Y} { 
	    set bursts($X,$Y) false
	    set pieces($X,$Y,0) 0
	    set filled($X,$Y) 0
	    set owner($X,$Y) 0
	}   
    }
    
    
    ## if the computer goes first, change stuff around
    if {!$gHumanGoesFirst} {
    	set altPlayer 2
    } else {
	set altPlayer 1
    }
        
    set numberOfPaths 0
    $w delete tagPiece
}



proc empty {w a b c d e} {
}
































