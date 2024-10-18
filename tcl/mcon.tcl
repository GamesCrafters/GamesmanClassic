####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
# Updated Fall 2004 by Jeffrey Chiang, and others
####################################################

#############################################################################
# GS_InitGameSpecific sets characteristics of the game that
# are inherent to the game, unalterable.  You can use this fucntion
# to initialize data structures, but not to present any graphics.
# It is called FIRST, ONCE, only when the player
# starts playing your game, and before the player hits "New Game"
# At the very least, you must set the global variables kGameName
# and gInitialPosition in this function.
############################################################################
proc GS_InitGameSpecific {} {
    
    ### Set the name of the game
    
    global kGameName
    set kGameName "Connections"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to connect a path from top to bottom, bottom to top, left to right, or right to left WINS"
    set kMisereString "First player to force the other player to connect a path from top to bottom, bottom to top, left to right, or right to left LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    #global gMisereGame
    #if {!$gMisereGame} {
	#SetToWinString "To Win: (fill in)"
    #} else {
	#SetToWinString "To Win: (fill in)"
    #}
    #SetToMoveString "To Move: Move mouse over to a highlighted box and click to initiate move."

        # Set toMove toWin
    global gMisereGame
    set gMisereGame 0
    if { $gMisereGame } {
	set toWin1 "To Lose: Other player has made a connection from one side to another, either from top to bottom or left to right"
    } else {
	set toWin1 "To Win: Make a continuous path in your color from top to bottom, bottom to top, left to right, or right to left"
    }

    set toWin2 ""

    SetToWinString [concat $toWin1 $toWin2]

    SetToMoveString  "To Move: Move mouse over to a highlighted box and click to initiate move. With every adjacent box your color, a path will be created between them"
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors " Brian Carnes"
    set kTclAuthors "Ly, Minh Van (Larry)"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"
}


#############################################################################
# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"
#############################################################################
proc GS_NameOfPieces {} {

    ### FILL THIS IN
    return [list left right top bottom]

}


#############################################################################
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
#############################################################################
proc GS_ColorOfPlayers {} {

	### FILL THIS IN
    return [list blue red]
    
}


#############################################################################
# GS_SetupRulesFrame sets up the rules frame;
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
#############################################################################
proc GS_SetupRulesFrame { rulesFrame } {

    set standardRule \
	[list \
	     "What would you like your winning condition to be:" \
	     "Standard" \
	     "Misere" \
	     ] 
    set moveType \
	[list \
	     "What type of animation do you want:" \
	     "Big" \
	     "Small" \
	     ]

    # List of all rules, in some order
    set ruleset [list $standardRule $moveType]

    # Declare and initialize rule globals
    global gMisereGame
    set gMisereGame 0

    global gAnimateType
    set gAnimateType 1

    # List of all rule globals, in same order as rule list
    set ruleSettingGlobalNames [list "gMisereGame" "gAnimateType"]

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
# GS_GetOption gets the game option specified by the rules frame
# Returns the option of the variant of the game specified by the 
# global variables used by the rules frame
# Args: none
# Modifies: nothing
# Returns: option (Integer) - the option of the game as specified by 
# getOption and setOption in the module's C code
#############################################################################
proc GS_GetOption { } {
    # TODO: Needs to change with more variants
    global gMisereGame
    set option 1
    set option [expr $option + (1-$gMisereGame)]
    return $option
}


#############################################################################
# GS_SetOption modifies the rules frame to match the given options
# Modifies the global variables used by the rules frame to match the 
# given game option.
# This procedure only needs to support options that can be selected 
# using the rules frame.
# Args: option (Integer) -  the option of the game as specified by 
# getOption and setOption in the module's C code
# Modifies: the global variables used by the rules frame
# Returns: nothing
#############################################################################
proc GS_SetOption { option } {
    # TODO: Needs to change with more variants
    global gMisereGame
    set option [expr $option - 1]
    set gMisereGame [expr 1-($option%2)]
}


#############################################################################
# GS_Initialize is where you can start drawing graphics.  
# Its argument, c, is a canvas.  Please draw only in this canvas.
# You could put an opening animation in this function that introduces the game
# or just draw an empty board.
# This function is called ONCE after GS_InitGameSpecific, and before the
# player hits "New Game"
#############################################################################
proc GS_Initialize { c } {

    ### TODO: fill this in
    global margin gFrameWidth gFrameHeight boardSize maxSize color1 color2 IndivSquSize boardMoves definition gAnimateType

    #set the default animate type
    #set gAnimateType 0
   
    #how detail you want the animation to be 
    set definition 150

    #change this if board is board is bigger
    set boardSize 5

    #sets the intial possible move board used later to highlight possible moves, we can
    #change this later if we start with a different board.
    set boardMoves [generateBoardMoves $boardSize]
    
    set margin 10

    set maxSize [min $gFrameWidth $gFrameHeight]

    #change this to change the color of the board
    set color1 #2E2E38
    set color2 #2E2E38

    set IndivSquSize [expr $maxSize/$boardSize]

    #Draw the Board
    drawBoard $c
    
    #GS_ShowMoves $c "value" [generateBoardMoves $boardSize] [list 0 1 2 3 4 6 7 9 10 11 13 14 16 17 18 19 20]

    #animateFullMove $c 1 3 red "vert"
    #animateFullMove $c 3 3 magenta "hori"
    #animateSquMove $c 2 1 purple
    #animateLine $c 1 1 yellow "vert"
    #animateLine $c 4 3 yellow "hori"
    #animateSingleBox $c 0 0 green "hori" "right"
    #animateSingleBox $c 4 4 green "hori" "left"
    #animateSingleBox $c 1 2 green "vert" "top"
    #animateSingleBox $c 3 1 green "vert" "bottom"

} 


#############################################################################
# move = the move number
# this proc returns the zeroth position that that moves cooresponds to
#############################################################################
proc mapMove { move } {
    
    global boardSize
    
    set firstLine [numOfSpotsLine "firstLast"]
    
    if { $move < $firstLine } {
	return [expr 1 + $move]
    } elseif { $move >= [totalNumOfZeros $boardSize] - 4 - $firstLine } {
	return [expr 3 + $move]
    } else {
	return [expr 2 + $move]
    }
}


#############################################################################
# size = boardSize
# this proc returns the total number of available zeros in the inital empty board
#############################################################################
proc totalNumOfZeros { size } {
    return [expr (($size * $size) / 2) + 1]
}


#############################################################################
# size = this is just really boardSize, but we want to keep as general as possible
# This procedure generates the boardMoves variable needed for the animation 
# given a size
#############################################################################
proc generateBoardMoves { size } {
    if { [expr $size % 2] != 1 } {
	puts "error, boardSize is not odd"
    } else {
	set returnString [string repeat "0" [expr $size * $size]]
	set copyString1 "10"
	set copyString2 "02"

	set stringTemp1 [string repeat "0" $size]
	set stringTemp2 [string repeat "0" $size]
	set stringTemp2 [string replace $stringTemp2 0 0 "2"]
	for { set j 0 } { $j < [expr ($size - 1) / 2] } {incr j 1} {
	    set stringTemp1 [string replace $stringTemp1 [expr ($j * 2) + 1] [expr ($j * 2) + 2] $copyString1]
	    set stringTemp2 [string replace $stringTemp2 [expr ($j * 2) + 1] [expr ($j * 2) + 2] $copyString2]
	}

	for { set i 0 } { $i < [expr ($size - 1) / 2] } {incr i 1} {
	    set returnString [string replace $returnString [expr ($i * 2 * $size)] [expr ($i * 2 * $size) + $size] $stringTemp1]
	    set returnString [string replace $returnString [expr ($i * 2 * $size) + $size] [expr ($i * 2 * $size) + (2 * $size)] $stringTemp2]
	}
	set returnString [string replace $returnString [expr $size * ($size - 1)] [expr ($size * $size) - 1] $stringTemp1]
    }
    return $returnString
}


############################################################################
# zeroSpots = hard to explain
# this method returns the coordinate that that zeros maps to and returns it in list form
############################################################################
proc zerosToCoor { zeroSpots } {
    
    global boardSize

    set oddLine [numOfSpotsLine "odd"]

    set block [expr $zeroSpots / $boardSize]
    
    set temp [expr ($zeroSpots % $boardSize)]

    set lineNum [expr ($temp / $oddLine) + $block * 2]
    
    #test if first or second in each block
    if { [expr $temp / $oddLine] == 1 } {
	set i [expr $temp - $oddLine]
	set yCoor [expr ($i*2) + 1]
    } else {
	set yCoor [expr $temp * 2]
    }

    return [list $lineNum $yCoor]
    
}


############################################################################
# type = the line type you 
# This procedure returns the number of available spots in the first and last line
# or the odd or the even
############################################################################
proc numOfSpotsLine { type } {
    global boardSize
    
    if { $type == "firstLast" } {
	return [expr ((($boardSize - 1) / 2) + 1) - 2]
    } elseif { $type == "odd" } {
	return [expr ((($boardSize - 1) / 2) + 1)]
    } else {
	return [expr ((($boardSize - 1) / 2))]
    }
}


############################################################################
# c = canvas
# the other variable used are global
# This method draws the board without moves on them
############################################################################
proc drawBoard { c } {

    global boardSize IndivSquSize color1 color2

    for { set i 0 } { $i < $boardSize } {incr i 1} {
	 if { [expr $i % 2] == 0 } {
	    drawBlockSlice $c $color1 $color2 $boardSize 0 [expr $IndivSquSize * $i] $IndivSquSize $i
	} else {
	    drawBlockSlice $c $color2 $color1 $boardSize 0 [expr $IndivSquSize * $i] $IndivSquSize $i
	}
    }

}


############################################################################
# c = canvas
# colorOne = the first square color
# colorTwo = the second square color
# *these color will later will alternate in color
# numOfSquare = number of square for this block slice
# XStart = initial upper left position (x position)
# YStart = initial upper left position (y position)
# verticalSize = thickness of the square in the y-axis direction
# IndivSquSize = the size of each square
# lineNum = the line number that you are drawing
# this procedure draws horizontally the alternating color squares for one 
# slice starting at the verticalStart position and drawing square that are 
# verticalSize in thickness
############################################################################
proc drawBlockSlice { c colorOne colorTwo numOfSquare XStart YStart IndivSquSize lineNum} {
    for { set i 0 } { $i < $numOfSquare } {incr i 1} {
	if { [expr $i % 2] == 0 } {
	    $c create rect \
		[expr $i*$IndivSquSize + $XStart] \
		$YStart \
		[expr ($i+1)*$IndivSquSize + $XStart] \
		[expr $IndivSquSize + $YStart] \
		-fill $colorOne \
		-outline $colorOne
	    if { [expr $lineNum % 2 ] != 0 } {
		$c create rect \
		    [expr $i*$IndivSquSize + $XStart + ($IndivSquSize * 0.25)] \
		    [expr $YStart + ($IndivSquSize * 0.25)] \
		    [expr ($i+1)*$IndivSquSize + $XStart - ($IndivSquSize * 0.25)] \
		    [expr $IndivSquSize + $YStart - ($IndivSquSize * 0.25)] \
		    -fill [lindex [GS_ColorOfPlayers] 1]
	    }
	} else {
	    $c create rect \
		[expr $i*$IndivSquSize + $XStart] \
		$YStart \
		[expr ($i+1)*$IndivSquSize + $XStart] \
		[expr $IndivSquSize + $YStart] \
		-fill $colorTwo \
		-outline $colorOne
	    if { [expr $lineNum % 2 ] == 0 } {
		$c create rect \
		    [expr $i*$IndivSquSize + $XStart + ($IndivSquSize * 0.25)] \
		    [expr $YStart + ($IndivSquSize * 0.25)] \
		    [expr ($i+1)*$IndivSquSize + $XStart - ($IndivSquSize * 0.25)] \
		    [expr $IndivSquSize + $YStart - ($IndivSquSize * 0.25)] \
		    -fill [lindex [GS_ColorOfPlayers] 0]
	    }
	} 
    }
}


############################################################################
# take in a position and unhashes the position and
# return the current representation of the board in string form
############################################################################
proc unhash { position } {
    global boardSize

    set numOfMoves [expr [totalNumOfZeros $boardSize] - 4]
    set board [string repeat "0" $numOfMoves]

    for { set i 0 } { $i < $numOfMoves } { incr i 1} {
	if { [expr ($position >> ($i * 2)) & 3] == 1 } {
	    set board [string replace $board $i $i "1"]
	} elseif { [expr ($position >> ($i * 2)) & 3] == 0 } {
	    set board [string replace $board $i $i "0"] 
	} else {
	    set board [string replace $board $i $i "2"]
	}
    }
    return $board
}


############################################################################
# unhashString = the string after running the unhash method
# This procedure will undate the master boardMoves variable and return it too
############################################################################
proc updateBoardMoves { board } {
    global boardMoves boardSize
    
    set numOfMoves [expr [totalNumOfZeros $boardSize] - 4]

    for { set i 0 } { $i < $numOfMoves } { incr i 1} {
	set tempCoor [zerosToCoor [mapMove $i]]
	set posString [posInString [lindex $tempCoor 0] [lindex $tempCoor 1]]
	
	set boardMoves [string replace $boardMoves $posString $posString [string index $board $i]]
    }
    return $boardMoves
}   


############################################################################
# x = position of row in the board
# y = position of col in the board
# returns the position in the string of the place of that x y coordinate
############################################################################
proc posInString { x y } {
    global boardSize

    return [expr [expr $x * $boardSize] + $y]
}


############################################################################
# returns the minimum of valueOne and valueTwo
############################################################################
proc min { valueOne valueTwo } {
    if { $valueOne < $valueTwo } {
	return $valueOne
    }
    return $valueTwo
}


#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why this
# is here, so whoever put this here should update this.  -Jeff
#############################################################################
proc GS_Deinitialize { c } {
    $c delete all
}


#############################################################################
# GS_DrawPosition this draws the board in an arbitrary position.
# It's arguments are a canvas, c, where you should draw and the
# (hashed) position.  For example, if your game is a rearranger,
# here is where you would draw pieces on the board in their correct positions.
# Imagine that this function is called when the player
# loads a saved game, and you must quickly return the board to its saved
# state.  It probably shouldn't animate, but it can if you want.
#
# BY THE WAY: Before you go any further, I recommend writing a tcl function that 
# UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.
#############################################################################
proc GS_DrawPosition { c position } {
    
    ### TODO: Fill this in 
    drawBoard $c
    drawPieces $c [updateBoardMoves [unhash $position]]

}


#############################################################################
# board = current state of the board
# c = canvas
# This method just draw the pieces with the given board position without animation
#############################################################################
proc drawPieces { c board } {
    global boardSize 
    for { set i 0 } { $i < $boardSize } { incr i 1} {
	for { set j 0 } { $j < $boardSize } { incr j 1} {
	    if { [string index $board [posInString $i $j]] == "0" } { #if there is not piece there, then no need to check
		continue
	    }
	    if { ($j - 1) >= 0 && ($j + 1) < $boardSize } {
		if { ($i - 1) >= 0 && ($i + 1) < $boardSize } {
		    if { ([string index $board [posInString [expr $i - 1] $j]] == [string index $board [posInString $i $j]]) && \
			     ([string index $board [posInString [expr $i + 1] $j]] == [string index $board [posInString $i $j]]) } {
			drawVerticalLine $c $i $j [getColor $i $j $board] "yes" "both"
			set board [string replace $board [posInString $i $j] [posInString $i $j] "0"]
			continue
		    } elseif { ([string index $board [posInString $i [expr $j - 1]]] == [string index $board [posInString $i $j]]) && \
				   ([string index $board [posInString $i [expr $j + 1]]] == [string index $board [posInString $i $j]]) } {
			drawHorizontalLine $c $i $j [getColor $i $j $board] "yes" "both"
			set board [string replace $board [posInString $i $j] [posInString $i $j] "0"]
			continue
		    } else {
			continue
		    }
		} else {
		    if { ($j - 1) < 0 || ($j + 1) >= $boardSize } {
			continue
		    }
		    if  { ([string index $board [posInString $i [expr $j - 1]]] == [string index $board [posInString $i $j]]) && \
				   ([string index $board [posInString $i [expr $j + 1]]] == [string index $board [posInString $i $j]]) } { 
			drawHorizontalLine $c $i $j [getColor $i $j $board] "yes" "both"
			set board [string replace $board [posInString $i $j] [posInString $i $j] "0"]
			continue
		    } else {
			continue
		    }
		}
	    } else {
		if { ($i - 1) < 0 || ($i + 1) >= $boardSize } {
		    continue
		}
		if { ([string index $board [posInString [expr $i - 1] $j]] == [string index $board [posInString $i $j]]) && \
			 ([string index $board [posInString [expr $i + 1] $j]] == [string index $board [posInString $i $j]]) } {
		    drawVerticalLine $c $i $j [getColor $i $j $board] "yes" "both"
		    set board [string replace $board [posInString $i $j] [posInString $i $j] "0"]
		    continue
		} else {
		    continue
		}
	    }
	}
    }
}


#############################################################################
# x = vertical position you want to place the line
# y = horizontal position you want to place the line
# board = board configuration
# this procedure returns the appropriate color for the line depending on which player it belongs to
#############################################################################
proc getColor { x y board } {
    if { [string index $board [posInString $x $y]] == 1 } {
	return blue
    }
    return red
}


#############################################################################
# c = canvas
# x = center position x-coordinate where the line should be drawn
# y = center position y-coordinate where the line should be drawn
# *note that these x != x-axis when drawing the board 
# and y != y-axis when drawing the board
# this function draws an inner smaller box within the square itself
#############################################################################
proc drawSmallSquare { c x y color tagName} {
    global IndivSquSize
    
    $c create rect \
	[expr [getCenterCoor $y] - [expr $IndivSquSize * 0.25]] \
	[expr [getCenterCoor $x] - [expr $IndivSquSize * 0.25]] \
	[expr [getCenterCoor $y] + [expr $IndivSquSize * 0.25]] \
	[expr [getCenterCoor $x] + [expr $IndivSquSize * 0.25]] \
	-fill $color \
	-outline $color \
	-tag $tagName
}


#############################################################################
# c = canvas
# x = exact veritcal position on the board
# y = exact horizontal position on the board
# w = width of the rect divide by 2
# h = length of the rect divide by 2
# color = the color of the box 
# this proc will draw boxes with width w and hieght h, but this compensate if the center of the box is 
# outside the coordinate
#############################################################################
proc drawRect { c x y w h color tagName } {
    global IndivSquSize boardSize
    
    if { [expr $x + $h] > [expr $IndivSquSize * $boardSize] } {
	if { [expr $x + $h] < 0 } {
	    set bottom 0
	} else {
	    set bottom [expr $IndivSquSize * $boardSize]
	}
    } else {
	set bottom [expr $x + $h]
    } 

    if { [expr $x - $h] < 0 } {
	set top 0
    } else {
	if { [expr $x - $h] > [expr $IndivSquSize * $boardSize] } {
	    set top [expr $IndivSquSize * $boardSize]
	} else {
	    set top [expr $x - $h]
	}
    }
	 
    if { [expr $y + $w] > [expr $IndivSquSize * $boardSize] } {
	set right [expr $IndivSquSize * $boardSize]
    } else {
	if { [expr $y + $w] < 0} {
	    set right 0
	} else {
	    set right [expr $y + $w]
	}
    }

    if { [expr $y - $w] < 0 } {
	set left 0 
    } else {
	if { [expr $y - $w] > [expr $IndivSquSize * $boardSize] } {
	    set left [expr $IndivSquSize * $boardSize]
	} else {
	    set left [expr $y - $w]
	}
    }
    
    $c create rect \
	$left \
	$top \
	$right \
	$bottom \
	-fill $color \
	-outline $color \
	-tag $tagName
}


#############################################################################
# c = canvas
# x = exact veritcal position on the board
# y = exact horizontal position on the board
# l = length of the square
# color = the color of the box 
# this proc will draw a square with length l but this compensate if the center of the box is 
# outside the coordinate
# same as above but for square
#############################################################################
proc drawSqu { c x y l color tagName } {
    drawRect $c $x $y $l $l $color $tagName
}

#############################################################################
# initialSize = the initial size of the piece in radius
# finalSize = the final size of the piece in radius *this size will be constant 
# base on the size of the board, but I wrote it so that it can be modular
# Plus I did it this way so that the animation can work backward as well
# numOfFrames = this is the number of frames that will be drawn
# frame = this is the exact frame that we are on
# This procedure just draws return the radius for size square and general Shapes 
#############################################################################
proc sizeOfRadius { initialSize finalSize numOfFrames frame } {
    set individualSize [expr [expr $initialSize - $finalSize] / $numOfFrames]

    set temp [expr $individualSize * $frame]
    
    return [expr $initialSize - $temp] 
    #if say frame = 0 then it should just return initialSize
} 


#############################################################################
# regularSize = the regular size of the final piece, usually in this case it's constant
# base of the board size. This is the thickness of the connection line;
# ex: I set this to be 1/4 length of the squares on this board. This must be in exact size
# timesBigger = the number of times the regular size of a piece
# position = the position of the box, there is only four in this case, top, bottom, left, and right
# This procedure just calculate the initial center position of where the piece should
# be place base on the ratio of the size.
# This is just doing straight scaling 
# This proc return a list that contains the exact coordinate of the center of new box in a list form
# the first being the vertical position and the second is the horizontal position
#############################################################################
proc initialCenter { regularSize timesBigger position} {
    set width [expr $regularSize * 3]
    set temp [expr $width * $timesBigger]
    set final [expr $temp + $regularSize * $timesBigger]
    
    if { $position == "left" } { 
	return [list [lindex [centerBoard] 0] [expr [lindex [centerBoard] 1] - $final]]
    } elseif { $position == "right" } {
	return [list [lindex [centerBoard] 0] [expr [lindex [centerBoard] 1] + $final]]
    } elseif { $position == "top" } {
	return [list [expr [lindex [centerBoard] 0] - $final] [lindex [centerBoard] 1]]
    } else { 
	return [list [expr [lindex [centerBoard] 0] + $final] [lindex [centerBoard] 1]]
    }
}


#############################################################################
# This proc returns the exact coordinates of the board in list form
# the first being the vetical coordinate and the second is the horizontal
# this proc hacks a bit cause it expliot the fact that board is only 5 by 5 big
# updated now support any size board
#############################################################################
proc centerBoard {} {
    global boardSize
    
    return [list [getCenterCoor [expr ($boardSize - 1) / 2]] [getCenterCoor [expr ($boardSize - 1) / 2]]]
}

proc getCenter {} {
    global boardSize 
    
    return [list [expr ($boardSize - 1) / 2] [expr ($boardSize - 1) / 2]]
}


#############################################################################
# initialPoint = the initial (vertical, horizontal) coordinate in list form
# finalPoint = the final (vertical, horizontal) coordinate in list form
# numOfFrames = the total number of frames in animation
# frame = the current frame that we are examining 
# This proc basically just return in list form the points that is inbetween at that frame
#############################################################################
proc calCenter {initialPoint finalPoint numOfFrames frame } {
    set vertical [expr ((([lindex $finalPoint 0] - [lindex $initialPoint 0]) / \
			     $numOfFrames) * \
			    $frame) + \
		      [lindex $initialPoint 0]]

    set horizontal [expr ((([lindex $finalPoint 1] - [lindex $initialPoint 1]) / \
			       $numOfFrames) * \
			      $frame) + \
			[lindex $initialPoint 1]]

    return [list $vertical $horizontal]
}



#############################################################################
# c = canvas
# x = center position x-coordinate where the line should be drawn
# y = center position y-coordinate where the line should be drawn
# *note that these x != x-axis when drawing the board 
# and y != y-axis when drawing the board
# This method draws the horizontal line that passes through this point
#############################################################################
proc drawHorizontalLine { c x y color square where} {
    global IndivSquSize
    
    #default position
    set tagName "0,0" 

    #set the tag to be the position, used later to delete the peice for undo
    set tagName [string replace $tagName 2 2 $y]
    set tagName [string replace $tagName 0 0 $x]
    
    $c create line [getCenterCoor [expr $y - 1]] [getCenterCoor $x] \
	[getCenterCoor [expr $y + 1]] [getCenterCoor $x] \
	-width [expr $IndivSquSize * 0.25] \
	-fill $color \
	-tag $tagName

    if { $square == "yes" } {
	if { $where == "both" } { 
	    drawSmallSquare $c $x [expr $y - 1] $color $tagName
	    drawSmallSquare $c $x [expr $y + 1] $color $tagName
	} elseif { $where == "right" } {
	    drawSmallSquare $c $x [expr $y + 1] $color $tagName
	} else {
	    drawSmallSquare $c $x [expr $y - 1] $color $tagName
	}
    }
}


#############################################################################
# same as above, but now vertical line
#############################################################################
proc drawVerticalLine { c x y color square where} {
    global IndivSquSize 

    #default position
    set tagName "0,0" 

    #set the tag to be the position, used later to delete the peice for undo
    set tagName [string replace $tagName 2 2 $y]
    set tagName [string replace $tagName 0 0 $x]

    $c create line [getCenterCoor $y] [getCenterCoor [expr $x - 1]] \
	[getCenterCoor $y] [getCenterCoor [expr $x + 1]] \
	-width [expr $IndivSquSize * 0.25] \
	-fill $color \
	-tag $tagName

    if { $square == "yes" } {
	if { $where == "both" } { 
	    drawSmallSquare $c [expr $x - 1] $y $color $tagName
	    drawSmallSquare $c [expr $x + 1] $y $color $tagName
	} elseif { $where == "bottom" } {
	    drawSmallSquare $c [expr $x + 1] $y $color $tagName
	} else {
	    drawSmallSquare $c [expr $x - 1] $y $color $tagName
	}
    }
}


#############################################################################
# vertical = vertical coordinate of the block
# returns the the exact coordinate of the center of the block on the cavas
#############################################################################
proc getCenterCoor { vertical } {
    global IndivSquSize
    
    return [expr (($vertical + 1) * $IndivSquSize) - ($IndivSquSize/2)]
}


#############################################################################
# GS_NewGame should start playing the game. 
# It's arguments are a canvas, c, where you should draw 
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.
#############################################################################
proc GS_NewGame { c position } {
    # TODO: The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    GS_DrawPosition $c $position
}


#############################################################################
# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
#############################################################################
proc GS_WhoseMove { position } {
    # Optional Procedure
    return ""    
}


#############################################################################
# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"
#
# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.
#############################################################################
proc GS_HandleMove { c oldPosition theMove newPosition } {

    global boardSize

    set coor [zerosToCoor [mapMove $theMove]]
    set tempBoard [updateBoardMoves [unhash $newPosition]]

    #if either of the coordinate is out of range of vertical, then it must be horizontal
    if { [expr [lindex $coor 0] - 1] < 0 || [expr [lindex $coor 0] + 1] >= $boardSize } {
	set pos "hori"
    } elseif { [expr [lindex $coor 1] - 1] < 0 || [expr [lindex $coor 1] + 1] >= $boardSize } {
	set pos "vert"
    } else {
	if {  [string index $tempBoard [posInString [expr [lindex $coor 0] - 1] [lindex $coor 1]]] == \
		  [string index $tempBoard [posInString [lindex $coor 0] [lindex $coor 1]]] && \
		  [string index $tempBoard [posInString [lindex $coor 0] [lindex $coor 1]]] == \
		  [string index $tempBoard [posInString [expr [lindex $coor 0] + 1] [lindex $coor 1]]] } {
	    set pos "vert"
	} else {
	    set pos "hori"
	}
    }

    animateLine $c [lindex $coor 0] [lindex $coor 1] [getColor [lindex $coor 0] [lindex $coor 1] $tempBoard] $pos
    
    #animateFullMove $c [lindex $coor 0] [lindex $coor 1] [getColor [lindex $coor 0] [lindex $coor 1] $tempBoard] $pos
}


#############################################################################
# GS_ShowMoves draws the move indicator (be it an arrow or a dot, whatever the
# player clicks to make the move)  It is also the function that handles coloring
# of the moves according to value. It is called by gamesman just before the player
# is prompted for a move.
#
# Arguments:
# c = the canvas to draw in as usual
# moveType = a string which is either value, moves or best according to which radio button is down
# position = the current hashed position
# moveList = a list of lists.  Each list contains a move and its value.
# These moves are represented as numbers (same as in C)
# The value will be either "Win" "Lose" or "Tie"
# Example:  moveList: { 73 Win } { 158 Lose } { 22 Tie } 
#############################################################################
proc GS_ShowMoves { c moveType position moveList } {

    ### TODO: Fill this in
    foreach move $moveList {
	drawMove $c [lindex $move 0] [lindex $move 1] $moveType
    }
}


#############################################################################
# c = canvas 
# value = the move 
# type = the type of move, win, lose, or tie
# moveType is the string that either value, moves or best according to which radio button is down
# This procedure draw the possible move given the value of the move. 
############################################################################# 
proc drawMove { c moveValue type moveType} {

    global IndivSquSize

    switch $moveType {
	value {
	    
	    # If the move leads to a win, it is a losing move (RED)
	    # If the move leads to a losing position, it is a winning move (GREEN)
	    # If the move leads to a tieing position, it is a tieing move (YELLOW)
	    switch $type {
		Win { set color darkred }
		Lose { set color green }
		Tie { set color yellow }
		default { set color cyan }
	    }
	}
	default {
	    set color cyan
	}
    }

    set move [zerosToCoor [mapMove $moveValue]]

    set leftTop [list [expr [getCenterCoor [lindex $move 1]] - [expr $IndivSquSize / 2]] \
		     [expr [getCenterCoor [lindex $move 0]] - [expr $IndivSquSize / 2 ]]]
    
    set rightBottom [list [expr [getCenterCoor [lindex $move 1]] + [expr $IndivSquSize / 2]] \
			 [expr [getCenterCoor [lindex $move 0]] + [expr $IndivSquSize / 2]]]
    
    set tmp [$c create polygon \
		 [expr [lindex $leftTop 0] + ($IndivSquSize * 0.25)] \
		 [lindex $leftTop 1] \
		 [expr [lindex $rightBottom 0] - ($IndivSquSize * 0.25)] \
		 [lindex $leftTop 1] \
		 [lindex $rightBottom 0] \
		 [expr [lindex $leftTop 1] + ($IndivSquSize * 0.25)] \
		 [lindex $rightBottom 0] \
		 [expr [lindex $rightBottom 1] - ($IndivSquSize * 0.25)] \
		 [expr [lindex $rightBottom 0] - ($IndivSquSize * 0.25)] \
		 [lindex $rightBottom 1] \
		 [expr [lindex $leftTop 0] + ($IndivSquSize * 0.25)] \
		 [lindex $rightBottom 1] \
		 [lindex $leftTop 0] \
		 [expr [lindex $rightBottom 1] - ($IndivSquSize * 0.25)] \
		 [lindex $leftTop 0] \
		 [expr [lindex $leftTop 1] + ($IndivSquSize * 0.25)] \
		 -fill $color \
		 -tag "moves"]

    #used previously for generating square move
    #set tmp [$c create rect \
		 #[expr [getCenterCoor [lindex $move 1]] - [expr $IndivSquSize / 2]] \
		 #[expr [getCenterCoor [lindex $move 0]] - [expr $IndivSquSize / 2]] \
		 #[expr [getCenterCoor [lindex $move 1]] + [expr $IndivSquSize / 2]] \
		 #[expr [getCenterCoor [lindex $move 0]] + [expr $IndivSquSize / 2]] \
		 #-fill $color \
		 #-tag "moves"]



    $c bind $tmp <Enter> "$c itemconfigure $tmp -fill black"
    $c bind $tmp <Leave> "$c itemconfigure $tmp -fill $color"
    $c bind $tmp <ButtonRelease-1> "ReturnFromHumanMove $moveValue"
}


#############################################################################
# c = canvas
# vertical is the vertical coordinates that you want to put the move
# horizontal is the same as above, but horizontal
# color = color you want to color it 
# pos = either "hori" or "vert" position
# this procedure just use the animateGeneralMove to animate the full move
#############################################################################
proc animateFullMove { c vertical horizontal color pos } {
    
    global gAnimateType
    
    animateGeneralMove $c $vertical $horizontal $color $pos "full" "whocares" "fullMoves" $gAnimateType $gAnimateType
} 


#############################################################################
# vertical is the vertical coordinates that you want to put the move
# horizontal is the same as above, but horizontal
# color = color you want to color it 
# pos = either "hori" or "vert" position
# this procedure just use the animateGeneralMove to animate just the line that is 
# place inbetween the two square
#############################################################################
proc animateLine { c vertical horizontal color pos } {

    global gAnimateType 

    animateGeneralMove $c $vertical $horizontal $color $pos "line" "whocares" "fullMoves" $gAnimateType $gAnimateType
}


#############################################################################
# vertical is the vertical coordinates that you want to put the move
# horizontal is the same as above, but horizontal
# Note that it's just the coordinate of the line not the box
# color = color you want to color it 
# pos = either "hori" or "vert" position
# this procedure just use the animateGeneralMove to animate the move with just a
# square off one of the line
#############################################################################
proc animateSingleBox { c vertical horizontal color pos placement } {

    global gAnimateType

    animateGeneralMove $c $vertical $horizontal $color $pos "singleBox" $placement "fullMove" $gAnimateType $gAnimateType
}


#############################################################################
# c = canvas
# vertical = vertical coor
# horizontal = horizontal coor
# color of the move
# pos = the list of the coor where you want to put the move
# form = line || linew/box || fullmove
# placement = only used if using linew/box because you need to tell if which side the square should exist
# moveType = either undo or regular. If undo, then it's used for animating the undo, else, just regular
# moveSize = either small or big
# reverse = if 0 means that you don't want it to reverse the animation and 1 if you do want to reverse the animation
# this proc will animate full moves for you, given the coordinate on connection board
#############################################################################
proc animateGeneralMove { c vertical horizontal color pos form placement moveType moveSize reverse} {

    global definition IndivSquSize boardSize gAnimationSpeed 

    #default variable 
    set pos1 "top"
    set pos2 "bottom"

    if { $pos == "hori" } {
	set pos1 "left"
	set pos2 "right"
    }
    
    #This is the default biggest size the the piece will obtain
    set maxBiggness [expr $boardSize * 4]

    set place1 [list $vertical $horizontal]

    if { $gAnimationSpeed < 0 } {
	if { $gAnimationSpeed % 2 == 1 } {
	    set speed [expr [expr $gAnimationSpeed / 2] + 4]
	} else {
	    set speed [expr [expr $gAnimationSpeed /2] + 3]
	} 
    } else {
	set speed [expr [expr $gAnimationSpeed / 2] + 3]
    }

    #if speed is max, just set to fastest
    if { $gAnimationSpeed == 5 } {
	set speed 10
    }

    for { set i 0 } { $i < $definition } {incr i $speed} {

	if { $reverse == 1 } {
	    set j [expr $definition - $i]
	} else {
	    set j $i
	}

	set finalCenterX1 [getCenterCoor [expr [lindex $place1 0] - 1]]
	set finalCenterY1 [getCenterCoor [lindex $place1 1]]
	set finalCenterX2 [getCenterCoor [expr [lindex $place1 0] + 1]]
	set finalCenterY2 $finalCenterY1
	if { $pos == "hori" } {
	    set finalCenterX1 [getCenterCoor [lindex $place1 0]]
	    set finalCenterY1 [getCenterCoor [expr [lindex $place1 1] - 1]]
	    set finalCenterX2 $finalCenterX1
	    set finalCenterY2 [getCenterCoor [expr [lindex $place1 1] + 1]]
	}

	if { $moveSize == 1 } {
	    set finalCenterX [getCenterCoor [lindex $place1 0]]
	    set finalCenterY [getCenterCoor [lindex $place1 1]]
	}

	set tempCenter1 [calCenter \
			     [initialCenter [expr 0.25*$IndivSquSize] $maxBiggness $pos1] \
			     [list $finalCenterX1 $finalCenterY1] \
			     $definition  \
			     [expr $j + 1]]
	
	set tempCenter2 [calCenter \
			     [initialCenter [expr 0.25*$IndivSquSize] $maxBiggness $pos2] \
			     [list $finalCenterX2 $finalCenterY2] \
			     $definition \
			     [expr $j + 1]]

	if { $moveSize == 1 } {
	    set tempCenter1 [calCenter \
				 [list $finalCenterX1 $finalCenterY1] \
				 [list $finalCenterX $finalCenterY] \
				 $definition  \
				 [expr $j + 1]]
	
	    set tempCenter2 [calCenter \
				 [list $finalCenterX2 $finalCenterY2] \
				 [list $finalCenterX $finalCenterY] \
				 $definition \
				 [expr $j + 1]]
	}

	set timesBigger [expr ([lindex [getCenter] 0]) + .5]
	
	if { $moveSize == 1 } {
	    set radiusSize [sizeOfRadius [expr $IndivSquSize * 0.25] 0 $definition [expr $j + 1]]
	} else {
	    set radiusSize [sizeOfRadius [expr $timesBigger * $IndivSquSize] [expr $IndivSquSize * 0.25] $definition [expr $j + 1]]
	}

	#default
	set placement1 "left"
	set placement2 "right"

	if { $form == "singleBox" } {
	    if { $placement == "left" || $placement == "top" } {
		set placement1 $placement
		set placement2 "gibberish"
	    } else {
		set placement1 "gibberish"
		set placement2 $placement
	    }
	} 

	if { $form != "line" } {
	    if { $placement1 == "left" || $placement1 == "top" } {
		#draws the left or top box
		drawSqu $c [lindex $tempCenter1 0] \
		    [lindex $tempCenter1 1] \
		    $radiusSize\
		    $color \
		    box1
	    }
	    if { $placement2 == "right" || $placement2 == "bottom" } {
		#draw the right or bottom box
		drawSqu $c [lindex $tempCenter2 0] \
		    [lindex $tempCenter2 1] \
		    $radiusSize \
		    $color \
		    box2
	    }
	}

        set tempCenterLine [calCenter \
				[list [getCenterCoor [lindex [getCenter] 0]] [getCenterCoor [lindex [getCenter] 1]]] \
				[list [getCenterCoor [lindex $place1 0]] [getCenterCoor [lindex $place1 1]]] \
				$definition \
				[expr $j + 1]]

	set initialSize [expr $timesBigger * $IndivSquSize]
	set finalSize [expr $IndivSquSize * 0.125]
	
	if { $moveSize == 1 } {
	    set initialSize [expr $IndivSquSize * 0.125] 
	    set finalSize 0
	    set tempCenterLine [list [getCenterCoor [lindex $place1 0]] [getCenterCoor [lindex $place1 1]]]
	}

	if { $pos == "hori" } {
	    #draw the line in between
	    drawRect $c [lindex $tempCenterLine 0] \
		[lindex $tempCenterLine 1] \
		[expr [lindex $tempCenter1 1] - [lindex $tempCenterLine 1]] \
		[sizeOfRadius $initialSize $finalSize $definition [expr $j + 1]] \
		$color lineBetween
	} else {
	    drawRect $c [lindex $tempCenterLine 0] \
		[lindex $tempCenterLine 1] \
		[sizeOfRadius $initialSize $finalSize $definition [expr $j + 1]] \
		[expr [lindex $tempCenter1 0] - [lindex $tempCenterLine 0]] \
		$color lineBetween 
	}

	after 1
	update idletask
	$c delete box1
	$c delete box2
	$c delete lineBetween
    }

    if {$moveType != "undo" } {
	if { $pos == "hori" } {
	    if { $form == "line" } {
		drawHorizontalLine $c [lindex $place1 0] [lindex $place1 1] $color "no" "whocares"
	    } elseif { $form == "singleBox" } {
		drawHorizontalLine $c [lindex $place1 0] [lindex $place1 1] $color "yes" $placement
	    } else {
		drawHorizontalLine $c [lindex $place1 0] [lindex $place1 1] $color "yes" "both"
	    }
	} else {
	    if { $form == "line" } {
		drawVerticalLine $c [lindex $place1 0] [lindex $place1 1] $color "no" "whocares"
	    } elseif { $form == "singleBox" } {
		drawVerticalLine $c [lindex $place1 0] [lindex $place1 1] $color "yes" $placement
	    } else {
		drawVerticalLine $c [lindex $place1 0] [lindex $place1 1] $color "yes" "both"
	    }
	}
    }
}
    

#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

    ### TODO: Fill this in
    $c delete moves

}


#############################################################################
# GS_HandleUndo handles undoing a move (possibly with animation)
# Here's the undo logic
# The game was in position A, a player makes move M bringing the game to position B
# then an undo is called
# currentPosition is the B
# theMoveToUndo is the M
# positionAfterUndo is the A
#
# By default this function just calls GS_DrawPosition, but you certainly don't 
# need to keep that.
#############################################################################
proc GS_HandleUndo { c currentPosition theMoveToUndo positionAfterUndo} {

    global boardSize

    ### TODO if needed

    set move [zerosToCoor [mapMove $theMoveToUndo]]

    #default position
    set tagName "0,0" 

    #set the tag to be the position, used later to delete the peice for undo
    set tagName [string replace $tagName 2 2 [lindex $move 1]]
    set tagName [string replace $tagName 0 0 [lindex $move 0]]

    $c delete $tagName

    set tempBoard [updateBoardMoves [unhash $currentPosition]]

    #if either of the coordinate is out of range of vertical, then it must be horizontal
    if { [expr [lindex $move 0] - 1] < 0 || [expr [lindex $move 0] + 1] >= $boardSize } {
	set pos "hori"
    } elseif { [expr [lindex $move 1] - 1] < 0 || [expr [lindex $move 1] + 1] >= $boardSize } {
	set pos "vert"
    } else {
	if {  [string index $tempBoard [posInString [expr [lindex $move 0] - 1] [lindex $move 1]]] == \
		  [string index $tempBoard [posInString [lindex $move 0] [lindex $move 1]]] && \
		  [string index $tempBoard [posInString [lindex $move 0] [lindex $move 1]]] == \
		  [string index $tempBoard [posInString [expr [lindex $move 0] + 1] [lindex $move 1]]] } {
	    set pos "vert"
	} else {
	    set pos "hori"
	}
    }
    
    #animateLine $c [lindex $move 0] [lindex $move 1] [getColot [lindex $move 0] [lindex $move 1] $tempBoard $pos

    animateUndoMove $c [lindex $move 0] [lindex $move 1] [getColor [lindex $move 0] [lindex $move 1] $tempBoard] $pos

}


#############################################################################
# c = canvas
# x = vertical coor of the move
# y = horizontal coor of the move
# color = the color of the player moving
# pos = the type of move that was initially there like horizontal or vertical
#############################################################################
proc animateUndoMove { c x y color pos } {
    
    global gAnimateType

    if { $gAnimateType == 0 } {
	animateGeneralMove $c $x $y $color $pos "line" "whocares" "undo" $gAnimateType 1
	#animateGeneralMove $c $x $y $color $pos "full" "whocares" "undo" $gAnimateType 1 
    } else {
	animateGeneralMove $c $x $y $color $pos "line" "whocares" "undo" $gAnimateType 0
	#animateGeneralMove $c $x $y $color $pos "full" "whocares" "undo" $gAnimateType 0
    }
}


#############################################################################
###################### DO ONLY IF HAVE TIME #################################
#############################################################################



#############################################################################
# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
#############################################################################
proc GS_GetGameSpecificOptions { } {
}


#############################################################################
# GS_GameOver is called the moment the game is finished ( won, lost or tied)
# You could use this function to draw the line striking out the winning row in 
# tic tac toe for instance.  Or, you could congratulate the winner.
# Or, do nothing.
#############################################################################
proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove} {

	### TODO if needed
	
}


#############################################################################
# GS_UndoGameOver is called when the player hits undo after the game is finished.
# This is provided so that you may undo the drawing you did in GS_GameOver if you 
# drew something.
# For instance, if you drew a line crossing out the winning row in tic tac toe, 
# this is where you should delete the line.
#
# note: GS_HandleUndo is called regardless of whether the move undoes the end of the 
# game, so IF you choose to do nothing in GS_GameOver, you needn't do anything here either.
#############################################################################
proc GS_UndoGameOver { c position } {

	### TODO if needed

}




#The Following Procedure are never used, but someone might find it useful for later addition for the tcl of this games

#############################################################################
# vertical is the actually coordinate of the board
# This method returns board vertical coordinates
#############################################################################
proc getBoxCoorX { vertical } {
    global IndivSquSize

    return [expr $vertical / $IndivSquSize]
}


#############################################################################
# horizontal is the actually coordinate of the board
# This method returns board horizontal coordinates
#############################################################################
proc getBoxCoorY { horizontal } {
    global IndivSquSize 

    return [expr $horizontal / $IndivSquSize]
}


#############################################################################
# c = canvas
# move = the move that will be taken
#
#DO NOT USE, THIS PROC IS NEVER USED, IT'S HARDCODED :)
#
# this proc will animate just the box for you, given the coordinate on connection board
#############################################################################
proc animateSquMove { c vertical horizontal color} {

    global definition IndivSquSize

    set place1 [list $vertical $horizontal]

    for { set i 0 } { $i < $definition } {incr i 1} {

	set tempCenter [calCenter \
			    [list [getCenterCoor 2] [getCenterCoor 2]]\
			    [list [getCenterCoor [lindex $place1 0]] [getCenterCoor [expr [lindex $place1 1] - 1]]] \
			    $definition  \
			    [expr $i + 1]]
	 
	set radiusSize [sizeOfRadius [expr $timesBigger * $IndivSquSize] [expr $IndivSquSize * 0.25] $definition [expr $i + 1]]

	#draw the Square
	drawSqu $c [lindex $tempCenter 0] \
	    [lindex $tempCenter 1] \
	    $radiusSize\
	    $color \
	    Box

	if { [expr $i + 1]  >= $definition } {
	    continue
	}
	after 1
	update idletask
	$c delete Box
    }
}

