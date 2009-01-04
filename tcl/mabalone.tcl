
# GS_InitGameSpecific initializes game-specific features
# of the current game.  You can use this function 
# to initialize data structures, but not to present any graphics.
# It is called when the player first opens the game
# and after every rule change.
# You must set the global variables kGameName, gInitialPosition,
# kCAuthors, kTclAuthors, and kGifAuthors in this function.

proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "Abalone"

    # Authors Info
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Melinda Franco & Michael Mottmann"
    set kTclAuthors "Michael Mottmann"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    
    global gInitialPosition gPosition pieceString Boardsize
    
        
    set gInitialPosition [C_InitialPosition]
    #Print "initial pos is: $gInitialPosition"
    set gPosition $gInitialPosition
    set pieceString [string range [C_GenericUnhash $gInitialPosition $Boardsize] 0 [expr $Boardsize - 1]]
    

    # Set toMove toWin
    #global gMisereGame
    #if { $gMisereGame } {
#	set toWin1 "To Lose: "
 #   } else {
	#set toWin1 "To Win: "
#    }

 #   set toWin2 "Push the specified number of pieces off the board."

#    SetToWinString [concat $toWin1 $toWin2]

#    SetToMoveString  "To Move: On your turn you may move either 1,2, or 3 of you marbles into any of the six adjacent spaces. The space you move to must be empty and all the marbles must move in the same direction. To push your opponent set up a sumito by having a numerical superiority over your opponent's marbles. In other words, your pieces must outnumber the player's pieces which are being pushed. Remember that you may only use 3 pieces at a time to push your opponent. During the push move, you may eject pieces off the board by pushing the opponent's pieces off the board. "
    
    ### Set the initial position of the board
 #   global gInitialPosition gPosition
  #  set gInitialPosition [expr int((pow(2, $gRows*3) - 1)) * 2]
   # set gPosition $gInitialPosition
}


# GS_Initialize draws the graphics for the game on the canvas c
# You could put an opening animation or just draw an empty board.
# This function is called after GS_InitGameSpecific

proc GS_Initialize { c } {

    # you may want to start by setting the size of the canvas; this line isn't cecessary
    global cHeight cWidth N gPosition
    $c configure -width $cWidth -height $cHeight
    $c create rect 0 0 $cWidth $cHeight -fill white -outline white 
    #-tag base
    
    makeBoard $c
    $c lower arrow
    #$c itemconfig arrow -fill cyan
    #$c raise arrowNW56
    #$c raise arrow
    #$c raise west
    #$c raise northeast
    #$c raise northwest
    #set slot 0
    #set x 275
    #set varname x$slot$slot

    #set x$slot$slot $x
    #$c create text 250 250 -text $$varname

    
    #setPieces $c
    
    #$c itemconfig red1 -width 100
    #$c bind red1 <Enter> "ExpandRed1 $c"

    global pieceString gInitialPosition Boardsize

    #set op [maxPieces $N]
    #$c create text 250 250 -text $pieceString
    #$c create text 250 250 -text [expr $gInitialPosition + 1]
    #$c create line 250 250 300 250 -tags N$N.x
    #$c itemconfig N2.x -width 5
    #$c lower arrow
    #$c raise arrow
    #$c itemconfig arrow -arrow last -arrowshape {10 10 3}
    
}
#proc MouseOverExpand {dot c} {
#    $c itemconfig $dot -width 50
#}

proc totalSpaces {n} {
    if {$n==1} {
	return 1
    } else {
	return [expr ($n-1)*6 + [totalSpaces [expr $n-1]]]
    }
}

proc madePushMove {slot direction} {
    global pieceString
    set slot2 [C_Destination $slot $direction]
    set slot3 [C_Destination $slot2 $direction]

    if {[string compare [string index $pieceString $slot2] [string index $pieceString $slot]] != 0} {
	#one piece move
	ReturnFromHumanMove [C_MoveHash $slot 99 99 $direction]
    } elseif {[string compare [string index $pieceString $slot3] [string index $pieceString $slot]] !=0} {
	#two piece move
	ReturnFromHumanMove [C_MoveHash $slot $slot2 99 $direction]
    } else {
	#three piece move
	ReturnFromHumanMove [C_MoveHash $slot $slot2 $slot3 $direction]
    }
}

proc makeBoard {c} {
    global cHeight cWidth spacer diameter N ygap xgap lineWidth lineSpace
    set slot 0
    set coords [list]
    
    set y [expr $ygap + .5*$diameter]

    for {set row 1} {$row<=[expr 2*$N-1]} {incr row} { 
	set x [expr $xgap + .5*$diameter*(abs($N-$row) + 1)]
	set nextx [expr $x + $spacer + $diameter]
	set prevx [expr $x - $spacer - $diameter]

	for {set piece 1} {$piece<=[expr (2*$N-1)-abs($N-$row)]} {incr piece} {
	    $c create oval [expr $x - ($diameter/2)] [expr $y-($diameter/2)] [expr $x+($diameter/2)] [expr $y+($diameter/2)] -outline black -fill Black -tags [list slot slot$slot]
	    	    
	    #if not at the end of the row
	    if {$piece!=[expr (2*$N-1)-abs($N-$row)]} {
		#push east
		$c create line $x $y [expr ($x+$nextx)/2] $y -arrow last -width $lineWidth -tags [list arrow east arrowE$slot]
		$c bind arrowE$slot <ButtonRelease-1> "madePushMove $slot 1"
	    }
	    
	    #if not at the start of the row
	    if {$piece!=1} {
		#push west
		$c create line $x $y [expr ($x+$prevx)/2] $y -arrow last -width $lineWidth -tags [list arrow west arrowW$slot]
		$c bind arrowW$slot <ButtonRelease-1> "madePushMove $slot -1"
	    }
	    #if not the first row
	    if {$row!=1} {
		#make sure there exists a piece to the NW
		if {($piece!=1) || ($row>$N)} {
		    set upslot [C_Destination $slot -2]
		    set upx [expr ($prevx + $x)/2]
		    set upy [expr $y - $diameter]
		    set midx [expr ($upx + $x)/2]
		    set midy [expr ($upy + $y)/2]
		    set midx0 [expr $midx - ($diameter + $spacer)/2]
		    set midx2 [expr $midx + ($diameter + $spacer)/2]

		    #push SE
		    $c create line $upx $upy [expr ($upx +$x)/2] [expr ($upy+$y)/2] -width $lineWidth -fill cyan -arrow last -tags [list arrow southeast arrowSE$upslot]
		    $c bind arrowSE$upslot <ButtonRelease-1> "madePushMove $upslot 2"
		    #push NW
		    $c create line [expr ($x + $upx)/2] $midy $x $y -width $lineWidth -fill cyan -arrow first -tags [list arrow northwest arrowNW$slot]
		    $c bind arrowNW$slot <ButtonRelease-1> "madePushMove $slot -2" 
		    
		    #sidesteps w/this piece
		    if {($piece>2) || (($row>$N) && ($piece>1))} {
			#can sidestep west
			$c create line $x $y $upx $upy -width $lineWidth -tags [list arrow arrowW$upslot$slot]
			$c create line $midx $midy $midx0 $midy -width $lineWidth -arrow last -tags [list arrow arrowW$upslot$slot]
			$c bind arrowW$upslot$slot <ButtonRelease-1> "ReturnFromHumanMove [C_MoveHash $upslot $slot 99 -1]"
			##3 piece sidestep here
		    }
		    if {($piece < [expr (2*$N-2)-abs($N-$row)]) || (($piece<[expr (2*$N-1)-abs($N-$row)]) && ($row>=$N))} {
			#can sidestep east
			$c create line $x $y $upx $upy -width $lineWidth -tags [list arrow arrowE$upslot$slot]
			$c create line $midx $midy $midx2 $midy -width $lineWidth -arrow last -tags [list arrow arrowE$upslot$slot]
			$c bind arrowE$upslot$slot <ButtonRelease-1> "ReturnFromHumanMove [C_MoveHash $upslot $slot 99 1]"
			##3 piece sidestep here
		    }
		}
		#make sure there exists a piece to the NE
		if {($piece!=[expr (2*$N-1)-abs($N-$row)]) || ($row>$N)} {
		    set upslot [C_Destination $slot -3]
		    set upx [expr ($nextx + $x)/2]
		    set upy [expr $y - $diameter]
		    set midx [expr ($upx + $x)/2]
		    set midy [expr ($upy + $y)/2]
		    set midx0 [expr $midx - ($diameter + $spacer)/2]
		    set midx2 [expr $midx + ($diameter + $spacer)/2]

		    #push SW
		    $c create line $upx $upy [expr ($upx + $x)/2] [expr ($upy +$y)/2] -width $lineWidth -arrow last -tags [list arrow southwest arrowSW$upslot]
		    $c bind arrowSW$upslot <ButtonRelease-1> "madePushMove $upslot 3"
		    #push NE
		    $c create line [expr ($x + $upx)/2] [expr ($y + $upy)/2] [expr $x] $y -width $lineWidth -arrow first -tags [list arrow northeast arrowNE$slot]
		    $c bind arrowNE$slot <ButtonRelease-1> "madePushMove $slot -3"
		    #now do sidestes w/this piece
		    if {$piece>1} {
			#can go west
			$c create line $upx $upy $x $y -width $lineWidth -tags [list arrow arrowW$upslot$slot]
			$c create line $midx $midy $midx0 $midy -width $lineWidth -arrow last -tags [list arrow arrowW$upslot$slot]
			$c bind arrowW$upslot$slot <ButtonRelease-1> "ReturnFromHumanMove [C_MoveHash $upslot $slot 99 -1]"
		    }
		    if {($piece < [expr (2*$N-2)-abs($N-$row)]) || (($row > $N) && ($piece < [expr (2*$N-1)-abs($N-$row)]))} {
			#can go east
			$c create line $upx $upy $x $y -width $lineWidth -tags [list arrow arrowE$upslot$slot]
			$c create line $midx $midy $midx2 $midy -width $lineWidth -arrow last -tags [list arrow arrowE$upslot$slot]
			$c bind arrowE$upslot$slot <ButtonRelease-1> "ReturnFromHumanMove [C_MoveHash $upslot $slot 99 1]"
		    }
			
		}
	    }

	    #now do horizontal sidesteps, could have done above, but want to be more readable, so there
	    if {$piece > 1} {
		#do two pieces
		if {$row != 1} {
		    if {($row > $N) || ($piece>2)} {
			#can go NW
			set backslot [expr $slot-1]
			set upslot1 [C_Destination $backslot -2]
			set upslot2 [expr $upslot1 + 1]
			set upx2 [expr ($prevx +$x)/2]
			set upx1 [expr $upx2 -$spacer -$diameter]
			set upy [expr $y -$diameter]
			set midy [expr ($y + $upy)/2]

			$c create line $prevx $y $x $y -width $lineWidth -tags [list arrow arrowNW$backslot$slot] 
			$c create line $upx2 $y [expr ($upx2+$prevx)/2] $midy -width $lineWidth -arrow last -tags [list arrow arrowNW$backslot$slot]
			$c bind arrowNW$backslot$slot <ButtonRelease-1> "ReturnFromHumanMove [C_MoveHash $backslot $slot 99 -2]"
			
			#therefore, there exists a SE move
			$c create line $upx1 $upy $upx2 $upy -width $lineWidth -tags [list arrow arrowSE$upslot1$upslot2]
			$c create line $prevx $upy [expr ($upx2+$prevx)/2] $midy -width $lineWidth -arrow last -tags [list arrow arrowSE$upslot1$upslot2]
			$c bind arrowSE$upslot1$upslot2 <ButtonRelease-1> "ReturnFromHumanMove [C_MoveHash $upslot1 $upslot2 99 2]"
			
		    }
		    if {($row > $N) || ($piece < [expr (2*$N-1)-abs($N-$row)])} {
			#can go NE
			set backslot [expr $slot-1]
			set upslot1 [C_Destination $backslot -3]
			set upslot2 [expr $upslot1 + 1]
			set upx1 [expr ($prevx +$x)/2]
			set upx2 [expr $upx1 + $spacer + $diameter]
			set upy [expr $y -$diameter]
			set midy [expr ($y + $upy)/2]

			$c create line $prevx $y $x $y -width $lineWidth -tags [list arrow arrowNE[expr $slot-1]$slot] 
			$c create line $upx1 $y [expr ($upx1+$x)/2] $midy -width $lineWidth -arrow last -tags [list arrow arrowNE$backslot$slot]
			$c bind arrowNE$backslot$slot <ButtonRelease-1> "ReturnFromHumanMove [C_MoveHash $slot $backslot 99 -3]"
			
			#therefore, there exists a SW move
		      	$c create line $upx1 $upy $upx2 $upy -width $lineWidth -tags [list arrow arrowSW$upslot1$upslot2]
			$c create line $x $upy [expr ($x+$upx1)/2] $midy -width $lineWidth -arrow last -tags [list arrow arrowSW$upslot1$upslot2]
			$c bind arrowSW$upslot1$upslot2 <ButtonRelease-1> "ReturnFromHumanMove [C_MoveHash $upslot1 $upslot2 99 3]"
		    }
		}
		
	    }
	    if {$piece > 2} {
		#do three pieces
	    }

	    set prevx $x
	    set x $nextx
	    set nextx [expr $x + $spacer + $diameter]
	    set slot [expr $slot + 1]
	}
	set y [expr $y + $diameter]
	}
    }

proc setPieces {c} {
    global Pieces spacer diameter N ygap xgap pieceString
    set slot 0
    
    set y $ygap
    for {set row 1} {$row<=[expr 2*$N-1]} {incr row} { 
	set x [expr $xgap + .5*$diameter*abs($N-$row)]
	for {set piece 1} {$piece<=[expr (2*$N-1)-abs($N-$row)]} {incr piece} {
	    
	    if {[string compare [string index $pieceString $slot] "x"] == 0} {
		set slot$slot [$c create oval $x $y [expr $x+$diameter] [expr $y+$diameter] -outline black -fill Red]
		#$c bind slot$slot <Enter> "MouseOverExpand $redsLeft $c"
	    } elseif {[string compare [string index $pieceString $slot] "*"] == 0} {
		set slot$slot [$c create oval $x $y [expr $x+$diameter] [expr $y+$diameter] -outline white -fill black]
	    } else {
		set slot$slot [$c create oval $x $y [expr $x+$diameter] [expr $y+$diameter] -outline black -fill Blue]
	    }
	    set slot [expr $slot + 1]
	    set x [expr $x + $spacer + $diameter]
	}
	set y [expr $y + $diameter]
    }
}

# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"
# The left player's piece should be the first item in the list.
# The right player's piece should be second.

proc GS_NameOfPieces {} {
    return [list left right]
}

# GS_ColorOfPlayers should return a list of two strings, 
# each representing the color of a player.
# If a specific color appears uniquely on one player's pieces,
# it might be a good choice for that player's color.
# In impartial games, both players may share the same color.
# If the game is tic tac toe, this might be the line 
# return [list blue red]
# If the game is nim, this might be the line
# return [list green green]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he clicks "New Game"
# The left player's color should be the first item in the list.
# The right player's color should be second.

proc GS_ColorOfPlayers {} {
    return [list blue red]
}


# Setup the rules frame
# Adds widgets to the rules frame that will allow the user to 
# select the variant of this game to play. The options 
# selected by the user should be stored in a set of global
# variables.
# This procedure must initialize the global variables to some
# valid game variant.
# The rules frame must include a standard/misere setting.
# Args: rulesFrame (Frame) - The rules frame to which widgets
# should be added
# Modifies: the rules frame and its global variables
# Returns: nothing

proc GS_SetupRulesFrame { rulesFrame } {

    set standardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	    ]

    set rowsRule \
	[list \
	     "How many rows?" \
	     "1" "2" "3" "4" "5"
	 ]

    global N Misere NoSideSteps XHitKills Pieces Boardsize
    global diameter spacer lineWidth 
    global ygap xgap
    global cHeight cWidth

    set cHeight 500
    set cWidth 500
    
    set N 2
    set Misere 0
    set NoSideSteps 0
    set XHitKills 1
    set Pieces 2
    set Boardsize [totalSpaces $N]

    set diameter 50
    set spacer 7
    set lineWidth 5
    
    set ygap [expr ($cHeight - (2*$N-1)*$diameter)/2]
    set xgap [expr ($cWidth - ((2*$N-1)*$diameter + (2*$N-2)*$spacer))/2]

    # List of all rules, in some order
#    set ruleset [list $standardRule $rowsRule]

    # Declare and initialize rule globals
#    global gMisereGame gRowsOption
 #   set gMisereGame 0
  #  set gRowsOption 4

    # List of all rule globals, in same order as rule list
   # set ruleSettingGlobalNames [list "gMisereGame" "gRowsOption"]

 #   global kLabelFont
  #  set ruleNum 0
   # foreach rule $ruleset {
#	frame $rulesFrame.rule$ruleNum -borderwidth 2 -relief raised
#	pack $rulesFrame.rule$ruleNum  -fill both -expand 1
#	message $rulesFrame.rule$ruleNum.label -text [lindex $rule 0] -font $kLabelFont
#	pack $rulesFrame.rule$ruleNum.label -side left
#	set rulePartNum 0
#	foreach rulePart [lrange $rule 1 end] {
#	    radiobutton $rulesFrame.rule$ruleNum.p$rulePartNum -text $rulePart -variable [lindex $ruleSettingGlobalNames $ruleNum] -value $rulePartNum -highlightthickness 0 -font $kLabelFont
#	    pack $rulesFrame.rule$ruleNum.p$rulePartNum -side left -expand 1 -fill both
#	    incr rulePartNum
#	}
#	incr ruleNum
#    } 
}


# Get the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code

proc GS_GetOption { } {
    global N Misere NoSideSteps XHitKills Pieces
    set option 1;
    for {set n 2} {$n < $N} {incr n} {
	for {set p 2} {$p <= [maxPieces $n]} {incr p} {
	    set option [expr $option + (4*$p)]
	}
    }
    for {set p 2} {$p < $Pieces} {incr p} {
	set option [expr $option + (4*$p)]
    }
    if {$NoSideSteps == 1} {
	set option [expr $option + 2]
    }
    if {$Misere == 1} {
	set option [expr $option + 1]
    }
    return $option
}

proc maxPieces {n} {
    if {$n == 2} {
	return 2
    }
    return [expr $n + 2 * ($n - 2) + [maxPieces [expr $n - 1]]]
}


# Modify the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option.
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing

proc GS_SetOption { option } {
global N Misere NoSideSteps XHitKills Pieces
    set option [expr $option - 1]
    set n 2
    set pieces 2
    set kills 0
    for {set hash 0} {$hash<=$option} {set hash [expr $hash + 4]} {
	if {($pieces == [maxPieces $n]) && ($pieces == $kills)} {
	    set pieces 2
	    set kills 1
	    set n [expr $n + 1]
	} elseif {$kills == $pieces} {
	    set pieces [expr $pieces + 1]
	    set kills 1
	} else {
	    set kills [expr $kills + 1]
	}
    }
    set N $n
    set Pieces $pieces
    set XHitKills $kills
    
    set mod [expr fmod([expr $hash - $option], 4)]
    if {$mod == 0} {
	set NoSideSteps 0
	set Misere 0
    } elseif {$mod == 3} {
	set NoSideSteps 0
	set Misere 1
    } elseif {$mod == 2} {
	set NoSideSteps 1
	set Misere 0
    } else {
	set NoSideSteps 1
	set Misere 1
    }
}
 
proc GS_Deinitialize { c } {
    $c delete all
}

proc MyReturnFromHumanMove {w h} {
#    global gPosition
#    set l [unhash $gPosition]
#    set dummy [expr [lindex $l [expr $w-1]]-$h+1]
#    ReturnFromHumanMove $w$dummy
}



# GS_DrawPosition this draws the board in an arbitrary position.
# It's arguments are a canvas, c, where you should draw and the
# (hashed) position.  For example, if your game is a rearranger,
# here is where you would draw pieces on the board in their correct positions.
# Imagine that this function is called when the player
# loads a saved game, and you must quickly return the board to its saved
# state.  It probably shouldn't animate, but it can if you want.

proc GS_DrawPosition { c position } {
    global Pieces spacer diameter N ygap xgap Boardsize pieceString
    set slot 0
    set pieceString [string range [C_GenericUnhash $position $Boardsize] 0 [expr $Boardsize-1]]

    for {set slot 0} {$slot < $Boardsize} {incr slot} {
	if {[string compare [string index $pieceString $slot] "x"] == 0} {
	    $c itemconfig slot$slot -fill Red3
	} elseif {[string compare [string index $pieceString $slot] "*"] == 0} {
	    $c itemconfig slot$slot -fill black
	} else {
	    $c itemconfig slot$slot -fill Blue3
	}
    }
   
}
      

#    set y $ygap
#    for {set row 1} {$row<=[expr 2*$N-1]} {incr row} { 
#	set x [expr $xgap + .5*$diameter*abs($N-$row)]
#	for {set piece 1} {$piece<=[expr (2*$N-1)-abs($N-$row)]} {incr piece} {
	    
#	    if {[string compare [string index $board $slot] "x"] == 0} {
#		set slot$slot [$c create oval $x $y [expr $x+$diameter] [expr $y+$diameter] -outline black -fill Red]
		#$c bind slot$slot <Enter> "MouseOverExpand $redsLeft $c"
#	    } elseif {[string compare [string index $board $slot] "*"] == 0} {
#		set slot$slot [$c create oval $x $y [expr $x+$diameter] [expr $y+$diameter] -outline white -fill black]
#	    } else {
#		set slot$slot [$c create oval $x $y [expr $x+$diameter] [expr $y+$diameter] -outline black -fill Blue]
#	    }
#	    set slot [expr $slot + 1]
#	    set x [expr $x + $spacer + $diameter]
#	}
#	set y [expr $y + $diameter]
#    }
 #   set l [unhash $position]
    
#    global gRows
 #   for {set j 0} {$j<7} {incr j} {
	#for {set i 0} {$i<$gRows} {incr i} {
	    
#	    set x [expr 70*$i+60]
#	    set y [expr 400-(30*$j+50)]
#	    set h [expr $j+1]
#	    set w [expr $i+1]
#	    $c coords move-$w$h [expr $x-25] [expr $y-20] [expr $x+25] [expr $y+20]
#	    $c raise move-$w$h
#	}
 #   }
    
#    for {set i 1} {$i<=$gRows} {incr i} {
	#set height [lindex $l [expr $i-1]]
#	for {set j 7} {$j>$height} {incr j -1} {
#	    $c move move-$i$j 0 -400
#	}
#    }


# GS_NewGame should start playing the game. "let's play"  :)
# It's arguments are a canvas, c, where you should draw and
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.

proc GS_NewGame { c position } {
    # The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    GS_DrawPosition $c $position
}


# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.

proc GS_WhoseMove { position } {
    return [C_WhoseMove position]
}


# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

proc GS_HandleMove { c oldPosition theMove newPosition } {
    GS_DrawPosition $c $newPosition

 #   set oldl [unhash $oldPosition]
#    set newl [unhash $newPosition]
#    set i  [expr $theMove/10]
    
 #   set im1 [expr $i-1]
    
#    for {set frame 0} {$frame < 50} {incr frame} {
	
#	for {set j [lindex $oldl $im1]} {$j>[lindex $newl $im1]} {incr j -1} {
#	    $c move move-$i$j 0 -8
#	}
#	update idletasks
#    }
}


# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever the
# player clicks to make the move)  It is also the function that handles coloring
# of the moves according to value. It is called by gamesman just before the player
# is prompted for a move.

# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which radio button is down
# position = the current hashed position
# moveList = a list of available moves to the player.  These moves are represented as numbers (same as in C)

# The code snippet herein may be helpful but is not necessary to do it that way.
# We provide a procedure called MoveValueToColor that takes in moveType and a moveValue and returns the correct color.
# For example [MoveValueToColor "value" "Tie"] would return yellow and [MoveValueToColor "all" "anything"] would return cyan.

proc GS_ShowMoves { c moveType position moveList } {
    $c lower arrow
    #$c lower slot
#    $c create text 250 250 -text $moveList
    global pieceString Boardsize
    set pieceString [string range [C_GenericUnhash $position $Boardsize] 0 [expr $Boardsize-1]]
    #Print "calling ShowMoves w/list: $moveList"
    foreach item $moveList {
	#Print $item
	set move  [lindex $item 0]
	set value [lindex $item 1]
	set color cyan
	
	if {$moveType == "value"} {
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} {
		set color green
	    } else {
		set color red
	    }
	}
	
	set dir [expr fmod($move,10)]
	if {$move<0} {
	    set move [expr 0 - $move]
	}
	set move [expr int($move/10)]

	set slot1 [expr int(fmod($move, 100))]
	set move [expr int($move/100)]
	set slot2 [expr int(fmod($move, 100))]
	set slot3 [expr int($move/100)]
	
	if {($slot1 == 99) && ($slot2 == 99)} {
	    #one piece scenario
	    set slot1 $slot3
	    set numpieces 1
	} elseif {($slot2==99) && ($slot3==99)} {
	    #one piece scenario
	    set numpieces 1
	} elseif {$slot1 == 99} {
	    set slot1 $slot2
	    set slot2 $slot3
	    set numpieces 2
	} elseif {$slot3 == 99} {
	    set numpieces 2
	} else {
	    set numpieces 3
	}
	
	if {$dir == 1} {
	    set dirstr "E"
	} elseif {$dir == 2} {
	    set dirstr "SE"
	} elseif {$dir == 3} {
	    set dirstr "SW"
	} elseif {$dir == -1} {
	    set dirstr "W"
	} elseif {$dir == -2} {
	    set dirstr "NW"
	} else {
	    set dirstr "NE"
	}
	
	
	if {$numpieces == 1} {
	     $c itemconfig arrow$dirstr$slot1 -fill $color
	    $c raise arrow$dirstr$slot1
	    #$c create text 250 250 -text arrow$dirstr$slot1
	} elseif {$numpieces == 2} {
	    if {$slot2 == [C_Destination $slot1 $dir]} {
		#it's a push
		$c itemconfig arrow$dirstr$slot1 -fill $color
		$c raise arrow$dirstr$slot1
	    } else {
		#it's a sidestep
		#if {$dir<0} {
		    $c itemconfig arrow$dirstr$slot2$slot1 -fill $color
		    $c raise arrow$dirstr$slot2$slot1
		#} else {
		    $c itemconfig arrow$dirstr$slot1$slot2 -fill $color
		    $c raise arrow$dirstr$slot1$slot2
		#}
	    }
	} else {
	    if {$slot2 == [C_Destination $slot $dir]} {
		#it's a push
		$c itemconfig arrow$dirstr$slot -fill $color
		$c raise arrow$dirstr$slot1
	    } else {
		#it's a sidestep
	    
	    }
	}
	    
    }
    update idletasks    
}



# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.

proc GS_HideMoves { c moveType position moveList} {
    $c lower arrow
}



# GS_HandleUndo draws a move undoing itself sortof.
# the names of the arguments explain themsleves but just to clarify...
# The game was in position A, a player makes move M bringing the game to position B
# then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A

# By default this function just calls GS_DrawPosition, but you certainly don't need to keep that.

proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {
    GS_DrawPosition $c $positionAfterUndo
}



# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
proc GS_GetGameSpecificOptions { } {
}



# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# you could use this function to draw the line striking out the winning row in tic tac toe for instance
# or you could congratulate the winner or do nothing if you want.

proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove } {
}


# GS_UndoGameOver is called then the player hits undo after the game is finished.
# this is provided so that you may undo the drawing you did in GS_GameOver if you drew something.
# for instance, if you drew a line crossing out the winning row in tic tac toe, this is where you sould delete the line.

# note: GS_HandleUndo is called regardless of whether the move undoes the end of the game, so IF you choose to do nothing in
# GS_GameOver, you needn't do anything here either.

proc GS_UndoGameOver { c position } {
}
