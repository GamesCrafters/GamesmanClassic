####################################################
# this is a template for tcl module creation
#
# created by Alex Kozlowski and Peterson Trethewey
####################################################
#
# Jesse Phillips
# Jennifer Lee
# Rach Liu
# Jeff Chiang
####################################################


# GS_InitGameSpecific initializes game-specific features
# of the current game.  You can use this function 
# to initialize data structures, but not to present any graphics.
# It is called when the player first opens the game
# and after every rule change.
# You must set the global variables kGameName, gInitialPosition,
# kCAuthors, kTclAuthors, and kGifAuthors in this function.

# NOTES: HOW THE BOARD IS REPRESENTED:
# say, for 3x4 board
#   1  2  3  4   
#   5  6  7  8
#   9 10 11 12
# pos 1 == array[0]
# pos 2 == array[1]
# pos 3 == array[2]
# and so on...

global initialized
set initialized false

proc GS_InitGameSpecific {} {
    # puts "begin initgamespecific"

    ### Set the name of the game
    
    global kGameName
    set kGameName "QuickCross"

    # Authors Info
    global kRootDir
    global kCAuthors kTclAuthors kGifAuthors
    set kCAuthors "Thomas Yiu"
    set kTclAuthors "Jesse Phillips, Jennifer Lee, Rach Liu, Jeff Chiang"
    set kGifAuthors "$kRootDir/../bitmaps/DanGarcia-310x232.gif"

    ### Globals

    global CANVAS_WIDTH CANVAS_HEIGHT TILE_WIDTH TILE_HEIGHT BOARD_LENGTH BOARD_HEIGHT
    
    # LEFT_PADDING is the amount of space from the board left side to canvas left edge
    global LEFT_PADDING   
    # RIGHT_PADDING is the amount of space from the board right side to the canvas right edge
    global RIGHT_PADDING
    # TOP_PADDING is the amount of space from the board top side to canvas upper edge
    global TOP_PADDING   
    # BOTTOM_PADDING is the amount of space from the board bottom side to the canvas bottom edge
    global BOTTOM_PADDING

    global gFrameWidth gFrameHeight

    set CANVAS_WIDTH   [expr int([min $gFrameWidth $gFrameHeight])]
    set CANVAS_HEIGHT  $CANVAS_WIDTH
    set LEFT_PADDING   50
    set RIGHT_PADDING  50
    set TOP_PADDING    100
    set BOTTOM_PADDING 100

    set BOARD_LENGTH [expr $CANVAS_WIDTH - $LEFT_PADDING  - $RIGHT_PADDING]
    set BOARD_HEIGHT [expr $CANVAS_HEIGHT - $TOP_PADDING - $BOTTOM_PADDING]

    # board line vars

    # colours for board lines
    global BOARD_LINE_COLOUR
    set BOARD_LINE_COLOUR black
    global WIN_LINE_COLOR
    set WIN_LINE_COLOR red
    # thicknesses for board lines
    global BOARD_HORIZ_THICKNESS
    global BOARD_VERT_THICKNESS
    global WIN_LINE_THICKNESS
    set BOARD_HORIZ_THICKNESS 5
    set BOARD_VERT_THICKNESS 5
    set WIN_LINE_THICKNESS [expr 3 * $BOARD_VERT_THICKNESS]
    # board colour vars
    global BACK_COLOUR
    global BOARD_COLOUR
    set BACK_COLOUR honeydew
    set BOARD_COLOUR lightgrey

    # board width vars
    global BOARD_WIDTH
    set BOARD_WIDTH 0

    # pieces vars

    # piece buffering vars
    # this is the amount of space the each end of vert pieces are from tile each edge
    global VERT_PIECE_SPACE
    set VERT_PIECE_SPACE 15
    # amount of space each end of horiz piece is from each tile edge
    global HORIZ_PIECE_SPACE
    set HORIZ_PIECE_SPACE 15

    # piece thickness vars
    global VERT_PIECE_THICKNESS
    global HORIZ_PIECE_THICKNESS
    set VERT_PIECE_THICKNESS 15
    set HORIZ_PIECE_THICKNESS 15

    # piece color vars
    global PIECE_COLOUR
    set PIECE_COLOUR cyan

    # Shadow pieces
    global SHADOW_WIDTH_SCALE
    set SHADOW_WIDTH_SCALE 0.5

    global VERT_SHADOW_THICKNESS
    global VERT_SHADOW_COLOUR
    global VERT_SHADOW_SPACE_OFFSET
    set VERT_SHADOW_THICKNESS [expr $SHADOW_WIDTH_SCALE * $VERT_PIECE_THICKNESS]
    set VERT_SHADOW_COLOUR gray75
    set VERT_SHADOW_SPACE_OFFSET 3

    global HORIZ_SHADOW_THICKNESS
    global HORIZ_SHADOW_COLOUR
    global HORIZ_SHADOW_SPACE_OFFSET
    set HORIZ_SHADOW_THICKNESS [expr $SHADOW_WIDTH_SCALE * $HORIZ_PIECE_THICKNESS]
    set HORIZ_SHADOW_COLOUR gray75
    set HORIZ_SHADOW_SPACE_OFFSET 3

    #whoseturn variable
    global WHOSETURN
    set WHOSETURN 1

    # NUM_TILES_HORIZ is number of tiles horizontally
    global NUM_TILES_HORIZ
    # NUM_TILES_VERT is number of tiles vertically
    global NUM_TILES_VERT
    # LENTOWIN is hte length to win
    global LENTOWIN
    # NUM_SQUARES is the number of squares on the board
    global NUM_SQUARES

    set NUM_TILES_HORIZ 4
    set NUM_TILES_VERT  3
    set LENTOWIN 3
    set NUM_SQUARES [expr $NUM_TILES_HORIZ * $NUM_TILES_VERT]

    global BOARD_LENGTH BOARD_HEIGHT
    global TILE_WIDTH TILE_HEIGHT
    set TILE_WIDTH [expr $BOARD_LENGTH / $NUM_TILES_HORIZ]
    set TILE_HEIGHT [expr $BOARD_HEIGHT / $NUM_TILES_VERT]

    #array for the board
    global SHADOW_ARRAY
    for {set q 0} {$q < $NUM_SQUARES} {incr q} {
	lappend SHADOW_ARRAY 0
    }

    ### Set the initial position of the board

    global gInitialPosition gPosition
    set gInitialPosition 0
    set gPosition $gInitialPosition

    ### Set toWin and toMove
    global gMisereGame
    if { $gMisereGame } {
	set toWin1 "To Lose: "
    } else {
	set toWin1 "To Win: "
    }

    set toWin2 "Connect any 3 vertical or horizontal pieces in a row"

    SetToWinString [concat $toWin1 $toWin2]

    SetToMoveString  "To Move: Each player may either place a piece in an empty square or flip the bar vertically or horizontally. Click on a thin vertical or horizontal bar to place a vertical or horizontal piece in the square.  Click on a thick piece to flip it from horizontal to vertical."

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
# given game option.
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


# GS_NameOfPieces should return a list of 2 strings that represent
# your names for the "pieces".  If your game is some pathalogical game
# with no concept of a "piece", give a name to the game's sides.
# if the game is tic tac toe, this might be a single line: return [list x o]
# This function is called FIRST, ONCE, only when the player
# starts playing the game, and before he hits "New Game"
# The left player's piece should be the first item in the list.
# The right player's piece should be second.

proc GS_NameOfPieces {} {
    # puts "begin name of pieces"
    return [list x o]
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
    return [list black black]
}


proc min { a b } {
    if {$a < $b} {
	return $a
    }
    return $b
}

# GS_Initialize draws the graphics for the game on the canvas c
# You could put an opening animation or just draw an empty board.
# This function is called after GS_InitGameSpecific

proc GS_Initialize { c } {
    global initialized
    if { $initialized == false } {
	font create Winner -family arial -size 80
    }
    
    global CANVAS_WIDTH CANVAS_HEIGHT gFrameWidth gFrameHeight

    set CANVAS_WIDTH [expr int([min $gFrameWidth $gFrameHeight])]
    set CANVAS_HEIGHT $CANVAS_WIDTH

    # puts "begin of initialize"
    # you may want to start by setting the size of the canvas; this line isn't necessary
    $c configure -width $CANVAS_WIDTH -height $CANVAS_HEIGHT

    MakeBoard $c

    set initialized true
    # create font for Winner text at end of game
    #font create Winner -family arial -size 80
    #animationQualityQuery $c

    #start_animation $c

}

proc GS_Deinitialize { c } {
    $c delete all
    font delete "Winner"
}

proc MakeBoard { c } {
    global CANVAS_WIDTH CANVAS_HEIGHT
    global BOARD_LENGTH
    global BOARD_HEIGHT
    global LEFT_PADDING   
    global RIGHT_PADDING
    global TOP_PADDING   
    global BOTTOM_PADDING
    global NUM_TILES_HORIZ
    global NUM_TILES_VERT
    global NUM_SQUARES
    global TILE_WIDTH
    global TILE_HEIGHT

    # board colour vars
    global BACK_COLOUR
    global BOARD_COLOUR

    # board width vars
    global BOARD_WIDTH

    # puts "begin MakeBoard"

    $c delete all

    # Create giant backboard to hide things
    $c create rectangle 0 0 [expr $CANVAS_WIDTH - 1] [expr $CANVAS_HEIGHT - 1] -fill $BACK_COLOUR -width 0 -tag BACK

    # Create board rectangle
    $c create rectangle $LEFT_PADDING $TOP_PADDING [expr $LEFT_PADDING + $BOARD_LENGTH] [expr $TOP_PADDING + $BOARD_HEIGHT] -fill $BOARD_COLOUR -width $BOARD_WIDTH -tags {BOARD BOARD_BACK}

    # colours for board lines
    global BOARD_LINE_COLOUR

    # thicknesses for board lines
    global BOARD_HORIZ_THICKNESS
    global BOARD_VERT_THICKNESS
    global WIN_LINE_THICKNESS
    global WIN_LINE_COLOR
    # Draw horizontal board lines
    set x1 $LEFT_PADDING
    set x2 [expr $CANVAS_WIDTH - $RIGHT_PADDING]
    for { set i 0 } { $i <= $NUM_TILES_VERT } { incr i } {
	set y1 [expr $TILE_HEIGHT * $i + $TOP_PADDING]
	# in case want to have different y2 in future
	set y2 $y1

	$c create line $x1 $y1 $x2 $y2 -tag [list BOARD_LINES BOARD_HORIZ_LINE[expr $i+1]]  -width $BOARD_HORIZ_THICKNESS -fill $BOARD_LINE_COLOUR -capstyle round
    }

    # DRAW horizontal WIN LINES

    #horizontal wins starting in the first column of each row
    set x2 [expr $CANVAS_WIDTH - $RIGHT_PADDING - $TILE_WIDTH]
    for { set i 0 } {$i < $NUM_TILES_VERT } {incr i} {
	set y1 [expr $TILE_HEIGHT * $i + $TOP_PADDING + $TILE_HEIGHT / 3]
	set y2 $y1

	$c create line $x1 $y1 $x2 $y2 -tag [list ROW_WIN1$i WINS] -width $WIN_LINE_THICKNESS -fill $WIN_LINE_COLOR -capstyle round
	
    }

    set x2 [expr $CANVAS_WIDTH - $RIGHT_PADDING]
    set x1 [expr $LEFT_PADDING + $TILE_WIDTH]
    for {set i 0 } {$i < $NUM_TILES_VERT} {incr i} {
	set y1 [expr $TILE_HEIGHT * $i + $TOP_PADDING + $TILE_HEIGHT / 3]
	set y2 $y1	

	$c create line $x1 $y1 $x2 $y2 -tag [list ROW_WIN2$i WINS] -width $WIN_LINE_THICKNESS -fill $WIN_LINE_COLOR -capstyle round

    }

    # Draw vertical board lines
    set y1 $TOP_PADDING
    set y2 [expr $CANVAS_HEIGHT - $BOTTOM_PADDING]
    for { set i 0 } { $i <= $NUM_TILES_HORIZ} {incr i} {
	set x1 [expr $LEFT_PADDING + $i * $TILE_WIDTH]
	# in case want to have diff x2 in future (say, to rotate the board)
	set x2 $x1

	$c create line $x1 $y1 $x2 $y2 -tag [list BOARD_LINES BOARD_VERT_LINE[expr $i+1]] -width $BOARD_VERT_THICKNESS -fill $BOARD_LINE_COLOUR -capstyle round
    }

    # DRAW VERTICAL WIN LINES
    for {set i 0} {$i < $NUM_TILES_HORIZ} {incr i} {
	set x1 [expr $LEFT_PADDING + $TILE_HEIGHT / 3 + $i * $TILE_WIDTH]
	set x2 $x1

	$c create line $x1 $y1 $x2 $y2 -tag [list COL_WIN$i WINS] -width $WIN_LINE_THICKNESS -fill $WIN_LINE_COLOR -capstyle round
    }

    # DRAW DIAG WIN LINES INDIVIDUALLY

    # diag win line 1
    set a1 $LEFT_PADDING
    set b1 $TOP_PADDING
    set a2 [expr $LEFT_PADDING + $TILE_WIDTH]
    set a3 [expr $LEFT_PADDING + 3*$TILE_WIDTH]
    set a4 [expr $CANVAS_WIDTH - $RIGHT_PADDING]
    set b2 [expr $CANVAS_HEIGHT - $BOTTOM_PADDING]

    $c create line $a1 $b1 $a3 $b2 -tag [list DIAGWIN1 WINS] -width $WIN_LINE_THICKNESS -fill $WIN_LINE_COLOR -capstyle round
    $c create line $a2 $b1 $a4 $b2 -tag [list DIAGWIN2 WINS] -width $WIN_LINE_THICKNESS -fill $WIN_LINE_COLOR -capstyle round
    $c create line $a3 $b1 $a1 $b2 -tag [list DIAGWIN3 WINS] -width $WIN_LINE_THICKNESS -fill $WIN_LINE_COLOR -capstyle round
    $c create line $a4 $b1 $a2 $b2 -tag [list DIAGWIN4 WINS] -width $WIN_LINE_THICKNESS -fill $WIN_LINE_COLOR -capstyle round

    # Draw all pieces
    # NOTE: piece numbering starts at 0, not 1, so the piece numbers
    #       correspond to the array numbering in the mqx.c (see note at top of this file)

    # piece buffering vars
    global VERT_PIECE_SPACE
    global HORIZ_PIECE_SPACE

    # piece thickness vars
    global VERT_PIECE_THICKNESS
    global HORIZ_PIECE_THICKNESS

    # piece color vars
    global PIECE_COLOUR

    # Shadow pieces
    global VERT_SHADOW_THICKNESS
    global VERT_SHADOW_COLOUR
    global VERT_SHADOW_SPACE_OFFSET

    global HORIZ_SHADOW_THICKNESS
    global HORIZ_SHADOW_COLOUR
    global HORIZ_SHADOW_SPACE_OFFSET

    for { set i 0} { $i < $NUM_TILES_VERT } {incr i} {
	# puts "verts "
	#Draw all the vertical bar pieces "|"
	set y1 [expr $TOP_PADDING + $i * $TILE_HEIGHT]
	set y2 [expr $y1 + $TILE_HEIGHT]
	#Now, shorten y1 and y2 so piece is shorter than a full tile-height
	set y1 [expr $y1 + $VERT_PIECE_SPACE]
	set y2 [expr $y2 - $VERT_PIECE_SPACE]
	for {set j 0} {$j < $NUM_TILES_HORIZ } {incr j} {
	    #note: TILE_WIDTH/2 is integer division
	    set x1 [expr $LEFT_PADDING + $j * $TILE_WIDTH + $TILE_WIDTH/2]
	    set x2 $x1

	    # create the piece
	    set piece_num [expr $i*$NUM_TILES_HORIZ + $j]
	    $c create line $x1 $y1 $x2 $y2 -tag [list ALL_PIECES PIECES PIECES_VERT PIECE$piece_num PIECE_VERT$piece_num] -width $VERT_PIECE_THICKNESS -fill $PIECE_COLOUR -capstyle round
	    # create the shadow piece
	    $c create line $x1 [expr $y1+$VERT_SHADOW_SPACE_OFFSET] $x2 [expr $y2-$VERT_SHADOW_SPACE_OFFSET] -tag [list ALL_PIECES SHADOWS SHADOW$piece_num SHADOW_VERT$piece_num] -width $VERT_SHADOW_THICKNESS -fill $VERT_SHADOW_COLOUR -capstyle round
	}

	#Draw all the horizontal bar pieces "-"
	set y1 [expr $TOP_PADDING + $i * $TILE_HEIGHT + $TILE_HEIGHT/2]
	set y2 $y1
	for {set j 0} {$j < $NUM_TILES_HORIZ} {incr j} {
	    # puts "horiz"
	    set x1 [expr $LEFT_PADDING + $j * $TILE_WIDTH]
	    set x2 [expr $x1 + $TILE_WIDTH]
	    # now, "shorten" x1 x2 so that the piece length is less than tile-width
	    set x1 [expr $x1 + $HORIZ_PIECE_SPACE]
	    set x2 [expr $x2 - $HORIZ_PIECE_SPACE]

	    # create the piece
	    set piece_num [expr $i*$NUM_TILES_HORIZ + $j]
	    $c create line $x1 $y1 $x2 $y2 -tag [list ALL_PIECES PIECES PIECES_HORIZ PIECE$piece_num PIECE_HORIZ$piece_num] -width $HORIZ_PIECE_THICKNESS -fill $PIECE_COLOUR -capstyle round
	    # create the shadow piece
	    $c create line [expr $x1+$HORIZ_SHADOW_SPACE_OFFSET] $y1 [expr $x2-$HORIZ_SHADOW_SPACE_OFFSET] $y2 -tag [list ALL_PIECES SHADOWS SHADOW$piece_num SHADOW_HORIZ$piece_num] -width $HORIZ_SHADOW_THICKNESS -fill $HORIZ_SHADOW_COLOUR -capstyle round
	}

	# puts "test 0"
	set tileNum 0
	set vertTiles 0
	set horizTiles 0
	# puts "test 1"
	# puts "nume verts"
	# puts $vertTiles
	for {set j $TOP_PADDING} {$vertTiles < $NUM_TILES_VERT} {set j [expr $j + $TILE_HEIGHT]} {
	# puts "nume verts"
	# puts $vertTiles
	    # puts "test 3"
	    for {set k $LEFT_PADDING} {$horizTiles < $NUM_TILES_HORIZ} {set k [expr $k + $TILE_WIDTH]} {
		$c create rectangle $k $j [expr $k + $TILE_WIDTH] [expr $j + $TILE_HEIGHT] -fill $BOARD_COLOUR -tag [list TILES TILE$tileNum]
		set tileNum [expr $tileNum + 1]
		# puts "test 4"
		incr horizTiles
		# puts "test 5"
	    }
	    set horizTiles 0
	    incr vertTiles
	}
	
	# puts "test 2"
    }
    global SHADOW_ARRAY
    # put bindings on tiles
    for {set i 0} {$i < $NUM_SQUARES} {incr i} {
#	$c bind TILE$i <Enter> "RaiseShadows $c $i"
#	$c bind TILE$i <Leave> "$c lower SHADOW$i"
	$c bind SHADOW_HORIZ$i <ButtonRelease-1> "lset SHADOW_ARRAY $i 1; shrinkAnim $c SHADOW$i; $c raise PIECE_HORIZ$i; $c lower SHADOW$i; SendMove - $i;"
	$c bind SHADOW_VERT$i  <ButtonRelease-1> "lset SHADOW_ARRAY $i 1; shrinkAnim $c SHADOW$i; $c raise PIECE_VERT$i; $c lower SHADOW$i; SendMove | $i;"
	$c bind PIECE_HORIZ$i  <ButtonRelease-1> " $c lower PIECE_HORIZ$i TILES; anim_piece $c PIECE_HORIZ$i 90 20; $c raise PIECE_VERT$i; update idletasks; SendMove x $i;"
	$c bind PIECE_VERT$i   <ButtonRelease-1> " $c lower PIECE_VERT$i TILES; anim_piece $c PIECE_VERT$i 90 20; $c raise PIECE_HORIZ$i; update idletasks; SendMove x $i;"
    }

    $c lower BACK all
    $c lower WINS all
    $c raise BOARD all
    $c raise TILES
    $c raise BOARD_LINES
    
} 

proc wait {} {
    for {set j 0} {$j < 5000} {incr j} {
	update idletasks
    }
}

proc spinwait { n } {
    
    for {set j 0} {$j < $n} {incr j} {
	update idletasks
    }

}

proc RaiseShadows { c pieceNum } {
    global SHADOW_ARRAY
    # puts "begin raise shadows"
    if { [lindex $SHADOW_ARRAY $pieceNum] == 0 } {
	$c raise SHADOW$pieceNum
    }

}

# GS_DrawPosition this draws the board in an arbitrary position.
# It's arguments are a canvas, c, where you should draw and the
# (hashed) position.  For example, if your game is a rearranger,
# here is where you would draw pieces on the board in their correct positions.
# Imagine that this function is called when the player
# loads a saved game, and you must quickly return the board to its saved
# state.  It probably shouldn't animate, but it can if you want.

# BY THE WAY: Before you go any further, I recommend writing a tcl function that 
# UNhashes You'll thank yourself later.
# Don't bother writing tcl that hashes, that's never necessary.

#unhash:

proc expt { num power} {
    #puts "begin expt: with:num $num, power: $power"
    if {$power == 0} {
	return 1
    }
    return [expr $num * [expt $num [expr $power - 1]]]
} 

proc unhash { position } {
    set board {}
    #puts "begin unhash"
     if { $position >= 43046721 } {
	lappend board H
 	set position [expr {$position - 43046721}]
     } else {
	 lappend board V
     }

    # puts "-->unhash--> $board"

    for {set i 11} {$i >= 0} {set i [expr $i - 1]} {
	set ifV [expr {2 * [expt 3 $i]}]
	set ifH [expr {1 * [expt 3 $i]}]
	if {$position >= $ifV} {
	    lappend board 2
	    set position [expr {$position - $ifV}]
	} elseif {$position >= $ifH} {
	    lappend board 1
	    set position [expr {$position - $ifH}]
	} else {
	    lappend board 0
	}
	# puts "-->unhash--> $board"
    }  

    #arrange board
    set orderedBoard {}
    for {set i 12} {$i >= 0} {set i [expr $i - 1]} {
        lappend orderedBoard [lindex $board $i] 
    }    
    
    # puts "---> UNHASH PRODUCES --> $orderedBoard"

    return $orderedBoard
}

proc GS_DrawPosition { c position } {
    $c raise BOARD all
    $c raise BOARD_LINES all

    #puts "begin draw position"

    # BTW too: don't make any assumptions about the state of the board.
    set boardList [unhash $position]

    for {set i 0} {$i <= 11} {set i [expr $i + 1]} {
	set posi [lindex $boardList $i]
	if { $posi == 2 } {
	    $c itemconfig PIECE_VERT$i =fill black
	    $c raise PIECE_VERT$i all
	} elseif { $posi == 1 } {
	    $c itemconfig PIECE_HORIZ$i -fill black
	    $c raise PIECE_HORIZ$i all
	}
    }
}




# GS_NewGame should start playing the game. "let's play"  :)
# It's arguments are a canvas, c, where you should draw and
# the hashed starting position of the game.
# This is called just when the player hits "New Game"
# and before any moves are made.

proc GS_NewGame { c position } {
    # The default behavior of this funciton is just to draw the position
    # but if you want you can add a special behaivior here like an animation
    
    # puts "begin NEWGAME"
    global PIECE_COLOUR
    $c lower WINS all
    $c itemconfig PIECES -fill $PIECE_COLOUR 
    GS_DrawPosition $c $position
    # puts "--end NEWGAME--"
}


# GS_HandleMove draws (animates) a move being made.
# The user or the computer has just made a move, animate it or draw it
# or whatever.  Draw the piece moving if your game is a rearranger, or
# the piece appearing if it's a "dart board"

# By the way, if you animate, a function that will be useful for you is
# update idletasks.  You can call this to force the canvas to update if
# you make changes before tcl enters the event loop again.

# There three types of QX moves: 1) place a -, 2) place a |, 3) flip a - or |
# 
# 1) this is represented in theMove by a number 0 through [NUMSQUARES - 1].  The number
#    is the square at which to place a -.
#
# 2) this is represented in theMove by a number NUMSQUARES thru [2*NUMSQUARES - 1]. The number
#    is the square + NUMSQUARES at which to place a |.
#
# 3) this is represented by a number between 2*NUMSQUARES, and 3*NUMSQUARES-1.  The number is
#    the square (+ 2*NUMSQUARES) at which to flip the piece.
# 

proc GS_HandleMove { c oldPosition theMove newPosition } {
    global NUM_SQUARES

    #puts "*****  begin HandleMove  WITH MOVE $theMove"
    set oldBoard [unhash $oldPosition]
    set newBoard [unhash $newPosition]
    #puts "** OLD BOARD: $oldBoard"
    #puts "** NEW BOARD: $newBoard"


    if { $theMove < $NUM_SQUARES } {
	# thus, place a - at theMove
	$c lower SHADOW$theMove

	# raise the right piece
	$c raise PIECE_HORIZ$theMove
	#puts "******** placed a - at square $theMove (handlemove)"
	update idletasks
    } elseif { $theMove < [expr 2*$NUM_SQUARES] } {
	# thus, place a | at the square.
	set square [expr $theMove - $NUM_SQUARES]
	# lower the shadows at square
	$c lower SHADOW$square
	# raise the |
	$c raise PIECE_VERT$square
	#puts "****** placed a | piece at square $square (handlemove)"
	update idletasks
    } elseif { $theMove <= [expr 3*$NUM_SQUARES] } {
	# thus, flip the piece at the square
	set square [expr $theMove - [expr 2 * $NUM_SQUARES]]
	# figure out which piece is currently on top, and flip it with the other
	# NOTE: WE NEED TO MAKE SURE THAT oldPosition Holds the right stuff, and what it holds, THEN MAKE SURE what the value
	#       at that square is.
	set boardlist [unhash $oldPosition]
	#puts "** HANDLEMOVE BOARDLIST: $boardlist"
	set currentPiece [lindex $boardlist $square]
	# should add animation here.
	if { $currentPiece == 1} {
	    # thus flip to a |
	    #puts "**** FLIPPED A - TO A | (HandleMove)"
	    
	    $c lower PIECE_HORIZ$square TILES
	    
	    $c raise PIECE_VERT$square
	    
	    update idletasks
	} else {
	    #puts "******** FLIPPED A | TO A - (HandleMove)"
	    # thus flip to a -
	    
	    $c lower PIECE_VERT$square TILES
	    
	    $c raise PIECE_HORIZ$square
	    
	    update idletasks
	}
    } else {
	#puts "BAD ELSE: HandleMove, theMove > 3*NUM_SQUARES, this should never happen, code is broken somewhere"
    }

}

proc move { c type square } {
    global NUM_SQUARES

    # puts "begin move"

    if { $type == "-" } {
	GS_HandleMove $c 0 $square 0
    } elseif { $type == "|" } {
	set square [expr $NUM_SQUARES + $square]
	GS_HandleMove $c 0 $square 0
    } elseif { $type == "x"} {
	set square [expr $square + [expr 2 * $NUM_SQUARES]]
	GS_HandleMove $c 0 $square 0
    } else {
	#puts "BAD ELSE IN MOVE, move was not -, | or x"
    }
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
    set boardlist [unhash $position]

    # #puts "begin ShowMoves"
  
    foreach item $moveList {
	set move  [lindex $item 0]
	set value [lindex $item 1]
	set color cyan
	if {$moveType == "value"} {
	    if {$value == "Tie"} {
		set color yellow
	    } elseif {$value == "Lose"} {
		set color green
	    } else {
		set color red4
	    }
	}
	set theSquareNumber [expr $move % 12]
	if {$move < 12} {
	    $c itemconfig SHADOW_HORIZ$theSquareNumber -fill $color 
	    $c raise SHADOW_HORIZ$theSquareNumber all
	} elseif {$move < 24} {
	    $c itemconfig SHADOW_VERT$theSquareNumber -fill $color 
	    $c raise SHADOW_VERT$theSquareNumber all    
	} else {
	    set thePlacedPieceNum [lindex $boardlist $theSquareNumber]
	    if {$thePlacedPieceNum == 2} {
		$c itemconfig PIECE_VERT$theSquareNumber -fill $color
		$c raise PIECE_VERT$theSquareNumber all
	    } else {
		$c itemconfig PIECE_HORIZ$theSquareNumber -fill $color
		$c raise PIECE_HORIZ$theSquareNumber all
	    }
	}
	update idletasks
    }
    
}



# GS_HideMoves erases the moves drawn by GS_ShowMoves.  It's arguments are the same as GS_ShowMoves.
# You might not use all the arguments, and that's okay.

proc GS_HideMoves { c moveType position moveList} {
    # #puts "begin HIDEMOVES"
     #set color cyan
     set boardlist [unhash $position]
     foreach item $moveList {
 	set move  [lindex $item 0]
 	set theSquareNumber [expr $move % 12]

 	if {$move < 12} {
 	    #$c itemconfig SHADOW_VERT$theSquareNumber -fill $color 
 	    $c lower SHADOW_VERT$theSquareNumber
 	} elseif {$move < 24} {
 	    #$c itemconfig SHADOW_HORIZ$theSquareNumber -fill $color 
 	    $c lower SHADOW_HORIZ$theSquareNumber    
 	} else {
 	    #set color black
 	    set thePlacedPieceNum [lindex $boardlist $theSquareNumber]
 	    if {$thePlacedPieceNum == 2} {
 		#$c itemconfig PIECE_VERT$theSquareNumber -fill $color
 	    } else {
 		#$c itemconfig PIECE_HORIZ$theSquareNumber -fill $color
 	    }
 	}
 	update idletasks
     }
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
    global NUM_SQUARES

    #GS_DrawPosition $c $positionAfterUndo
   
    # #puts "begin HandleUndo"

    update idletasks

    set afterUndoBoard [unhash $positionAfterUndo]
    if { [expr $theMoveToUndo < $NUM_SQUARES] } {
	$c lower PIECE_HORIZ$theMoveToUndo all
	$c raise SHADOW$theMoveToUndo
    } elseif { $theMoveToUndo < [expr 2 * $NUM_SQUARES] } {
	set square [expr $theMoveToUndo - $NUM_SQUARES]
	$c lower PIECE_VERT$square all
	$c raise SHADOW$square
    } elseif { $theMoveToUndo < [expr 3 * $NUM_SQUARES] } {
	set square [expr $theMoveToUndo - 2 * $NUM_SQUARES]
	if { [lindex $afterUndoBoard $square] == 2 } {
	    ##puts "flip a horiz to a vert"
	    anim_piece $c PIECE_HORIZ$square 90 20
	    $c lower PIECE_HORIZ$square TILES
	    $c raise PIECE_VERT$square
	    update idletasks
	} else {
	    ##puts "flip the vert to a horiz"
	    anim_piece $c PIECE_VERT$square 90 20
	    $c lower PIECE_VERT$square TILES
	    $c raise PIECE_HORIZ$square
	    update idletasks
	}
    } else {
	#puts "ERROR: badelse from GS_HandleUndo: theMoveToUndo was > NUM_SQUARES = $NUM_SQUARES"
    }
   
}





# GS_GetGameSpecificOptions is not quite ready, don't worry about it .
proc GS_GetGameSpecificOptions { } {
}




#both players are able to use all pieces on the board whether they are - or |...whose turn depends on whether the
#internal position is represented as a negative or positive number according to the function PositionToBlankHV in
#the c code.  i am unclear of who the -/+ representations correspond to and how whose turn is represented.
proc GS_WhoseMove { position } {
     global WHOSETURN
     # #puts "begin whoseMOve given: $position"
     if { $WHOSETURN != "o" } {
 	set $WHOSETURN "x"
 	# #puts "-- whoseMove returns o"
 	return o
     } else {
 	set $WHOSETURN "o"
 	# #puts "--whoseMove returns x"
 	return x
     }
    
 }

 # GS_GameOver is called the moment the game is finished ( won, lost or tied)
 # you could use this function to draw the line striking out the winning row in tic tac toe for instance
 # or you could congratulate the winner or do nothing if you want.

 proc GS_GameOver { c position gameValue nameOfWinningPiece nameOfWinner lastMove} {
     global NUM_SQUARES NUM_TILES_HORIZ LENTOWIN CANVAS_WIDTH CANVAS_HEIGHT
     $c itemconfig PIECES -fill black

     $c create text [expr $CANVAS_WIDTH / 2] [expr $CANVAS_HEIGHT / 2 - 30] -text "$nameOfWinner\n" -font Winner -fill green -tags winner
     $c create text [expr $CANVAS_WIDTH / 2] [expr $CANVAS_HEIGHT / 2 + 30] -text "WINS!"         -font Winner -fill green -tags winner      

     # FIRST: get the winning square
     if { $lastMove < $NUM_SQUARES } {
	 set winningSquare $lastMove
     } elseif { $lastMove < [expr 2 * $NUM_SQUARES] } {
	 set winningSquare [expr $lastMove - $NUM_SQUARES]
     } elseif { $lastMove < [expr 3 * $NUM_SQUARES] } {
	 set winningSquare [expr $lastMove - 2 * $NUM_SQUARES]
     } else {
	 #puts "BAD ELSE-- GS_GameOver: lastMove is an invalid move"
     }
     # SECOND: get the winning piece (- or |, 1 or 2)
     set boardList [unhash $position]
     set winningPiece [lindex $boardList $winningSquare]
     
     # THIRD: find-out which row and column this square is in
     set myRow [expr $winningSquare / $NUM_TILES_HORIZ ]
     set myColumn [expr $winningSquare % $NUM_TILES_HORIZ]
     

     #puts "row: $myRow, col: $myColumn, winningPiece: $winningPiece, winningSquare: $winningSquare"
     #puts "board: $boardList"

     # FOURTH: look for wins in myRow
     # NOTE: we're assuming a row size of at most 4.
     set rowWin [checkRowWin $myRow $winningPiece $boardList]
     if { [lindex $rowWin 0] != -1 } {
	 #bring up the winning slash for this row
	 #puts "win on ROW_WIN$rowWin$myRow"
	 raiseWinningTiles $c $rowWin
	 return
     }

     #FIFTH: look for wins in myColumn
     set colWin [checkColumnWin $myColumn $winningPiece $boardList]
     if { [lindex $colWin 0] != -1 } {
	 #raise the appropriate slash for this column
	 #puts "win on COL_WIN$myColumn$colWin"
	 raiseWinningTiles $c $colWin
	 return
     }
     
     #SIXTH: check diags for wins
     set diagWin [checkDiagWin $myRow $myColumn $winningPiece $boardList]
     if { [lindex $diagWin 0] != -1 } {
	 # raise the appropriate slash for the given diag
	 #puts "DIAGWIN$diagWin"
	 raiseWinningTiles $c $diagWin
	 return
     }

     #puts "BAD ELSE: GS_GAMEOVER, no win was found"

 }

# check the row if a win has been achieved on the given boardList
#  where the win is of the given length.
# If ther is a win on the given row, returns the first square num
# in the win, otherwise, it returns 0

proc checkRowWin {rowNum winPiece board} {
    global NUM_TILES_HORIZ
    set winningSquare -1
    # sqr1 is the first square in this row
    set sqr1 [expr $rowNum * $NUM_TILES_HORIZ]
    set sqr2 [expr $sqr1 + 1]
    set sqr3 [expr $sqr1 + 2]
    set sqr4 [expr $sqr1 + 3]
    # lastSquare is the last Square in this row
    set lastSqr [expr ($rowNum + 1) * $NUM_TILES_HORIZ - 1]
    #puts "sqr1: $sqr1, sqr2: $sqr2, sqr3: $sqr3, sqr4: $sqr4"
    # p1 represents the piece at first square of this row, px represents the piece at the xth square of this row
    set p1 [lindex $board $sqr1]
    set p2 [lindex $board $sqr2]
    set p3 [lindex $board $sqr3]
    set p4 [lindex $board $sqr4]
    #puts "p1: $p1, p2: $p2, p3: $p3, p4: $p4"
    if {[expr $p1 == $winPiece && $p2 == $winPiece && $p3 == $winPiece]} {
	# a win starting from the first square of this row
	return [list $sqr1 $sqr2 $sqr3]
    } elseif { [expr $p2 == $winPiece && $p3 == $winPiece && $p4 == $winPiece] } {
	# a win starting from the 2nd square of this row
	return [list $sqr2 $sqr3 $sqr4]
    } else {
	# no win in this row
	#puts "CHECK ROW WIN RETURNS 0"
	return [list -1 -1 -1]
    }
}

# check the column if a win has been achieved on the given boardList
#  where the win is of the given length.
# If ther is a win on the given row, returns the first square num
# in the win, otherwise, it returns 0

proc checkColumnWin {colNum winPiece board} {
    global NUM_TILES_HORIZ NUM_TILES_VERT
    # sqr1 is the first square in this column
    set sqr1 $colNum
    set sqr2 [expr $sqr1 + $NUM_TILES_HORIZ]
    set sqr3 [expr $sqr2 + $NUM_TILES_HORIZ]
    set sqr4 [expr $sqr3 + $NUM_TILES_HORIZ]
    # p1 represents the piece at first square of this column, px represents the piece at the xth square of this column
    set p1 [lindex $board $colNum]
    set p2 [lindex $board [expr $colNum + $NUM_TILES_HORIZ]]
    set p3 [lindex $board [expr $colNum + 2 * $NUM_TILES_HORIZ]]
    set p4 [lindex $board [expr $colNum + 3 * $NUM_TILES_HORIZ]]
    #puts "CHECK COLUMN WIN:"
    #puts "p1: $p1, p2: $p2, p3: $p3, p4: $p4"
    if { $p1 == $winPiece && $p2 == $winPiece && $p3 == $winPiece } {
	#puts "checkColumnWin returns 1"
 	set result [list $sqr1 $sqr2 $sqr3]
    } elseif { $NUM_TILES_VERT == 4 && $p2 == $winPiece && $p3 == $winPiece && $p4 == $winPiece } {
	#puts "checkColumnWin returns 2"
 	set result [list $sqr2 $sqr3 $sqr4]
     } else {
	#puts "checkColumnWin returns 0"
	 set result [list -1 -1 -1]
     }
     return $result
}


# check all diags if a win has been achieved on the given boardList
# If there is a win on a diag, returns the first square num
# in the win, otherwise, it returns 0

proc checkDiagWin {rowNum colNum winPiece board} {
    global NUM_TILES_HORIZ NUM_TILES_VERT
    # we are assuming a 3x4 board.  This may be generalized in the future.
    # however, since I need a working product tonight, I must do this now 11/11/03
    
    # pi is the piece at the ith square of the board
    set p0 [lindex $board 0]
    set p1 [lindex $board 1]
    set p2 [lindex $board 2]
    set p3 [lindex $board 3]
    set p5 [lindex $board 5]
    set p6 [lindex $board 6]
    set p8 [lindex $board 8]
    set p9 [lindex $board 9]
    set p10 [lindex $board 10]
    set p11 [lindex $board 11]

    #check diag 1
    if { $p0 == $winPiece && $p5 == $winPiece && $p10 == $winPiece} {
	return [list 0 5 10]
    } elseif { $p1 == $winPiece && $p6 == $winPiece && $p11 == $winPiece } {
	return [list 1 6 11]
    } elseif { $p2 == $winPiece && $p5 == $winPiece && $p8 == $winPiece } {
	return [list 2 5 8]
    } elseif { $p3 == $winPiece && $p6 == $winPiece && $p9 == $winPiece } {
	return [list 3 6 9]
    } else {
	return [list -1 -1 -1]
    }
}


proc raiseWinningTiles {c squareList} {
    #puts "raiseWinningTiles called with $squareList"
    foreach item $squareList {
	$c itemconfig TILE$item -fill gray75
	$c raise TILE$item BOARD
    }
}

#     global LENTOWIN NUM_TILES_HORIZ
#     set winner 0

#     #puts "begin gameOver"

#     set board [unhash $position]
#     if { $nameOfWinningPiece == "-" } {
# 	set winner 1
#     } elseif { $nameOfWinningPiece == "|" } {
# 	set winner 2
#     } else {
# 	##puts "BAD ELSE: GS_GameOver, nameOfWinningPiece != - or |"
#     }
    
#     #win on rows
#     for {set i 1} {$i < [expr $NUM_TILES_VERT * $NUM_TILES_HORIZ]} {set i [expr $i + $NUM_TILES_HORIZ]} {
# 	for {set j $i} {$j <= [expr $i + $NUM_TILES_HORIZ - 3]} {incr j} {
# 	    if {[lindex $board $j] == $winner && [lindex $board [expr $j + 1]] == $winner && [lindex $board [expr $j + 2]] == $winner} {set p {$j [expr $j + 3]}}}}

#     #win on columns
#     for {set i 1} {$i <= $NUM_TILES_HORIZ} {incr i} {
# 	for {set j $i} {$j <= [expr $NUM_TILES_VERT * $NUM_TILES_HORIZ]} {set j [expr $j + 4]} {
# 	    if {[expr $j + [expr 2 * $NUM_TILES_HORIZ]] <= [expr $NUM_TILES_VERT * $NUM_TILES_HORIZ]} {
# 		if {[lindex $board $j] == $winner && [lindex $board [expr $j + $NUM_TILES_HORIZ]] == $winner && [lindex $board [expr $j + [expr 2 * $NUM_TILES_HORIZ]]] == $winner} {set p {$j [expr $j + [expr 2 * $NUM_TILES_HORIZ]]}}}}}

#     # win on diags L 2 R
#     for {set i 0} {$i <= [expr $NUM_TILES_VERT - $LENTOWIN]}{incr i} {
# 	set startSqr [expr $i * $NUM_TILES_HORIZ]
# 	for {set j 0}{$j <= [expr $NUM_TILES_HORIZ - $LENTOWIN]}{incr j} {
# 	    set diagStrt [expr $startSqr + $j]
# 	    set diagResult [DiagWin $board $diagStrt $LENTOWIN "L2R" $winner]
# 	    if {[lindex $diagResult 0] == "WIN"} {
# 		set p {[lindex $diagResult 1] [lindex $diagResult 2]}
# 	    }
# 	}
#     }

#     #win on diags R2L
#     for {set i [expr $LENTOWIN - 1]}{$i < $NUM_TILES_HORIZ}{incr i}{
# 	for {set j 0}{$j <= [expr $NUM_TILES_VERT - $LENTOWIN]}{incr j}{
# 	    set startSqr [expr $i + [expr $j * $NUM_TILES_HORIZ]]
# 	    set diagResult [DiagWin $board $startSqr $LENTOWIN "R2L" $winner]
# 	    if {[lindex $diagResult 0] == "WIN"} {
# 		set p {[lindex $diagReslt 1] [lindex $diagResult 2]}
# 	    }
# 	}
#     }

#     for {set i 0} {$i < $LENTOWIN} {incr i} {
# 	set t [lindex $p $i]
# 	$c itemconfig TILE$t -fill yellow
# 	if { $nameOfWinningPiece == "-" } {
# 	    $c raise PIECE_HORIZ$i
# 	} elseif { $nameOfWinningPiece == "|" } {
# 	    $c raise PIECE_VERT$i
# 	}
#     }
#     $c create text 250 160 -text "$nameOfWinner" -font Winner -fill orange -tags winner
#     $c create text 250 340 -text "WINS!"         -font Winner -fill orange -tags winner

proc DiagWin { board square length direction winner} {
    global NUM_TILES_HORIZ
    set mod 0
    if { $direction == "R2L" } {
	set mod -1
    } else {
	set mod 1
    }

    set result {"LOSE" 0 0}

    for {set i 0}{$i < length}{set i [expr $i + $NUM_TILES_HORIZ + mod]}{
	set checkSqr [expr $square + $i]
	if {[lindex $board $checkSqr] != $winner}{
	    return result
	}
    }

    set result { "WIN" $square $i }
    return result
}


# GS_UndoGameOver is called then the player hits undo after the game is finished.
# this is provided so that you may undo the drawing you did in GS_GameOver if you drew something.
# for instance, if you drew a line crossing out the winning row in tic tac toe, this is where you sould delete the line.

# note: GS_HandleUndo is called regardless of whether the move undoes the end of the game, so IF you choose to do nothing in
# GS_GameOver, you needn't do anything here either.

proc GS_UndoGameOver { c position } {
    global PIECE_COLOUR BOARD_COLOUR
    $c lower WINS all
    $c itemconfig PIECES -fill $PIECE_COLOUR
    $c itemconfig TILES -fill $BOARD_COLOUR
    $c delete winner
}

# Animation that "flies" a given list of objects; it displaces them xDistance units in the x direction at speed xSpeed, and yDistance units in the y direction at speed ySpeed.
# The last parameter is an optionals list that contains lists of commands to be evaluated by the tcl eval command, the commands are executed on every xSpeed increment (ie every "step" of the animation.

proc Fly { c label_list xDistance yDistance xSpeed ySpeed optionals} {

    #puts "begin fly"

# testing line
# .c create text 20 20 -text "A" -font {Helvetica 100} -fill red -tags {A word}

    set xIncr 1
    set yIncr 1

    # error handling
    # in case xSpeed is not zero even though xDistance is zero
    if { $xDistance == 0 && $xSpeed != 0} {
	set xSpeed 0
    }

    # in case ySpeed is not zero even though yDistance is zero
    if { $yDistance == 0 && $ySpeed != 0} {
	set ySpeed 0
    }


    if { $xDistance < 0 } {
	set xIncr [expr -1 * $xIncr]
	set xDistance [expr -1 * $xDistance]
    }

    if { $yDistance < 0 } {
	set yIncr [expr -1 * $yIncr]
	set yDistance [expr -1 * $yDistance]
    }

    while { $xDistance > 0 || $yDistance > 0 } {

	foreach item $label_list {
	    $c move $item [expr $xIncr*$xSpeed] [expr $yIncr*$ySpeed]
	}

	foreach item $optionals {
	    eval $item
	}

	#$c move $label_list [expr $xIncr * $xSpeed] [expr $yIncr * $ySpeed]

	set xDistance [expr $xDistance - $xSpeed]
	set yDistance [expr $yDistance - $ySpeed]

	if {$xDistance <= 0} {
	    set xSpeed 0
	}
	if {$yDistance <= 0} {
	    set ySpeed 0
	}

	update idletasks
    }
}


#using end-coordinates instead of moving abs distances
# NOTES: expr expressions used for varible coordinates MUST have no spaces after [expr, otherwise
#   will not be parsed correctly
# Give absolute values for xSpeed and ySpeed!
proc FlyGroupCoord { c group optionals globals_list} {

    # initiliaze all globals in globals_list
 #   puts globals_list:$globals_list
    set glength [llength $globals_list]
    set i 0
    while {$i < $glength} {
	set gVar [lindex $globals_list $i]
#	puts gVar:$gVar
	global $gVar
	set i [expr $i+1]
    }

    set xIncr 1
    set yIncr 1

    set num_to_go [llength $group]
    set group_size [llength $group]

    set i 0
    while { $i < $group_size } {

	set item [lindex $group $i]
	set name [lindex $item 0]

	# get numbers, code for handling expressions correctly when parsing

	set xFinal [lindex $item 1]
	if {[string equal $xFinal "\[expr"]} {
	    # this is an expression, not just a number of a straight $varname, so parse it
	    set part2 [lindex $item 2]
	    # concatenate, use concat b/c it puts a space inbetween automatically for us
	    set theExpr [concat $xFinal $part2]
	    # this looks something like: [expr [expr var]]
	    set xFinal [expr $theExpr]
	    lset item 1 $xFinal
	    # now move the elements down one index, killing element 2
	    # note: don't care that theres an "extra" undeleted element at the end now
	    set end [expr [llength $item]-1]
	    set k 2
	    while {$k < $end} {
		lset item [expr $k] [lindex $item [expr $k+1]]
		#puts "k:$k, item: $item"
		set k [expr $k+1]
	    }
	    #puts item:$item
	}

	set yFinal [lindex $item 2]
	if {[string equal $yFinal "\[expr"]} {
	    set part2 [lindex $item 3]
	    set theExpr [concat $yFinal $part2]
	    set yFinal [expr $theExpr]
	    lset item 2 $yFinal
	    set end [expr [llength $item]-1]
	    set k 3
	    while {$k < $end} {
		lset item [expr $k] [lindex $item [expr $k+1]]
		#puts "k:$k, item: $item"
		set k [expr $k+1]
	    }
	    #puts item:$item
	}

	set xSpeed [lindex $item 3]
	if {[string equal $yFinal "\[expr"]} {
	    set part2 [lindex $item 4]
	    set theExpr [concat $xSpeed $part2]
	    set xSpeed [expr $theExpr]
	    lset item 3 $xSpeed
	    set end [expr [llength $item]-1]
	    set k 4
	    while {$k < $end} {
		lset item [expr $k] [lindex $item [expr $k+1]]
		#puts "k:$k, item: $item"
		set k [expr $k+1]
	    }
	    #puts item:$item
	}

	set ySpeed [lindex $item 4]
	if {[string equal $yFinal "\[expr"]} {
	    set part2 [lindex $item 5]
	    set theExpr [concat $ySpeed $part2]
	    set ySpeed [expr $theExpr]
	    lset item 4 $ySpeed
	    set end [expr [llength $item]-1]
	    set k 5
	    while {$k < $end} {
		lset item [expr $k] [lindex $item [expr $k+1]]
		#puts "k:$k, item: $item"
		set k [expr $k+1]
	    }
	    #puts item:$item
	}

	set inner_optionals [lindex $item 5]

	set theCoords [$c coords $name]
	set xNow [lindex $theCoords 0]
	set yNow [lindex $theCoords 1]

	#----debug--------
	#puts theCoord:$theCoords
	#puts xNow:$xNow
	#puts yNow:$yNow
	#puts xFinal:$xFinal
	#puts yFinal:$yFinal
	#puts xFinal:[expr $xFinal]
	#puts yFinal:[expr $yFinal]
	#puts xSpeed:$xSpeed
	#puts ySpeed:$ySpeed
	#------------------

	set xDistance [expr $xFinal - $xNow]
	set yDistance [expr $yFinal - $yNow]

	puts xDistance:$xDistance
	puts yDistance:$yDistance

	if {$xDistance != 0} {
	    set signX [expr $xDistance/abs($xDistance)]
	    set xSpeed [expr $signX*$xSpeed]
	}

	if {$yDistance != 0} {
	    set signY [expr $yDistance/abs($yDistance)]
	    set ySpeed [expr $signY*$ySpeed]
	}

	set xDistance [expr abs($xDistance)]
	set yDistance [expr abs($yDistance)]

	# more debug
	#puts abs(xDistance):$xDistance
	#puts abs(yDistance):$yDistance

	lset group $i [list $name $xDistance $yDistance $xSpeed $ySpeed $inner_optionals]

	set i [expr $i+1]
    }

    while { $num_to_go > 0 } {

	set i 0

	while { $i < $group_size } {
	    set item [lindex $group $i]	 
	    
	    set name [lindex $item 0]

	    #puts "-------CURRENTLY ON: $name --------"

	    set xDistance [lindex $item 1]
	    set yDistance [lindex $item 2]

	    if {$xDistance == "DONE"} {
		set i [expr $i+1]
		foreach inner $inner_optionals {
		    eval $inner
		}
#		eval $inner_optional
		continue
	    }

	    set xSpeed [lindex $item 3]
	    set ySpeed [lindex $item 4]

	    set inner_optionals [lindex $item 5]

	    $c move $name [expr $xIncr*$xSpeed] [expr $yIncr*$ySpeed]
	    
	    foreach inner $inner_optionals {
		eval $inner
	    }
#	    eval $inner_optionals
	    
	    set xDistance [expr $xDistance - abs($xSpeed)]
	    set yDistance [expr $yDistance - abs($ySpeed)]

	    # debug
	    #puts $xDistance
	    #puts $yDistance

	    if {$xDistance <= 0} {
		# correct overshoot!
		if {$xDistance < 0 && $xSpeed != 0} {
	            $c move $name [expr (-1*$xSpeed/abs($xSpeed))*abs($xDistance) ] 0
		}
		set xSpeed 0
	    }
	    if {$yDistance <= 0} {
		# correct overshoot!
		if {$yDistance < 0 && $ySpeed != 0} {
	            $c move $name 0 [expr (-1*$ySpeed/abs($ySpeed))*abs($yDistance) ]
		}
		set ySpeed 0
	    }
	    if {$xDistance <= 0 && $yDistance <= 0} {
		#debug
		#puts "deleting 1 from num_to_go"
		#puts "--- yDistance: $yDistance"
		#puts "--- xDistance: $xDistance"
		set num_to_go [expr $num_to_go - 1]
		set xDistance "DONE"
	    }

	    lset group $i [list $name $xDistance $yDistance $xSpeed $ySpeed]

	    set i [expr $i+1]
	}

	foreach item $optionals {
	    eval $item
	}

	update idletasks
    }

    #puts $group
    #puts $num_to_go
}

proc random_colour {c object_list} {
    # #puts "begin random color"

    set i [expr rand()]
    while {$i < 1} {
	set i [expr $i*10]
    }
    set i [expr round($i)%10]

    switch $i {
	1 {set colour black}
	2 {set colour red}
	3 {set colour blue}
	4 {set colour green}
	5 {set colour yellow}
	6 {set colour brown}
	7 {set colour pink}
	8 {set colour purple}
	9 {set colour orange}
	10 {set colour cyan}
	default {set colour black}
    }


   foreach object $object_list {
	$c itemconfig $object -fill $colour
    }
}

proc start_animation { c } {

    # #puts "begin start_animation"

    .c create text -200 20 -text "QUICKCROSS" -font {Helvetica 40} -fill black -tags {word quickcross}

    Fly $c quickcross 450 30 15 3 {{random_colour $c quickcross}}
}


# the following start animation is too slow to run over ssh, excellent speed
# on unix though
proc start_animation_complex { c } {
    global CANVAS_WIDTH CANVAS_HEIGHT
    global BOARD_LENGTH
    global BOARD_HEIGHT
    global LEFT_PADDING   
    global RIGHT_PADDING
    global TOP_PADDING   
    global BOTTOM_PADDING
    global NUM_TILES_HORIZ
    global NUM_TILES_VERT
    global TILE_WIDTH
    global TILE_HEIGHT

    # note: these globals have to go into fly procs
    #   is there a way around this? 
    #   The problem is that these vars aren't passed to fly procs as args,
    #   the fly procs eval the parsed group arg, so it seems like
    #   making these global is the only way to go.
    #  these globals are put into a list, and passed to FlyGroupCoord, then FlyGroupCoord can individually declare each global in the list as global
    global SPACEOUT
    global LETTERWIDTH
    global TitleX
    global TitleY

    set SPACEOUT 100
    set LETTERWIDTH 40
    set TitleX [expr $CANVAS_WIDTH/5]
    set TitleY [expr $CANVAS_HEIGHT/8]

    global TitleFont
    global TitleSize
    set TitleFont Helvetica
    set TitleSize 40

    set TitleFontList [list $TitleFont $TitleSize]

    .c create text [expr -1*$SPACEOUT] [expr -1.0*$CANVAS_HEIGHT/3] -text "Q" -font $TitleFontList -fill black -tags {words quickcross qinline q q1}
    .c create text [expr $TitleX+$LETTERWIDTH] [expr $CANVAS_HEIGHT+$SPACEOUT] -text "U" -font $TitleFontList -fill black -tags {words quickcross qinline u u1}
    .c create text [expr 0.8*$CANVAS_WIDTH] [expr -1.0*$SPACEOUT] -text "I" -font $TitleFontList -fill black -tags {words quickcross qinline i i1}
    .c create text [expr 0.2*$CANVAS_WIDTH] [expr $CANVAS_HEIGHT+2*$SPACEOUT] -text "CK" -font $TitleFontList -fill black -tags {words quickcross qinline ck ck1}
    .c create text [expr -3*$SPACEOUT] $TitleY -text "CR" -font $TitleFontList -fill black -tags {words quickcross qinline cr cr1}
    .c create text [expr $TitleX+7*$LETTERWIDTH] [expr -4*$SPACEOUT] -text "O" -font $TitleFontList -fill black -tags {words quickcross qinline o o1}
    .c create text [expr $TitleX+8*$LETTERWIDTH] [expr $CANVAS_HEIGHT+2*$SPACEOUT] -text "S" -font $TitleFontList -fill black -tags {words quickcross qinline s1 s1_1}
    .c create text [expr $CANVAS_WIDTH+3*$SPACEOUT] $TitleY -text "S" -font $TitleFontList -fill black -tags {words quickcross qinline s2 s2_1}

    global line1xB
    global line1xF
    global line1y
    global line1length
    global line1width
    set line1xB [expr -4*$SPACEOUT]
    set line1xF [expr $CANVAS_WIDTH+4*$SPACEOUT]
    set line1y [expr $CANVAS_HEIGHT/9]
    set line1length 300
    set line1width 12
    .c create line $line1xB $line1y [expr $line1xB+$line1length] $line1y -fill blue2 -tag {lines line1} -width $line1width

    global line2xB
    global line2xF
    global line2y
    global line2length
    global line2width
    set line2xB [expr -3.5*$SPACEOUT]
    set line2xF [expr $CANVAS_WIDTH+3.5*$SPACEOUT]
    set line2y [expr $CANVAS_HEIGHT*0.3]
    set line2length 250
    set line2width 6
    .c create line $line2xB $line2y [expr $line2xB+$line2length] $line2y -fill green2 -tag {lines line2} -width $line2width

    global line3xB
    global line3xF
    global line3y
    global line3length
    global line3width
    set line3xF [expr -4*$CANVAS_WIDTH]
    set line3xB [expr $CANVAS_WIDTH+4*$SPACEOUT]
    set line3y [expr $CANVAS_HEIGHT*0.7]
    set line3length 300
    set line3width 8
    .c create line $line3xB $line3y [expr $line3xB+$line3length] $line3y -fill blue2 -tag {lines line3} -width $line3width

    global line4x
    global line4yB
    global line4yF
    global line4length
    global line4width
    set line4x [expr $CANVAS_WIDTH*0.4]
    set line4yB [expr -4*$SPACEOUT]
    set line4yF [expr $CANVAS_WIDTH+$SPACEOUT]
    set line4length 200
    set line4width 15
    .c create line $line4x $line4yB $line4x [expr $line4yB+$line4length] -fill red -tag {lines line4} -width $line4width

    global line5x
    global line5yB
    global line5yF
    global line5length
    global line5width
    set line5x [expr $CANVAS_WIDTH*0.7]
    set line5yF [expr -2*$SPACEOUT]
    set line5yB [expr $CANVAS_WIDTH+2*$SPACEOUT]
    set line5length 100
    set line5width 20
    .c create line $line5x $line5yB $line5x [expr $line5yB-$line5length] -fill gold -tag {lines line5} -width $line5width

    global line6x
    global line6yB
    global line6yF
    global line6length
    global line6width
    set line6x [expr $CANVAS_WIDTH*0.2]
    set line6yF [expr -2*$SPACEOUT]
    set line6yB [expr $CANVAS_WIDTH+2*$SPACEOUT]
    set line6length 150
    set line6width 15
    .c create line $line6x $line6yB $line6x [expr $line6yB-$line6length] -fill gold -tag {lines line6} -width $line6width    

    global line7xB
    global line7xF
    global line7y
    global line7length
    global line7width
    set line7xB [expr -3.5*$SPACEOUT]
    set line7xF [expr $CANVAS_WIDTH+3.5*$SPACEOUT]
    set line7y [expr $CANVAS_HEIGHT*0.4]
    set line7length 250
    set line7width 8
    .c create line $line7xB $line7y [expr $line7xB+$line7length] $line7y -fill green2 -tag {lines line7} -width $line7width

    global line8x
    global line8yB
    global line8yF
    global line8length
    global line8width
    set line8x [expr $CANVAS_WIDTH*0.8]
    set line8yF [expr -2*$SPACEOUT]
    set line8yB [expr $CANVAS_WIDTH+2*$SPACEOUT]
    set line8length 150
    set line8width 10
    .c create line $line8x $line8yB $line8x [expr $line8yB-$line8length] -fill gold -tag {lines line8} -width $line8width    

    set lineglobals {line1xB line1xF line1y line1length line2xB line2xF line2y line2length line3xB line3xF line3y line3length line4x line4yB line4yF line4length line5x line5yB line5yF line5length line1width line2width line3width line4width line5width line6x line6yB line6yF line6length line6width line7xB line7xF line7y line7length line7width line8x line8yB line8yF line8length line8width} 
    set titleglobals {SPACEOUT LETTERWIDTH TitleX TitleY}

    set globs [concat $lineglobals $titleglobals]

    # animate the lines and words simulataneously
    #FlyGroupCoord $c { {line1 $line1xF $line1y 5 5 {} } {line2 $line2xF $line2y 3 3 {} } {line3 $line3xF $line3y 7 7 {} } {line4 $line4x $line4yF 4 4 {} } {line5 $line5x $line5yF 5 5 {{dimW $c line5 0.5}} } {line6 $line6x $line6yF 5 5 {} } {line7 $line7xF $line7y 2 2 {} } {line8 $line8x $line8yF 4 4 {} } {q $TitleX $TitleY 15 3 {{random_colour $c {q1} }} } {u $TitleX+$LETTERWIDTH $TitleY 5 10 {{random_colour $c {u1}}} } {i [expr $TitleX+1.7*$LETTERWIDTH] $TitleY 5 5 {{random_colour $c {i1}}} } {ck [expr $TitleX+2.8*$LETTERWIDTH] $TitleY 3 3 {{random_colour $c {ck1}}} } {cr [expr $TitleX+4.5*$LETTERWIDTH] $TitleY 4 4 {{random_colour $c {cr1}}} } {o [expr $TitleX+5.8*$LETTERWIDTH] $TitleY 5 5 {{random_colour $c {o1}}} } {s1 [expr $TitleX+6.6*$LETTERWIDTH] $TitleY 5 5 {{random_colour $c {s1_1}}} } {s2 [expr $TitleX+7.4*$LETTERWIDTH] $TitleY 4 4 {{random_colour $c {s2_1}}} }   } {{random_colour $c {line6 line7 line8}}} $globs

    # animate just the lines
    #FlyGroupCoord $c { {line1 $line1xF $line1y 5 5 {} } {line2 $line2xF $line2y 3 3 {} } {line3 $line3xF $line3y 7 7 {} } {line4 $line4x $line4yF 4 4 {} } {line5 $line5x $line5yF 5 5 {} } }  {} $lineglobals

    # animate just the words
    
    # this one animations words, each letter has seperate random colour
   #  FlyGroupCoord $c { {q $TitleX $TitleY 15 3 {{random_colour $c {q1} }} } {u $TitleX+$LETTERWIDTH $TitleY 5 10 {{random_colour $c {u1}}} } {i [expr $TitleX+1.7*$LETTERWIDTH] $TitleY 5 5 {{random_colour $c {i1}}} } {ck [expr $TitleX+2.8*$LETTERWIDTH] $TitleY 3 3 {{random_colour $c {ck1}}} } {cr [expr $TitleX+4.5*$LETTERWIDTH] $TitleY 4 4 {{random_colour $c {cr1}}} } {o [expr $TitleX+5.8*$LETTERWIDTH] $TitleY 5 5 {{random_colour $c {o1}}} } {s1 [expr $TitleX+6.6*$LETTERWIDTH] $TitleY 5 5 {{random_colour $c {s1_1}}} } {s2 [expr $TitleX+7.4*$LETTERWIDTH] $TitleY 4 4 {{random_colour $c {s2_1}}} }   } {} {SPACEOUT LETTERWIDTH TitleX TitleY}
    # this one animates words, all letters have simultaneous random colour
    FlyGroupCoord $c { {q $TitleX $TitleY 30 25 {} } {u $TitleX+$LETTERWIDTH $TitleY 25 20 {} } {i [expr $TitleX+1.7*$LETTERWIDTH] $TitleY 30 30 {} } {ck [expr $TitleX+2.8*$LETTERWIDTH] $TitleY 30 30 {} } {cr [expr $TitleX+4.5*$LETTERWIDTH] $TitleY 28 24 {} } {o [expr $TitleX+5.8*$LETTERWIDTH] $TitleY 15 15 {} } {s1 [expr $TitleX+6.6*$LETTERWIDTH] $TitleY 25 25 {} } {s2 [expr $TitleX+7.4*$LETTERWIDTH] $TitleY 15 15 {} } } {{random_colour $c {quickcross}}} {SPACEOUT LETTERWIDTH TitleX TitleY}
	
}

proc SendMove { moveType square } {
    global NUM_SQUARES
    # #puts "$moveType $square"
    set theMove 0
    if { $moveType == "-" } {
	set theMove $square
    } elseif { $moveType == "|" } {
	set theMove [expr $square + $NUM_SQUARES] 
    } else {
	set theMove [expr $square + 2 * $NUM_SQUARES]
    }
    
    # #puts "the move is $theMove"

    ReturnFromHumanMove $theMove
}


####################################################################

  #                  RACH'S ANIMATION CODES             =)#

####################################################################

# objects in list objs MUST have a width property defined
# objs is a list of objects (or tags). percentage is the percent of the 
# current line width that the lines' width will become; assigns widths
# based on each line seperately, if more than one line
# NOTE: Caller is responsible for updating idletasks
proc shrink { c objs percentage } {

    foreach item $objs {
	set curWidth [$c itemcget $item -width]
	set newWidth [expr $curWidth * $percentage]
	$c itemconfig $item -width $newWidth
    }
    update idletasks
}

# set colour of objects in objs to given colour
# NOTE: Caller is responsible for updating idletasks
proc fade { c objs colour } {

    foreach item $objs {
	set curColour [$c itemcget $item -fill]
	$c itemconfig $item -fill $colour
    }
    update idletasks
}


# takes in a tag tagging only lines, shrinks the lines down to nothing
# Modifies: nothing; tagged lines are temporarily changed, but are restored to original state after proc finishes
# NOTE: This is used instead of shrinkAndFade because shrink does not fade using gray (fading using non-gray colours is difficult, because many other colours do not have full sets of 100 shades in Tcl/Tk; they usually have 4-5 shades, which is not accurate enough to do fade effects)
proc shrinkAnim { c objs } {
    # go through and back-up the original widths
    set orig_widths {}
    foreach item $objs {
	lappend orig_widths  [$c itemcget $item -width]
    }
    set num_to_go [llength $objs]

    while {$num_to_go > 0} {
       foreach item $objs {
	   
	   set curWidth [$c itemcget $item -width]
	   if { $curWidth >= 0.5 } {
	       shrink $c $item 0.75
	   } else {
	       set num_to_go [expr $num_to_go - 1]
	       shrink $c $item 0
	   }

	   update idletasks
       }
   }

    # restore original width, but first hide the item(s)!
    foreach item $objs {
	$c lower $item all
	
	set curWidth  [lindex $orig_widths  0]
	set orig_widths  [lrange $orig_widths  1 end]

	$c itemconfig $item -width $curWidth
	
    }
}

# takes in a tag tagging only lines, shrinks the lines down to nothing
# while fading them out.
# does NOT alter the tagged lines at all, creates new temp lines to achieve
# the desired effects
# NOTE: this has a gray colour while being shrinked.
proc shrinkAndFade { c objs frames bool_mutate} {
    # go through and back-up the original colours
    if { $bool_mutate == 0 } {
	set orig_colours {}
	set orig_widths {}
	foreach item $objs {
	    lappend orig_colours [$c itemcget $item -fill]
	    lappend orig_widths  [$c itemcget $item -width]
	}
    }

    # there are 100 shades of gray/grey in Tk
    set SHADES_GRAY 100

    set digits [list 1 2 3 4 5 6 7 8 9 0]
    set alphabet [list a b c d e f g h i j k l m n o p q r s t u v w x y z]

    set default_gray 50
    set gray_incr [expr $SHADES_GRAY / $frames]
    for {set i 0 } {$i <= $frames} {incr i} {
	foreach item $objs {
	    
	    # obtain current colour of item, if gray, use current colour, otherwise
	    # discard current colour and use default_gray_incr
	    set curColour [$c itemcget $item -fill]
	    set curColourDigTrim [string trim $curColour $digits]
	    if {[string equal $curColourDigTrim "gray"] || [string equal $curColourDigTrim "grey"]} {
		set curColourNum [string trim $curColour $alphabet]
		set nextColourNum [expr $curColourNum + $gray_incr]
	    } else {
		set default_gray [expr $default_gray + $gray_incr]
		set nextColourNum $default_gray
	    }

	    if { $nextColourNum > $SHADES_GRAY } {
		set nextColourNum $SHADES_GRAY
	    }

	    set curWidth [$c itemcget $item -width]
	    if { $curWidth > 0 } {
		shrink $c $objs 0.8
	    }

	    if { $nextColourNum < $SHADES_GRAY } {
		fade   $c $objs gray$nextColourNum
	    }

	    if { [expr floor($curWidth)] == 0 && $nextColourNum == 100 } {
		##puts "Reached CONTINUE"
		continue
	    }

	    # DEBUG
	    ##puts "curWidth: $curWidth"
	    ##puts "nextColourNum: $nextColourNum"
	    ##puts "Reached busy-wait"
	    #for {set j 0} {$j < 50000} {incr j} {
	    #}

	    ##puts "cur  colour: $curColour"
	    ##puts "next colour: $nextColourNum"
	    update idletasks
	}
	update idletasks
    }
    
    foreach item $objs {
	$c lower $item all
	
	if { $bool_mutate == 0 } {
	    set curColour [lindex $orig_colours 0]
	    set curWidth  [lindex $orig_widths  0]
	    set orig_colours [lrange $orig_colours 1 end]
	    set orig_widths  [lrange $orig_widths  1 end]
	    
	    $c itemconfig $item -fill $curColour
	    $c itemconfig $item -width $curWidth
	}
    }
}


# takes in a vector in R2 and returns
# a new vector in R2 rotated pheta radians
proc rotate { vector pheta } {

    set x [lindex $vector 0]
    set y [lindex $vector 1]

    set new_x [expr cos($pheta)*$x-sin($pheta)*$y]
    set new_y [expr sin($pheta)*$x+cos($pheta)*$y]

    return [list $new_x $new_y]
}

# takes in the endpoints of a line and returns
# the endpoints of the line rotated pheta radians
proc rotate_line { endpoints pheta } {

    set x1 [lindex $endpoints 0]
    set y1 [lindex $endpoints 1]
    set x2 [lindex $endpoints 2]
    set y2 [lindex $endpoints 3]

    # obtain coords of midpoints
    set x_mid [expr ($x1+$x2)/2]
    set y_mid [expr ($y1+$y2)/2]

    # get vector endpoints with respect to origin 0,0
    set posX [expr abs($x1-$x_mid)]
    set posY [expr abs($y1-$y_mid)]
    set negX [expr -1*$posX]
    set negY [expr -1*$posY]

    # rotate with respect to origin
    set new_pos [rotate [list $posX $posY] $pheta]
    set new_neg [rotate [list $negX $negY] $pheta]

    # get endpoints of rotated vectors
    set new_posX [lindex $new_pos 0]
    set new_posY [lindex $new_pos 1]
    set new_negX [lindex $new_neg 0]
    set new_negY [lindex $new_neg 1]

    # shift the rotated vectors back to midpoint
    set new_posX [expr $new_posX + $x_mid]
    set new_posY [expr $new_posY + $y_mid]
    set new_negX [expr $new_negX + $x_mid]
    set new_negY [expr $new_negY + $y_mid]

    return [list $new_posX $new_posY $new_negX $new_negY]
}


# animates the piece by rotating it, pheta in DEGREES
# NOTE: TIME_DELAY sets number of "line trails"
#       NOGRAY setting allows toggling of whether "line trails" are gray colour or not
proc anim_piece { c piece_tag pheta num_frames } {

#    set FRAMES 15
#    set ROT_ANG 1.570796

    #convert pheta from degress to radians
    set PI 3.141592654
    set pheta [expr $pheta * ($PI/180.0)]

    set FRAMES [ScaleDownAnimation $num_frames]
    set ROT_ANG $pheta
    set ROT_ANG_STEP [expr $ROT_ANG/$FRAMES]
    
    # other than the "leading" line, 
    # how many other lines are on-screen during anim?
    # a time delay of 0 has no "cursor trail"
    set TIME_DELAY 5
    set FADE_COLOUR_STEP 0
    if { $TIME_DELAY > 0 } {
	set FADE_COLOUR_STEP [expr round((100-50)/$TIME_DELAY)]
    }

    # set equal to 1 if do not want a gray shadow as piece rotates
    # instead, shadow is same colour
    set NOGRAY 0

    set theWidth  [$c itemcget $piece_tag -width]
    set theColour [$c itemcget $piece_tag -fill]

    for {set i 1} {$i <= $FRAMES} {incr i} {
	set ang [expr $i*$ROT_ANG_STEP]
	set curCoords [$c coords $piece_tag]
	set curCoords [rotate_line $curCoords $ang]
	
	if { $i-$TIME_DELAY-1 > 0 } {
	    $c delete animTemp[expr $i-$TIME_DELAY-1]
	}
	
	for {set j $TIME_DELAY} {$i > $TIME_DELAY && $j >=0} {set j [expr $j-1]} {
	    set d [expr $i-$j]
	    if {$NOGRAY == 0} {
		$c itemconfig animTemp[expr $i-$j] -fill gray[expr $FADE_COLOUR_STEP*$j+50]
	    }
	}

	$c create line $curCoords -width $theWidth -fill $theColour -tag animTemp$i
	update idletasks
    }

    # clean up
    for {set i [expr int($FRAMES-$TIME_DELAY)]} {$i <= $FRAMES} {incr i} {
	$c delete animTemp$i
    }

}
####################################################################
