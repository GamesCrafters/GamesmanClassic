proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    global kGameName
    set kGameName "TicTacToe"


    global kCAuthors kTclAuthors
    set kCAuthors "Dan Garcia"
    set kTclAuthors "GamesCrafters"

    ### Set the initial position of the board in our representation
    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition
    
}

proc GS_GetDefaultRules {} {
 
    

}

proc GS_NameOfPieces {} {
    return [list x o]
}

proc GS_ColorOfPlayers {} {
    return [list blue red]
}

# Setup the rules frame
# Adds widgets to the rules frame that will allow the user to 
# select the variant of this game to play. The options 
# selected by the user should be stored in a set of global
# variables. This procedure should not modify global variables
# that affect initialization or game play. Such actions should
# occur in GS_ImplementOption. 
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

    global gMisereGame
    set gMisereGame 0

    set ruleSettingGlobalNames [list "gMisereGame"]

    global kLabelFont
    set ruleset [list $standardRule]
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


# Get the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code
proc GS_GetOption { } {
    global gMisereGame
    set option 1
    set option [expr $option + $gMisereGame]
    return $option
}


# Modify the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option. This procedure should not modify any global 
# variables that affect initialization or game play. Such actions 
# should occur in GS_ImplementOption. 
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing
proc GS_SetOption { option } {
    global gMisereGame
    set option [expr $option - 1]
    set gMisereGame [expr $option%2]
}


# Implement the given game option
# Modifies the global variables used to initialize and play the game 
# to match the given option. This can include the To Win and To Move 
# strings if any option modifies them. 
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used during initialization and game play
# Returns: nothing
proc GS_ImplementOption { option } {
    set option [expr $option - 1]
    set standardOption [expr $option%2]
    
    if { $standardOption == "0" } {
	set toWin1 "To Win: "
    } elseif { $standardOption == "1" } {
	set toWin1 "To Lose: "
    }

    ### tomove and towin
    set toWin2 "Connect 3 in a row in any direction to win" 

    SetToWinString [concat $toWin1 $toWin2]

    SetToMoveString  "To Move: Click near one of the dots in an empty square. This will place a piece for you"

}


proc GS_Initialize { c } {
    #puts [format "GS_Initialize called on canvas %s" $c]
    set counter 0
    for {set j 0} {$j < 3} {set j [expr $j + 1]} {
        
        for {set i 0} {$i < 3} {set i [expr $i + 1]} {
            
            $c create rect [expr $i * 166] [expr $j * 166] [expr ($i+1) * 166] [expr ($j+1) * 166] -fill white -tag [list base base$counter]
            
            $c create oval [expr ($i+.4) * 166] [expr ($j+.4) * 166] [expr ($i+.6) * 166] [expr ($j+.6) * 166] -fill white -outline "" -tag [list moveindicators mi-$counter]
            
            $c bind base$counter <ButtonRelease-1> "ReturnFromHumanMove $counter"
            
            $c bind mi-$counter <ButtonRelease-1> "ReturnFromHumanMove $counter"
            incr counter
        }
    }
    
    $c create line 166 0 166 500 -width 2 -tag base
    $c create line 332 0 332 500 -width 2 -tag base
    $c create line 0 166 500 166 -width 2 -tag base
    $c create line 0 332 500 332 -width 2 -tag base
    MakePieces $c 0
    
    $c raise base
    DrawSplash $c
    update idletasks
}

proc GS_Deinitialize { c } {
    $c delete all
}

proc DrawSplash { c } {
    $c create line 33 33 133 33 -width 10 -fill red
    $c create line 83 33 83 133 -width 10 -fill red
}

proc MakePieces { c num } {
    MakeX $c [expr $num % 3] [expr $num / 3] $num
    MakeO $c [expr $num % 3] [expr $num / 3] $num
    if { $num != 8 } {
        MakePieces $c [expr $num + 1]
    }
}

proc MakeX { c x y tag } {
    set x [expr $x * 166]
    set y [expr $y * 166]
    $c create line [expr $x  + 10] [expr $y + 10] [expr $x + 156] [expr $y + 156] -width 10 -fill blue -tag x-$tag
    $c create line [expr $x  + 10] [expr $y + 156] [expr $x + 156] [expr $y + 10] -width 10 -fill blue -tag x-$tag
    $c lower x-$tag base 
}

proc MakeO { c x y tag } {
    set x [expr $x * 166]
    set y [expr $y * 166]
    $c create oval [expr $x  + 10] [expr $y + 10] [expr $x + 156] [expr $y + 156] -width 10 -outline red -tag o-$tag
    $c lower x-$tag base
}

proc GS_NewGame { c position } {
    GS_DrawPosition $c $position
}

proc GS_DrawPosition { c position } {
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
    for {set i 0} {$i < 9} {set i [expr $i + 1]} {
        
        if {[expr $position % 3] == 1} {   
            set a($i) x
        } elseif {[expr $position %3 == 2]} {
            set a($i) o
        } else {
            set a($i) -
        }
        set position [expr $position / 3]
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

proc GS_HandleMove { c oldPosition theMove newPosition } {
    set piece x
    if { [GS_WhoseMove $oldPosition] == "o"} {
        set piece o
    }
    $c raise $piece-$theMove base
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
                set color red
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
    
    UnHashBoard $position a
    
    if { $gameValue != "Tie" } {
                
        if {$a(0) == $a(1) && $a(1) == $a(2)} {set p {0 2}}
        if {$a(3) == $a(4) && $a(4) == $a(5)} {set p {3 5}}
        if {$a(6) == $a(7) && $a(7) == $a(8)} {set p {6 8}}
        if {$a(0) == $a(3) && $a(3) == $a(6)} {set p {0 6}}
        if {$a(1) == $a(4) && $a(4) == $a(7)} {set p {1 7}}
        if {$a(2) == $a(5) && $a(5) == $a(8)} {set p {2 8}}
        if {$a(0) == $a(4) && $a(4) == $a(8)} {set p {0 8}}
        if {$a(2) == $a(4) && $a(4) == $a(6)} {set p {2 6}}
                
        set fromx [expr ([lindex $p 0] % 3 + .5)*166]
        set fromy [expr ([lindex $p 0] / 3 + .5)*166]
        set tox [expr ([lindex $p 1] % 3 + .5)*166]
        set toy [expr ([lindex $p 1] / 3 + .5)*166]
        
        $c create line $fromx $fromy $tox $toy -width 15 -fill black -tag bigline
    }
}

proc GS_UndoGameOver { c position } {
    $c delete bigline    
}

proc GS_HandleUndo { c currentPosition theMoveToReverse positionAfterUndo} {
    set piece x
    if { [GS_WhoseMove $positionAfterUndo] == "o"} {
        set piece o
    }
    $c lower $piece-$theMoveToReverse base
}

proc GS_GetGameSpecificOptions { } {
}

proc GS_Deinitialize { c } {   
}
