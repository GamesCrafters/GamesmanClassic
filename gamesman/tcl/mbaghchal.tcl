####################################################
# mbaghchal.tcl
# 
# Baghchal GUI, Tcl/Tk
#
# Tcl/Tk done by Yuly Tenorio, based on Eudean Sun's Asalto.tcl
####################################################

#Game variables, other vars in DrawBoard and drawmove

# t = line thickness
set t 5

# dot = dot radius (at intersections)
set dot 20

# color = color of the board
set color "black"

# outlineColor = color of piece outlines
set outlineColor "black"

# outline = size of piece outlines
set outline 3

# player1 = color of player 1's pieces (goats)
set player1 "blue"

# player2 = color of player 2's pieces (tigers)
set player2 "red"

# arrowlength = relative length of arrows (100% would be completely across a square)
set arrowlength 0.7

# arrowcolor = default color of arrows showing moves
set arrowcolor cyan

# basespeed = base speed of the animations in milliseconds (values 10-1000, larger is faster)
set basespeed 100


set boardsize 3



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
    set kGameName "Bagh Chal"
    
    ### Set the initial position of the board (default 0)

    global gInitialPosition gPosition
#
    set gInitialPosition [C_InitialPosition]
    set gPosition $gInitialPosition

    ### Set the strings to be used in the Edit Rules

    global kStandardString kMisereString
    set kStandardString "First player to ______ WINS"
    set kMisereString "First player to ______ LOSES"

    ### Set the strings to tell the user how to move and what the goal is.
    ### If you have more options, you will need to edit this section

    global gMisereGame
    if {!$gMisereGame} {
	SetToWinString "To Win: As the tigers, your goal is to capture the goats (by jumping over them) and to avoid getting trapped by the goats. As the goats, your goal is to avoid getting captured and to attempt to trap the tigers."
    } else {
	SetToWinString "To Win: As the tigers, your goal is to avoid capturing the goats and to attempt to get trapped by them. As the goats, your goal is to attempt to get captured by the tigers and to avoid getting the tigers trapped."
    }
    SetToMoveString "To Move: For the tigers and for the goats (but only after placing all 20 goats on the board), possible moves are indicated by arrows around your pieces. Click on one of these arrows to move in the direction indicated."
	    
    # Authors Info. Change if desired
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Max Delgadillo"
    set kTclAuthors "Yuly Tenorio"
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
#
    return [list t g]

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
#
    # player1 = goats, player2 = tigers 	
    global player1 player2

    return [list $player1 $player2]
    
    
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
    
# from ttt3tier
#
#    global gMisereGame
#        if { $gMisereGame == 0 } {
#            set option 2
#        } else {
#            set option 1
#        }
#    return $option
    
    
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
    
    
   #from ttt3tier 
   #     global gMisereGame
   #     if { $option == 1 } {
   #         set gMisereGame 1
   #     } else {
   #         set gMisereGame 0
   # }
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
	#    global r s size margin gFrameWidth gFrameHeight
	global s margin size color outlineColor outline t dot r player1 player2 fontsize


	# Save the GC background skin
	$c addtag background all

	#
	#set board "T G T  G    G    G  T G T"
	set board "T T   T T"    

	# Draw the default board with default piece configuration
	DrawBoard $c
	DrawPieces $c $board

# Finally, draw a title
	 $c create rectangle \
		0 \
		[expr $size/2 - 50] \
		$size \
		[expr $size/2 + 50] -fill gray -width 1 -outline black
	 $c create text \
		[expr $size/2] [expr $size/2] -text "Welcome to Bagh Chal!" -font "Arial $fontsize" -anchor center -fill black -width [expr 4*$s] -justify center
}

proc min { a b } {
	if { $a < $b } {
		return $a
	}
	return $b
}	


######################
#Proc DrawBoard
######################
proc DrawBoard { c } {
	#    global size margin s
	global s margin size color outlineColor outline t dot r player1 player2 gFrameWidth gFrameHeight fontsize arrowwidth arrowhead boardsize

	# If we don't clean the canvas, things pile up and get slow
	# We need to keep the background, though (otherwise we get ugly borders around game on some resolutions)
	$c delete {!background}


	#size of board
	set size [min $gFrameWidth $gFrameHeight]
	#margin
	set margin [expr 0.1*$size]
	#side
	set s [expr ($size - 2*$margin)/($boardsize - 1)]
	#radius
	#set r [expr 0.2 * $s]     ###

	#radius
	if { $boardsize == 3} {
		set r [expr 0.2 * $s]
	} elseif { $boardsize == 4 } {
		set r [expr 0.25 * $s]
	} else {
		set r [expr 0.3 * $s]
	}



	set fontsize [expr int($size / 20)]
	set arrowwidth [expr 0.4 * $r]
	set arrowhead [list [expr 2 * $arrowwidth] [expr 2 * $arrowwidth] $arrowwidth]


	#Draw default board 

	#make gray background
	$c create rect \
	0 0 \
	$size $size \
	-fill darkgray -width 1 -outline black

	# main board lines
 	   
	for { set i 0 } { $i < $boardsize } { incr i } {
	# Vertical lines
	$c create line \
	    [expr $margin + $i * $s] \
	    $margin \
	    [expr $margin + $i * $s] \
	    [expr $size - $margin] \
	    -width 3

	# Horizontal lines
	$c create line \
	    $margin \
	    [expr $margin + $i * $s] \
	    [expr $size - $margin] \
	    [expr $margin + $i * $s] \
	    -width 3
	}

	# Diagonal lines

	$c create line \
	$margin \
	$margin \
	[expr $size - $margin] \
	[expr $size - $margin] \
	-width 3


	$c create line \
	$margin \
	[expr $size - $margin] \
	[expr $size - $margin] \
	$margin \
	-width 3


	if { $boardsize == 5 } {
		$c create line \
		$margin \
		[expr $size / 2] \
		[expr $size / 2] \
		[expr $size - $margin] \
		-width 3

		$c create line \
		[expr $size / 2] \
		$margin \
		[expr $size - $margin] \
		[expr $size / 2] \
		-width 3

		$c create line \
		$margin \
		[expr $size / 2] \
		[expr $size / 2] \
		$margin \
		-width 3

		$c create line \
		[expr $size / 2] \
		[expr $size - $margin] \
		[expr $size - $margin] \
		[expr $size / 2] \
		-width 3 	    
	}

	#draw dots on intersection of lines
	for {set j 0} {$j < $boardsize} {incr j} {
	    for {set i 0} {$i < $boardsize} { incr i} {
		$c create oval \
			[expr $margin - $dot + $i * $s] \
			[expr $margin - $dot + $j * $s] \
			[expr $margin + $dot + $i * $s] \
			[expr $margin + $dot + $j * $s] \
			-fill $color -width 0    	
	    }
	}    	
    
}

#############################
# Draws pieces on the board #
#############################
proc DrawPieces { c board } {

	global boardsize
	#	draw [string index $board 0] 0 $c

	for {set i 0} {$i < ( $boardsize * $boardsize ) } {incr i} {
	draw [string index $board $i] $i $c
	}
}

######################################
# Draws a piece at index on canvas c #
######################################
proc draw { piece index c } {
	global r player1 player2 outline

	set location [coords $index]

	switch $piece {
	G { $c create oval \
		[expr [lindex $location 0] - $r] \
		[expr [lindex $location 1] - $r] \
		[expr [lindex $location 0] + $r] \
		[expr [lindex $location 1] + $r] -fill $player1 -width $outline}
	T { $c create oval \
		[expr [lindex $location 0] - $r] \
		[expr [lindex $location 1] - $r] \
		[expr [lindex $location 0] + $r] \
		[expr [lindex $location 1] + $r] -fill $player2 -width $outline}
	default {}
	}

}

########################################
# Returns coordinates of a board index #
########################################
proc coords { index } {
	global margin s boardsize

    
        set q [expr $index / $boardsize ]
    	set res [expr $index % $boardsize]
      
        return [list [expr $margin+$res*$s] [expr $margin+$q*$s]]
 
    
}

proc UnhashBoard {position arrayname} {
	upvar $arrayname a

	set board [C_CustomUnhash $position]

	for {set i 3} {$i < 12} {incr i} {
	if {[string equal [string index $board $i] "G"]} {   
	    set a([expr $i - 3]) g
	} elseif {[string equal [string index $board $i] "T"]} {
	    set a([expr $i - 3]) t
	} else {
	    set a([expr $i - 3]) -
	}
	}  
    
}

##################################################################################
#
#	Unhashes a move the same way it's done in mbaghchal.c  
#	returns: a list with [startingPoint   endingPoint  JumpedOn(position of 
#		 goat jumped on)]
##################################################################################
proc UnhashMove { theMove } {
	global boardsize	
	
	set theMove2 [expr $theMove - $boardsize]
	set jump [expr theMove2 % 2]
	set theMove3 [expr $theMove2 / 2]
	
	set direction [expr $theMove % 8]
	set theMove [expr $theMove / 8]
	
	set i [expr $theMove / length + 1]
	set jumpI $i
	
	set j [expr $theMove % length + 1]
	set jumpJ $j
	
	set startPt $theMove
	
	set UP 0
	set DOWN 1
	set UP_RIGHT 2
	set UP_LEFT 3
	set DOWN_RIGHT 4
	set DOWN_LEFT 5
	set RIGHT 6
	set LEFT 7	
	
    	switch $direction {
    		UP 	{ if { jump } { 
    				set i [expr $i - 2]
    				set jumpI [expr $jumpI - 1]
    		   	} else {
    		   	  	set i [expr $i - 1] }}
    		DOWN 	{ if { jump } { 
		    		set i [expr $i + 2]
		    		set jumpI [expr $jumpI + 1]
		        } else {
    		     		set i [expr $i + 1] }}
    		RIGHT 	{ if { jump } { 
		    		set j [expr $j + 2]
		    		set jumpJ [expr $jumpJ + 1]
		        } else {
    		     		set j [expr $j + 1] }}
    		LEFT 	{ if { jump } { 
		    		set j [expr $j - 2]
		    		set jumpJ [expr $jumpJ - 1]
		        } else {
    		     		set j [expr $j - 1] }}    		     		
    		UP_RIGHT { if { jump } { 
		    		set i [expr $i - 2]
		    		set j [expr $j + 2]
		    		set jumpI [expr $jumpI - 1]		    		
		    		set jumpJ [expr $jumpJ + 1]
		        } else {
		        	set i [expr $i - 1]
    		     		set j [expr $j + 1] }}		
    		UP_LEFT { if { jump } { 
		    		set i [expr $i - 2]
		    		set j [expr $j - 2]
		    		set jumpI [expr $jumpI - 1]		    		
		    		set jumpJ [expr $jumpJ - 1]
		        } else {
		        	set i [expr $i - 1]
    		     		set j [expr $j - 1] }}		    		     		
    		DOWN_RIGHT { if { jump } { 
		    		set i [expr $i + 2]
		    		set j [expr $j + 2]
		    		set jumpI [expr $jumpI + 1]		    		
		    		set jumpJ [expr $jumpJ + 1]
		        } else {
		        	set i [expr $i + 1]
    		     		set j [expr $j + 1] }}		    		     		
    		DOWN_LEFT { if { jump } { 
		    		set i [expr $i + 2]
		    		set j [expr $j - 2]
		    		set jumpI [expr $jumpI + 1]		    		
		    		set jumpJ [expr $jumpJ - 1]
		        } else {
		        	set i [expr $i + 1]
    		     		set j [expr $j - 1] }}			
    	}
    	
    	set endPt [expr ($i-1) * $boardsize + $j -1]
    	
    	set jumpedOn [expr ($jumpI - 1) * $boardsize + $jumpJ - 1]
    	
    	return [list $startPt $endPt $jumpedOn]
	
}


###############################################################################
#
#	Draws the given array of g's and t's on the board
#
################################################################################
proc DrawPiecesArray { c arrayname } {
	
	global boardsize
	upvar $arrayname a
	#	draw [string index $board 0] 0 $c

	for {set i 0} {$i < ( $boardsize * $boardsize ) } {incr i} {
	if { $a($i) == "g" } {
		draw G $i $c 
	} elseif { $a($i) == "t" } {
		draw T $i $c
	}
	}
	
}



#############################################################################
# GS_Deinitialize deletes everything in the playing canvas.  I'm not sure why this
# is here, so whoever put this here should update this.  -Jeff
#############################################################################
proc GS_Deinitialize { c } {
	$c delete all
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

	DrawBoard $c
	GS_DrawPosition $c $position
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
#    
	set a(0) 0 
	### TODO: Fill this in
	UnhashBoard $position a              

	DrawBoard $c
	DrawPiecesArray $c a
	#DrawPieces $c $position
#
}





#############################################################################
# GS_WhoseMove takes a position and returns whose move it is.
# Your return value should be one of the items in the list returned
# by GS_NameOfPieces.
# This function is called just before every move.
#############################################################################
proc GS_WhoseMove { position } {
	# Optional Procedure

	    set board [C_CustomUnhash $position];
	       
	    
	    if { [string equal [string index $board 0] "1"] } {   
	      return t	  
	    } else {
	      return g
	    }
	    
	    
	#   set whoseTurn [lindex $boardList 0]
	#    return $whoseTurn    


	######
	#set board [C_CustomUnhash $position]

	#if {[string equal [string index $board 0] "1"]}{
	#return "T"
	#}else {
	#return "G"
	#}
}


proc NumGoatsLeft { position } {

	set board [C_CustomUnhash $position]
	
	if {[string equal [string index $board 1] "1"]} {
		return 1 }
	elseif { [string equal [string index $board 2] "0" } {
		return 0 
	} else { return 1 }
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

# There are two cases in Bagh Chal: dartboard, and rearranger
# In the first case, 
# 1) if the move is only one number, then we simply raise the correct piece.  
# 2) If the move is a slide move, then we cause the piece (which has already
#    been raised, to slide to the correct spot.
# So we:
# 1. figure out whose turn it is for dartboard moves:
#   a. unhash the oldPosition
#   b. set whoseTurn to the first element in boardList (which will be x or o)
# 2. if theMove is less than 10, then we are in dart board moves so:
#   a. raise the appropriate piece at piece$whoseTurn$theMove
# 3. Else, theMove is a rearanger move so:

proc GS_HandleMove { c oldPosition theMove newPosition } {

	global boardsize
	DrawBoard $c
	#set board [C_CustomUnhash $oldPosition]
	set a(0) 0

	set whoseTurn [GS_WhoseMove $oldPosition]
	



	set UnhashedMove [UnhashMove $theMove]

	set startPt [lindex $UnhashedMove 0]
	set origin [coords $startPt]  
	
	set endPt [lindex $UnhashedMove 1]
	set destination [coords $endPt]

	set JumpedOn [lindex $UnhashedMove 2]

	#set goatsLeft [NumGoatsLeft $oldPosition]
    
	
#	set b(0) 0
	
	UnhashBoard $newPosition a    		
#	UnhashBoard $oldPosition b
	
	
	
	#OR
	# if { $a(startPt) == "g" } {
      	# 	set whoseTurn G
	#  } else {
	#   	set whoseTurn T
	#  }
		
	
	if {whoseTurn == "g"} {
		set color blue 
		DrawBoard $c
		#goat being placed		
		if {$theMove < $boardsize} {
			#placing goat       		
			DrawPiecesArray $c a 
		} else { 
			#moving a goat
			#deleting goat
			set $a(startPt) -
			set $a(endPt) g
			#redraw board with goat at ending positiong    		
			DrawPiecesArray $c a  
		}
	} else {
		DrawBoard $c
		set color red
		set $a($startPt) -
		set $a($JumpedOn) -
		set $a($endPt) t
		
		DrawPiecesArray $c a


		}
	
	
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
		drawmove $c $move $moveType $position
	}
}

proc drawmove { c move moveType position } {
	global boardsize arrowcolor
	
	switch $moveType {
		value {

		    # If the move leads to a win, it is a losing move (RED)
		    # If the move leads to a losing position, it is a winning move (GREEN)
		    # If the move leads to a tieing position, it is a tieing move (YELLOW)
		    switch [lindex $move 1] {
			Win { set color darkred }
			Lose { set color green }
			Tie { set color yellow }
			default { set color $arrowcolor }
		    }
		}
		default {
		    set color $arrowcolor
		}
	}
	
	set UnhashedMove [UnhashMove $move]

	set startPt [lindex $UnhashedMove 0]
	set origin [coords $startPt]  
	
	set endPt [lindex $UnhashedMove 1]
	set destination [coords $endPt]

	set JumpedOn [lindex $UnhashedMove 2]
	
	set a(0) 0
	
	UnhashBoard $position a
	
	#drawing arrow
	
	set arrow [$c create line \
		   [expr [lindex $origin 0]] \
		   [expr [lindex $origin 1]] \
		   [expr [lindex $destination 0] - (1 - $arrowlength)*([lindex $destination 0] - [lindex $origin 0])] \
		   [expr [lindex $destination 1] - (1 - $arrowlength)*([lindex $destination 1] - [lindex $origin 1])] \
		   -width $arrowwidth -fill $color -arrow last -arrowshape $arrowhead]
	
	#set piece [string index $board [lindex $m 0]]
	set piece $a(startPt)

	draw $piece $startPt $c

	$c bind $arrow <Enter> "$c itemconfigure $arrow -fill black"
	$c bind $arrow <Leave> "$c itemconfigure $arrow -fill $color"
	$c bind $arrow <ButtonRelease-1> "ReturnFromHumanMove [lindex $move 0]"	
}


#############################################################################
# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the 
# same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.
#############################################################################
proc GS_HideMoves { c moveType position moveList} {

	### TODO: Fill this in
	GS_DrawPosition $c $position

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

	### TODO if needed
	GS_DrawPosition $c $positionAfterUndo
}


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
# this is where you sould delete the line.
#
# note: GS_HandleUndo is called regardless of whether the move undoes the end of the 
# game, so IF you choose to do nothing in GS_GameOver, you needn't do anything here either.
#############################################################################
proc GS_UndoGameOver { c position } {

	### TODO if needed

}
