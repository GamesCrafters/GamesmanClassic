### Drawing in a Canvas
###  (draw with mousebutton 1)

#canvas .c1
#pack .c1 -fill both -expand yes

#bind .c1 <B1-Motion> ".c1 create rectangle %x %y %x %y -width 5"









proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    global kGameName
    set kGameName "Tic-Tac-Toe"

    ### Set the initial position of the board in our representation
    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### tomove and towin
    global gMisereGame
    if { $gMisereGame } {
	set toWin1 "To Lose: "
    } else {
	set toWin1 "To Win: "
    }

    set toWin2 "Connect 3 in a row in any direction to win" 

    SetToWinString [concat $toWin1 $toWin2]

    SetToMoveString  "To Move: Click near one of the dots in an empty square. This will place a piece for you"

    ############   GLOBAL VARS #######################

    global dotgap dotmid
    global x1 x2 x3 y1 y2 y3
    global pieceSize pieceOutline xColor oColor pieceOffset
    global dotSize dotExpandAmount lineWidth lineColor baseColor base
    global dotx1 dotx2 dotx3 doty1 doty2 doty3
    global px1 px2 px3 py1 py2 py3
    global diagArrows horizArrows vertArrows slideDelay goDelay animQuality
    global gFrameWidth gFrameHeight
    global boardSize squareSize leftBuffer topBuffer
    # Authors Info
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors

    set kCAuthors "Jesse Phillips, Jennifer Lee"
    set kTclAuthors "Jesse Phillips, Jennifer Lee"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    ## IMPORTANT: These are variables used to change the board.
    # board size
    set boardSize [min $gFrameWidth $gFrameHeight]
    set squareSize [expr $boardSize / 3]
    set leftBuffer [expr [expr $gFrameWidth - $boardSize] / 2]
    set topBuffer [expr [expr $gFrameHeight - $boardSize] / 2]
    #coordinates:
    set dotgap [expr $boardSize / 3] 
    set dotmid [expr $dotgap / 2]
    set firstXCoord [expr $leftBuffer + $dotmid]
    set firstYCoord [expr $topBuffer + $dotmid]
    ## pieces
    set pieceSize [expr $dotmid - ($dotmid / 4)]
    set pieceOutline [expr $pieceSize / 14]
    set xColor blue4
    set oColor red2
    set pieceOffset $dotgap
    ## dots
    set dotSize [expr $pieceSize / 2.6]
    set dotExpandAmount [expr 3 * ( $dotSize / 4 )]
    ## lines
    set lineWidth [ expr 3 * ( $pieceOutline / 2 ) ]
    set lineColor CadetBlue4
    ## base
    set baseColor white
    ## arrow lists
    set diagArrows  {list 15 51 26 62 48 84 59 95 24 42 35 53 57 75 68 86}
    set horizArrows {list 12 21 23 32 45 54 56 65 78 87 89 98}
    set vertArrows  {list 14 41 25 52 36 63 47 74 58 85 69 96}
    ## animation delay
    set slideDelay 20000
    set goDelay 3000000
    set animQuality "low"
    # x and y position numbers
    set x1 $firstXCoord
    set x2 [expr $x1 + $dotgap]
    set x3 [expr $x2 + $dotgap]
    set x4 $x1
    set x5 $x2
    set x6 $x3
    set x7 $x1
    set x8 $x2
    set x9 $x3
    
    set y1 $firstYCoord
    set y2 $y1
    set y3 $y1
    set y4 [expr $y1 + $dotgap]
    set y5 $y4
    set y6 $y4
    set y7 [expr $y4 + $dotgap]
    set y8 $y7
    set y9 $y7
        
    set dotx1 [expr $x1 - [expr $dotSize / 2]];
    set dotx2 [expr $x2 - [expr $dotSize / 2]];
    set dotx3 [expr $x3 - [expr $dotSize / 2]];
    set doty1 [expr $y1 - [expr $dotSize / 2]];
    set doty2 [expr $y4 - [expr $dotSize / 2]];
    set doty3 [expr $y7 - [expr $dotSize / 2]];
    
    set px1 [expr $x1 - [expr $pieceSize / 2]];
    set px2 [expr $x2 - [expr $pieceSize / 2]];
    set px3 [expr $x3 - [expr $pieceSize / 2]];
    set py1 [expr $y1 - [expr $pieceSize / 2]];
    set py2 [expr $y4 - [expr $pieceSize / 2]];
    set py3 [expr $y7 - [expr $pieceSize / 2]];
    
    
}

proc GS_NameOfPieces {} {
    return [list x o]
}

proc GS_ColorOfPlayers {} {
    return [list blue red]
}

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

proc GS_GetOption { } {
    global gMisereGame
    if { $gMisereGame == 0 } {
        set option 2
    } else {
        set option 1
    }
    return $option
}

proc GS_SetOption { option } {
    global gMisereGame
    if { $option == 1 } {
        set gMisereGame 1
    } else {
        set gMisereGame 0
    }
}

proc min { num1 num2 } {
    if { $num1 < $num2 } {
	return $num1
    } else {
	return $num2
    }
}

proc GS_Initialize { c } {
    #puts [format "GS_Initialize called on canvas %s" $c]
    global boardSize squareSize leftBuffer topBuffer

    set counter 0
    for {set j 0} {$j < 3} {set j [expr $j + 1]} {
        
        for {set i 0} {$i < 3} {set i [expr $i + 1]} {
            
#            $c create rect [expr $i * 166] [expr $j * 166] [expr ($i+1) * 166] [expr ($j+1) * 166] -fill white -tag [list base base$counter]
            $c create rect \
		[expr $leftBuffer + [expr $i * $squareSize]] \
		[expr $topBuffer + [expr $j * $squareSize]] \
		[expr $leftBuffer + [expr ($i+1) * $squareSize]] \
		[expr $topBuffer + [expr ($j+1) * $squareSize]] \
		-fill white -tag [list base base$counter]
            
#            $c create oval [expr ($i+.4) * 166] [expr ($j+.4) * 166] [expr ($i+.6) * 166] [expr ($j+.6) * 166] -fill white -outline "" -tag [list moveindicators mi-$counter]
            $c create oval \
		[expr $leftBuffer + [expr ($i+.4) * $squareSize]] \
		[expr $topBuffer + [expr ($j+.4) * $squareSize]] \
		[expr $leftBuffer + [expr ($i+.6) * $squareSize]] \
		[expr $topBuffer + [expr ($j+.6) * $squareSize]] \
		-fill white -outline "" -tag [list moveindicators mi-$counter]
            
            $c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove $counter"
            
            $c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove $counter"
            incr counter
        }
    }
    

    #left-mid vertical border
#    $c create line 166 0 166 500 -width 2 -tag base
    $c create line [expr $leftBuffer + $squareSize] $topBuffer [expr $leftBuffer + $squareSize] [expr $topBuffer + $boardSize] -width 2 -tag base

    #right-mid vertical border
    $c create line [expr $leftBuffer + [expr 2 * $squareSize]] $topBuffer [expr $leftBuffer + [expr 2 * $squareSize]] [expr $topBuffer + $boardSize] -width 2 -tag base
    

    #top-mid horizontal border
#    $c create line 0 166 500 166 -width 2 -tag base
    $c create line $leftBuffer [expr $topBuffer + $squareSize] [expr $leftBuffer + $boardSize] [expr $topBuffer + $squareSize] -width 2 -tag base
    
    
    #bottom-mid horizontal border
#    $c create line 0 332 500 332 -width 2 -tag base
    $c create line $leftBuffer [expr $topBuffer + [expr 2 * $squareSize]] [expr $leftBuffer + $boardSize] [expr $topBuffer + [expr 2 * $squareSize]] -width 2 -tag base

    MakePieces $c 0
    
    $c raise base
    NameFlash $c
    update idletasks
   
}

proc GS_Deinitialize { c } {
    $c delete all
}


# fancy flashing of letters of the game (spelling the name of the game)
proc NameFlash { c } {
    global dotmid leftBuffer topBuffer

    #"T" in top left sq
    #$c create line 33 33 133 33 -width 10 -fill red
    #$c create line 83 33 83 133 -width 10 -fill red

    #"TIC"
    $c create text [expr 1 * $dotmid + $leftBuffer] [expr 1 * $dotmid + $topBuffer] -text "T" -font {Helvetica 100} -fill red -tags {tic ttt}
    $c create text [expr 3 * $dotmid + $leftBuffer] [expr 1 * $dotmid + $topBuffer] -text "I" -font {Helvetica 100} -fill red -tags {tic ttt}
    $c create text [expr 5 * $dotmid + $leftBuffer] [expr 1 * $dotmid + $topBuffer] -text "C" -font {Helvetica 100} -fill red -tags {tic ttt}
    
    #"TAC"
    $c create text [expr 1 * $dotmid + $leftBuffer] [expr 3 * $dotmid + $topBuffer] -text "T" -font {Helvetica 100} -fill green -tags {tac ttt}
    $c create text [expr 3 * $dotmid + $leftBuffer] [expr 3 * $dotmid + $topBuffer] -text "A" -font {Helvetica 100} -fill green -tags {tac ttt}
    $c create text [expr 5 * $dotmid + $leftBuffer] [expr 3 * $dotmid + $topBuffer] -text "C" -font {Helvetica 100} -fill green -tags {tac ttt}


    #"TOE"
    $c create text [expr 1 * $dotmid + $leftBuffer] [expr 5 * $dotmid + $topBuffer] -text "T" -font {Helvetica 150} -fill blue -tags {toe ttt}
    $c create text [expr 2.2 * $dotmid + $leftBuffer] [expr 5 * $dotmid + $topBuffer] -text "I" -font {Helvetica 150} -fill blue -tags {toe ttt}
    $c create text [expr 3.4 * $dotmid + $leftBuffer] [expr 5 * $dotmid + $topBuffer] -text "E" -font {Helvetica 150} -fill blue -tags {toe ttt}
    $c create text [expr 5 * $dotmid + $leftBuffer] [expr 5 * $dotmid + $topBuffer] -text "R" -font {Helvetica 150} -fill blue -tags {toe ttt}

    $c lower ttt base

    #flash the letters row at a time, then show all the letters
    set flash_time 90000

    $c raise tic
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }
    $c lower tic base
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }
    
    $c raise tac
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }
    $c lower tac base
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }

    $c raise toe
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }
    $c lower toe base
    update idletasks
    set next_time [expr [clock clicks] + $flash_time]
    while {$next_time > [clock clicks]} {
	# spin wait
    }

    $c raise ttt



}
global coordList 
set coordList {}


proc MakePieces { c num } {
  
    MakeX $c [expr $num % 3] [expr $num / 3] $num
    MakeO $c [expr $num % 3] [expr $num / 3] $num

    if { $num != 8 } {
        MakePieces $c [expr $num + 1]
    }
}


#Changed to make it look more like lite-3 - Jeffrey Chiang
proc MakeX { c x y tag } {
    global squareSize leftBuffer topBuffer

    #old way
    
     set x [expr $x * $squareSize + $leftBuffer]
     set y [expr $y * $squareSize + $topBuffer]
 
     #full X
    #$c create line [expr $x  + 10] [expr $y + 10] [expr $x + 156] [expr $y + 156] -width 10 -fill blue -capstyle round -tag x-$tag 
    #$c create line [expr $x  + 10] [expr $y + 156] [expr $x + 156] [expr $y + 10] -width 10 -fill blue -capstyle round -tag x-$tag
    #$c lower x-$tag base

     #X dots
    $c create line [expr $x  + 10] [expr $y + 10] [expr $x + 10] [expr $y + 10] -width 24 -fill blue -tag x-$tag-1 
    $c create line [expr $x  + [expr $squareSize - 10]] [expr $y + 10] [expr $x + [expr $squareSize - 10]] [expr $y + 10] -width 24 -fill blue -tag x-$tag-2
    $c lower x-$tag-1 base
    $c lower x-$tag-2 base

    #new way
### Drawing in a Canvas
###  (draw with mousebutton 1)

#bind $c <B1-Motion> "$c create rectangle  %x %y %x %y -outline blue  -width 10"
    
}

# Changed to make it look more like Lite-3 - Jeffrey Chiang
proc MakeO { c x y tag } {
    global squareSize leftBuffer topBuffer

    set x [expr $x * $squareSize + $leftBuffer]
    set y [expr $y * $squareSize + $topBuffer]

# for the red dot in the top corner of the square
#    $c create oval $x $y [expr $x + 1] [expr $y + 1] -width 10 -outline red -tag o-$tag

    $c create oval [expr $x  + 20] [expr $y + 20] [expr $x + [expr $squareSize - 20]] [expr $y + [expr $squareSize - 20]] -width 16 -outline red -tag actual-o-$tag
    $c create arc [expr $x  + 20] [expr $y + 20] [expr $x + [expr $squareSize - 20]] [expr $y + [expr $squareSize - 20]] -start 90 -extent 0 -style arc -width 16 -outline red -tag o-$tag
    $c lower o-$tag base

  #new way
### Drawing in a Canvas
###  (draw with mousebutton 1)



#bind $c <B1-Motion> "$c create rectangle %x %y %x %y -outline red -width 10"




}

proc GS_NewGame { c position } {
    GS_DrawPosition $c $position
}

proc GS_DrawPosition { c position } {
    global dotmid leftBuffer topBuffer
    $c raise base
    DrawPieces $c $position
}

proc DrawPieces {c position } {
    set a(0) 0
    
    UnHashBoard $position a
    
    for {set i 0} {$i < 9} {incr i} {   
        if {$a($i) == "x"} {
            $c raise x-$i base
        } elseif {$a($i) == "o"} {
            $c raise o-$i base
        } 
    }
}

proc UnHashBoard {position arrayname} {
    upvar $arrayname a
    
    set board [C_CustomUnhash $position]
    
    for {set i 0} {$i < 9} {incr i} {
        if {[string equal [string index $board $i] "X"]} {   
            set a($i) x
        } elseif {[string equal [string index $board $i] "O"]} {
            set a($i) o
        } else {
            set a($i) -
        }
    }
    
}

proc GS_WhoseMove { position } {
 
    set a(0) 0
    
    UnHashBoard $position a
    set counter 0
    for {set i 0} {$i < 9} {incr i} {
        if {$a($i) != "-" } {
            incr counter
        }
    }
    set val o
    if { [expr $counter % 2] == 0 } {
        set val x
    }
    return $val
    
}

#proc drawX {





proc DrawX { c x y positionNum } {
      


    for {set i 10} {$i < 157} {incr i 1} {
       

        $c create rectangle [expr $x + $i] [expr $y + $i] [expr $x + $i] [expr $y + $i] -outline blue  -width 10 -fill blue  -tag x-$positionNum
	after 0
	update idletasks

     
     }
    for {set i 10} {$i < 157} {incr i 1} {
       $c create rectangle [expr $x + $i] [expr $y + 166 - $i] [expr $x + $i] [expr $y + 166 - $i] -outline blue -width 10 -fill blue  -tag  x-$positionNum
	 after 0 
	 update idletasks
     }

}

# Changed this to make it look more like Lite-3  - Jeffrey Chiang
proc stretchX1 { c x y tag } {
    global squareSize gAnimationSpeed
    set animSpeed [expr 5 * [expr $gAnimationSpeed + 6]]
    for {set i 30} {$i < [expr $squareSize - 30]} {incr i $animSpeed} {
       $c coords $tag [expr $x + 30] [expr $y + 30] [expr $x + $i] [expr $y + $i]
	 after 0 
	 update idletasks
     }
    $c coords $tag [expr $x + 30] [expr $y + 30] [expr $x + $squareSize - 30] [expr $y + $squareSize - 30]
}

# Changed this to make it look more like Lite-3  - Jeffrey Chiang
proc stretchX2 { c x y tag } {
    global squareSize gAnimationSpeed
    set animSpeed [expr 5 * [expr $gAnimationSpeed + 6]]
    for {set i 30} {$i < [expr $squareSize - 30]} {incr i $animSpeed} {
	$c coords $tag [expr $x + [expr $squareSize - 30]] [expr $y + 30] [expr $x + $squareSize - $i] [expr $y + $i]
	 after 0 
	 update idletasks
     }
    $c coords $tag [expr $x + [expr $squareSize - 30]] [expr $y + 30] [expr $x + 30] [expr $y + $squareSize - 30]
}

proc stretchO { c tag } {
    global gAnimationSpeed
    set animSpeed [expr 5 * [expr $gAnimationSpeed + 6]]
    for {set i 0} {$i < 360} {incr i $animSpeed} {
	$c itemconfigure $tag -extent [expr 0 - $i]
	after 0
	update idletasks
    }

}

proc GS_HandleMove { c oldPosition theMove newPosition } {
    global squareSize leftBuffer topBuffer

    set x [expr [expr $theMove % 3] * $squareSize + $leftBuffer]
    set y [expr [expr $theMove / 3] * $squareSize + $topBuffer]

    #for getting position in which to draw the piece

   
    set piece x
   #  bind $c <B1-Motion> "$c create rectangle  %x %y %x %y -outline blue  -width 10" 
    if { [GS_WhoseMove $oldPosition] == "o"} {
        set piece o
# bind $c <B1-Motion> "$c create rectangle  %x %y %x %y -outline red  -width 10" 
	$c raise $piece-$theMove base
	stretchO $c $piece-$theMove
	$c raise actual-$piece-$theMove base	
	update idletasks
    }
    
       

if { [GS_WhoseMove $oldPosition] == "x"} {

 #   DrawX $c $xc $yc $theMove
    $c raise $piece-$theMove-1 base
    
    stretchX1 $c $x $y $piece-$theMove-1
   
    $c raise $piece-$theMove-2 base

    stretchX2 $c $x $y $piece-$theMove-2
    
}
   
    
#     test the do-update proc
#	do-update $c $piece $theMove 100
 


}


	proc do-update { c piece theMove loops }  {
	    for {set i 0} {$i < $loops } {incr i 1} {
		
		$c move $piece-$theMove 1 1
		after 10 
		update idletasks
	    }
	}



proc GS_ShowMoves { c moveType position moveList } {
    
    foreach item $moveList {
        set move  [lindex $item 0]
        set value [lindex $item 1]
        set color cyan
        
        $c raise mi-$move base
        
        if {$moveType == "value"} {
            if {$value == "Tie"} {
                set color yellow
            } elseif {$value == "Lose"} {
                set color green
            } else {
                set color red4
            }
        }
        $c itemconfigure mi-$move -fill $color
    }
    update idletasks
}

proc GS_HideMoves { c moveType position moveList} {
    $c lower moveindicators base
}

proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove } {
    global squareSize leftBuffer topBuffer

    UnHashBoard $position a
    
    if { $gameValue != "Tie" } {
                
        if {$a(0) != "-" && $a(0) == $a(1) && $a(1) == $a(2)} {set p {0 2}}
        if {$a(3) != "-" && $a(3) == $a(4) && $a(4) == $a(5)} {set p {3 5}}
        if {$a(6) != "-" && $a(6) == $a(7) && $a(7) == $a(8)} {set p {6 8}}
        if {$a(0) != "-" && $a(0) == $a(3) && $a(3) == $a(6)} {set p {0 6}}
        if {$a(1) != "-" && $a(1) == $a(4) && $a(4) == $a(7)} {set p {1 7}}
        if {$a(2) != "-" && $a(2) == $a(5) && $a(5) == $a(8)} {set p {2 8}}
        if {$a(0) != "-" && $a(0) == $a(4) && $a(4) == $a(8)} {set p {0 8}}
        if {$a(2) != "-" && $a(2) == $a(4) && $a(4) == $a(6)} {set p {2 6}}
                
        set fromx [expr ([lindex $p 0] % 3 + .5)*$squareSize + $leftBuffer]
        set fromy [expr ([lindex $p 0] / 3 + .5)*$squareSize + $topBuffer]
        set tox [expr ([lindex $p 1] % 3 + .5)*$squareSize + $leftBuffer]
        set toy [expr ([lindex $p 1] / 3 + .5)*$squareSize + $topBuffer]
        
        $c create line $fromx $fromy $tox $toy -width 15 -fill black -tag bigline -capstyle round
		update idletasks
    }
}

proc GS_UndoGameOver { c position } {
    $c delete bigline    
}

proc GS_HandleUndo { c currentPosition theMoveToReverse positionAfterUndo} {
    set piece x
    if { [GS_WhoseMove $positionAfterUndo] == "o"} {
        set piece o
	$c lower $piece-$theMoveToReverse base
	$c lower actual-$piece-$theMoveToReverse base
    }
    if { [GS_WhoseMove $positionAfterUndo] == "x"} {
	$c lower $piece-$theMoveToReverse-1 base
	$c lower $piece-$theMoveToReverse-2 base
    }
}

proc GS_GetGameSpecificOptions { } {
}

proc GS_Deinitialize { c } {   
}

