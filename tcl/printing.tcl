global p_outputs p_canvas 
set p_outputs("ps_path") "/tmp"
set p_outputs("left") "/tmp/left.ps"
set p_outputs("top") "/tmp/top.ps"
set p_outputs("right") "/tmp/right.ps"
set p_outputs("top_merge") "/tmp/top_merge.ps"
set p_outputs("left_merge") "/tmp/left_merge.ps"
set p_outputs("right_merge") "/tmp/right_merge.ps"
set p_outputs("output") "/tmp/output.ps"
set p_outputs("bot") "/tmp/bot.ps"
set p_outputs("bot_merge") "/tmp/bot_merge.ps"
set p_outputs("left_moves_merge") "/tmp/left_moves_merge.ps"
set p_outputs("left_moves") "/tmp/left_moves.ps"
set p_outputs("right_moves_merge") "/tmp/right_moves_merge.ps"
set p_outputs("right_moves") "/tmp/right_moves.ps"
set p_outputs("middle_moves_merge") "/tmp/middle_moves_merge.ps"
set p_outputs("middle_moves") "/tmp/middle_moves.ps"
set p_outputs("output_merge") "/tmp/output_merge.ps"
set p_outputs("outputPDF") "pdf/output.pdf"
set p_outputs("static_oxy") "../bitmaps/static_oxy.ps"
set p_outputs("static_legend") "../bitmaps/static_legend.ps"
set p_outputs("static_blank") "../bitmaps/static_blank.ps"
set p_outputs("gs_str") "exec /usr/bin/gs -q -dNOPAUSE -dBATCH -sDEVICE=pswrite \
	-sOutputFile="
set p_outputs("left_name") "/tmp/left_name.ps"
set p_outputs("right_name") "/tmp/right_name.ps"

# if you ever need to find the  
# to a search for the following array value
# and replace the occurences with the new path
# need to find a way to pass it to exec w/o the
# substitutions
set p_font "(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf)"

set p_canvas .printing 
canvas $p_canvas -width 500 -height 500 

# do the printing
proc doPrinting {c position winningSide} {
	global p_outputs gFrameWidth p_canvas

  # we need to resize to correct ratio for the current screen
  $p_canvas configure -width $gFrameWidth -height $gFrameWidth

	# capture the last position
	capture $c $position false [makePath $position false] 

  # create the PDF display widget
	$c create rectangle 0 [expr $gFrameWidth/2 - 75] $gFrameWidth [expr $gFrameWidth/2 + 75] -fill gray -width 1 -outline black -tag PDF
	$c create text [expr $gFrameWidth/2] [expr $gFrameWidth/2 - 40] -text "Generating PDF" -font {Courier 40} -tags PDF
	$c create text [expr $gFrameWidth/2] [expr $gFrameWidth/2 + 40] -text "Step 1 of 6" -font {Courier 40} -tags [list PDF PDFStep]
	update idletasks
	makeTop $c $position $winningSide
	
	$c itemconfigure PDFStep -text "Step 2 of 6"
	update idletasks
	makeBottom
	
	$c itemconfigure PDFStep -text "Step 3 of 6"
	update idletasks
	combine
	
	$c itemconfigure PDFStep -text "Step 4 of 6"
	update idletasks
	addHeader
	addFooter
	
	$c itemconfigure PDFStep -text "Step 5 of 6"
	update idletasks
	exec /usr/bin/psnup -q -1 -b0.25in -pletter -Pletter $p_outputs("output") $p_outputs("output_merge")
	
	$c itemconfigure PDFStep -text "Step 6 of 6"
	update idletasks
	# use gs to generate a pdf...
	# only needed for debugging
	makePDF $p_outputs("output_merge") $p_outputs("outputPDF")
	$c delete PDF
}

proc addFooter { } {
	global p_outputs 
	# omg... at this command
	# add titles for the bottom columns
	# also add the gamescrafters website location
	# the 2 or 3 in the third part is a seega hack...
	# need to find a better way to detect where to input
	exec /usr/bin/sed -i -r -e '/gsave mark/N' \
		-e '/gsave mark\[\[:space:]]+Q q/N' \
		-e 's/gsave mark\[\[:space:]]+Q q\[\[:space:]]+57\[23]/\
		%Added postscript\\n \
		\\/dispCenter \{dup stringwidth pop 2 div neg 0 rmoveto show\} bind def\\n \
		600 150 moveto\\n \
		-100 0 rlineto\\n \
		0 7620 rlineto\\n \
		100 0 rlineto\\n \
		2600 150 moveto\\n \
		3050 0 rlineto\\n \
		0 7620 rlineto\\n \
		-2550 0 rlineto\\n \
		500 3960 moveto\\n \
		100 0 rlineto\\n \
		5650 3960 moveto\\n \
		-1050 0 rlineto\\n \
		stroke\\n \
		139.0000 0.0000 0.0000 setrgbcolor\\n \
		4580 3710 moveto\\n \
		0 500 rlineto\\n \
		300 0 rlineto\\n \
		0 -500 rlineto\\n \
		fill\\n \
		255.0000 255.0000 0.0000 setrgbcolor\\n \
		4930 3710 moveto\\n \
		0 500 rlineto\\n \
		300 0 rlineto\\n \
		0 -500 rlineto\\n \
		fill\\n \
		0.0000 255.0000 0.0000 setrgbcolor\\n \
		5280 3710 moveto\\n \
		0 500 rlineto\\n \
		300 0 rlineto\\n \
		0 -500 rlineto\\n \
		fill\\n \
		0.0000 0.0000 0.0000 setrgbcolor\\n \
		90 rotate\\n \
		(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf) findfont\\n \
		200 scalefont\\n \
		setfont\\n \
		3960 -4800 moveto\\n \
		(Lose) dispCenter\\n \
		3960 -5150 moveto\\n \
		(Tie) dispCenter\\n \
		3960 -5500 moveto\\n \
		(Win) dispCenter\\n \
		700 -400 moveto\\n \
		(Before) show\\n \
		2750 -400 moveto\\n \
		(After) show\\n \
		4650 -400 moveto\\n \
		(Before) show\\n \
		6700 -400 moveto\\n \
		(After) show\\n \
		1980 -5850 moveto\\n \
		(GamesCrafters) dispCenter\\n \
		5940 -5850 moveto\\n \
		(http:\\/\\/gamescrafters.berkeley.edu\\/) dispCenter\\n \
		(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf) findfont\\n \
		500 scalefont\\n \
		setfont\\n \
		3960 -1000 moveto\\n \
		(M) dispCenter\\n \
		3960 -1500 moveto\\n \
		(I) dispCenter\\n \
		3960 -2000 moveto\\n \
		(S) dispCenter\\n \
		3960 -2500 moveto\\n \
		(T) dispCenter\\n \
		3960 -3000 moveto\\n \
		(A) dispCenter\\n \
		3960 -3500 moveto\\n \
		(K) dispCenter\\n \
		3960 -4000 moveto\\n \
		(E) dispCenter\\n \
		3960 -4500 moveto\\n \
		(S) dispCenter\\n \
		150 -1000 moveto\\n \
		(L) dispCenter\\n \
		150 -1500 moveto\\n \
		(E) dispCenter\\n \
		150 -2000 moveto\\n \
		(F) dispCenter\\n \
		150 -2500 moveto\\n \
		(T) dispCenter\\n \
		7750 -1000 moveto\\n \
		(R) dispCenter\\n \
		7750 -1500 moveto\\n \
		(I) dispCenter\\n \
		7750 -2000 moveto\\n \
		(G) dispCenter\\n \
		7750 -2500 moveto\\n \
		(H) dispCenter\\n \
		7750 -3000 moveto\\n \
		(T) dispCenter\\n \
		-90 rotate\\n&/' $p_outputs("output")	
}

proc addHeader { } {
	global p_outputs kGameName 
	# add the gamescrafters logo
	# center on 396 -75
	# add the time and the date
	# we need to escape the / in date
	# otherwise sed complains
	# get the hostname too
	# and escape incase it has /
	set name $kGameName
	set t [clock format [clock seconds] -format %T]
	set date [clock format [clock seconds] -format "%Y-%m-%d"]
	set host [exec /usr/bin/hostname]
	regsub -all {\/} $host "\\\/" host
	# remove mobile if it starts with mobile
	regsub -all {^mobile} $host "" host
	# replace the spaces in name... with -
	# otherwise gs dies
	# replace the ' also
	regsub -all { } $name "-" name
	regsub -all {\'} $name "`" name
	# this gets duplicated for some reason...
	# need to fix some time
	exec /usr/bin/sed -i -r -e 's/cleartomark end end pagesave restore showpage/&\\n \
		%Added postscript\\n \
		90 rotate\\n \
		\\/dispCenter \{dup stringwidth pop 2 div neg 0 rmoveto show\} bind def\\n \
		(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf) findfont\\n \
		40 scalefont\\n \
		setfont\\n \
		newpath\\n \
		396 -35 moveto\\n \
		(Post-Game Analysis) dispCenter\\n \
		396 -75 moveto\\n \
		($name) dispCenter\\n \
		265 -350 moveto\\n \
		(M) dispCenter\\n \
		265 -390 moveto\\n \
		(O) dispCenter\\n \
		265 -430 moveto\\n \
		(V) dispCenter\\n \
		265 -470 moveto\\n \
		(E) dispCenter\\n \
		530 -350 moveto\\n \
		(H) dispCenter\\n \
		530 -390 moveto\\n \
		(I) dispCenter\\n \
		530 -430 moveto\\n \
		(S) dispCenter\\n \
		530 -470 moveto\\n \
		(T) dispCenter\\n \
		530 -510 moveto\\n \
		(O) dispCenter\\n \
		530 -550 moveto\\n \
		(R) dispCenter\\n \
		530 -590 moveto\\n \
		(Y) dispCenter\\n \
		(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf) findfont\\n \
		20 scalefont\\n \
		setfont\\n \
		newpath\\n \
		60 -45 moveto\\n \
		($date) dispCenter\\n \
		60 -75 moveto\\n \
		($t) dispCenter\\n \
		750 -60 moveto\\n \
		($host) dispCenter\\n \
		-90 rotate/' $p_outputs("output")
}

# setup the top part
proc makeTop { c position winningSide} {
	global p_outputs kGameName
	# make the name tags
	makeTags $winningSide
	set maxMoves 6
	# disable caching
	set winPath [makePath $position false]
	set moves [makeMoveList [expr 3 * $maxMoves]]
	set emptyStr "$p_outputs(\"static_blank\") $p_outputs(\"static_blank\")"
	set leftMoves ""
	set middleMoves ""
	set rightMoves ""
	# we need to partition the moves to different parts now
	set i 0
	set j 0
	foreach move $moves {
		if {$i == 0 } {
			set leftMoves "$leftMoves \"$move\""
		} elseif {$i == 1} {
			set middleMoves "$middleMoves \"$move\""
		} else {
			set rightMoves "$rightMoves \"$move\""
		}
		# we want 2 across on each part
		incr j
		if {$j == 2} {
			incr i
			# reset i if we need to
			if {$i == 3} {
				set i 0
			}
			set j 0
		}
	}
	if {$leftMoves == "" } {
		set leftMoves $emptyStr
	}
	eval "$p_outputs(\"gs_str\")$p_outputs(\"left_moves_merge\") $leftMoves"
	exec /usr/bin/psnup -q -m0.65in -$maxMoves -H8.5in -W8.6in -pletter $p_outputs("left_moves_merge") $p_outputs("left_moves")
	
	if {$middleMoves == "" } {
		set middleMoves $emptyStr
	}
	eval "$p_outputs(\"gs_str\")$p_outputs(\"middle_moves_merge\") $middleMoves"
	exec /usr/bin/psnup -q -m0.65in -$maxMoves -H8.5in -W8.6in -pletter $p_outputs("middle_moves_merge") $p_outputs("middle_moves")	

	if {$rightMoves == "" } {
		set rightMoves $emptyStr
	}
	eval "$p_outputs(\"gs_str\")$p_outputs(\"right_moves_merge\") $rightMoves"
	exec /usr/bin/psnup -q -m0.65in -$maxMoves -H8.5in -W8.6in -pletter $p_outputs("right_moves_merge") $p_outputs("right_moves")		
		

	# make top
	set topStr "$p_outputs(\"left_name\") \"$winPath\" $p_outputs(\"right_name\") "
	set topStr "$topStr $p_outputs(\"left_moves\") $p_outputs(\"middle_moves\") $p_outputs(\"right_moves\")"
	eval "$p_outputs(\"gs_str\")$p_outputs(\"top_merge\") $topStr"
	exec /usr/bin/psnup -q -6 -Pletter -pletter $p_outputs("top_merge") $p_outputs("top") 
}

# make the pages for the names
proc makeTags { winningSide } {
	global gLeftName gRightName gFrameWidth gLeftPiece gRightPiece 
  global p_outputs p_canvas 

	set colors [GS_ColorOfPlayers]
	set maxLen [max [max [string length $gLeftName] \
	 					[string length $gRightName]] 1]
	# don't want to divide by zero...
	# leave a buffer
	set yOffset [expr $gFrameWidth / 2] 
	set maxPixels [expr [tk scaling] * 8.5 * 72 - 10]

	# make sure fontsize is an int
	# also don't make font too big
	set fontSize [min 128 [expr {int($maxPixels / $maxLen)}]]

	# pack the printing canvas
	# do some creation and capturing
	# courier is just used as a placeholder... we will replace it
	# with the desired font
	pack $p_canvas

	$p_canvas create text [expr $gFrameWidth / 2] [expr 1.9 * $yOffset]  \
		-justify center -text "WINNER" -font {Courier 128} \
		-tag __winner -state hidden

	$p_canvas create text [expr $gFrameWidth / 2] $yOffset -justify center \
		-text "LEFT\n$gLeftName" -font "Courier $fontSize" -tag __printing -fill [lindex $colors 0]

	if { $winningSide == $gLeftPiece } {
		$p_canvas itemconfigure __winner -state normal
    $p_canvas create rectangle 0 [expr $yOffset / 2] $gFrameWidth $gFrameWidth \
        -outline black -width 5 -tags p_rectangle 
	}
	update idletasks
	
	# reconfigure the text and capture again
	$p_canvas postscript -file $p_outputs("left_name") -pagewidth 8.5i
	if { $winningSide == $gRightPiece } {
		$p_canvas itemconfigure __winner -state normal
    $p_canvas create rectangle 0 [expr $yOffset / 2] $gFrameWidth $gFrameWidth \
        -outline black -width 5 -tags p_rectangle 
	} else {
		$p_canvas itemconfigure __winner -state hidden
    $p_canvas delete p_rectangle
	}

	$p_canvas itemconfigure __printing -text "RIGHT\n$gRightName" -fill [lindex $colors 1]
	update idletasks

	# again
	$p_canvas postscript -file $p_outputs("right_name") -pagewidth 8.5i
	pack forget $p_canvas
	$p_canvas delete __printing __winner
	update

	# replace the Courier font with
	# the vag rounded bt
	exec /usr/bin/sed -i -r -e 's/\\/Courier/(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf)/' $p_outputs("left_name")
	exec /usr/bin/sed -i -r -e 's/\\/Courier/(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf)/' $p_outputs("right_name")
}

proc makeMoveList { maxMoves } {
	global gGameSoFar
	set moves []
	set moveList ""
	# need to reverse the list...
	# tcl 8.5 has a built in reverse
	# we start at len - 2 because len is > last index
	# and the last index is pos 0 which we don't want
	for {set i [expr [llength $gGameSoFar] - 2]} { $i >= 0} {incr i -1} {
		lappend moves [lindex $gGameSoFar $i]
	}
	
	# truncate the list to how many we need
	# end index > len - 1 => len - 1
	set moves [lrange $moves 0 [expr $maxMoves - 1]]
	
	foreach move $moves {
		set path [makePath $move false]
		set moveList "$moveList \"$path\""
	}
	
	return $moveList
}

# generate mistake lists for the bottom
# then make the bottom
proc makeBottom {} {
	setMistakesLists
	generateBottom
}

# combine bottom and top outputs
proc combine {} {
	global p_outputs
	eval "$p_outputs(\"gs_str\")$p_outputs(\"output_merge\") $p_outputs(\"top\") \
		$p_outputs(\"bot\")"
	exec /usr/bin/psnup -q -2 -pletter -W8.25in $p_outputs("output_merge") $p_outputs("output")	
}

# go through all the mistakes
# and separate into left and right mistakes
proc setMistakesLists { } {
	global gMistakeList leftMistakes rightMistakes
	set leftMistakes []
	set rightMistakes []
	foreach mistake $gMistakeList {
		if {[lindex $mistake 0] == "Left" } {
			lappend leftMistakes [lrange $mistake 1 [llength $mistake]]
		} elseif {[lindex $mistake 0] == "Right" } {
			lappend rightMistakes [lrange $mistake 1 [llength $mistake]]
		}
	}
}

# generate the mistake pages to display
proc generateBottom { } {
	global leftMistakes rightMistakes p_outputs

	set maxErrors 4
	set leftExec [makeExec $leftMistakes $maxErrors]
	set rightExec [makeExec $rightMistakes $maxErrors]
	set mergeStr ""

	# if we added something then merge and psnup
	# then combine the pages, else we point to a blank page
	# need to point to blank page to preserve ordering
	if { $leftExec == "" } {
		set leftExec "$p_outputs(\"static_blank\") $p_outputs(\"static_blank\")"
	}
	eval "$p_outputs(\"gs_str\")$p_outputs(\"left\") $leftExec"
	exec /usr/bin/psnup -q -[expr 2 * $maxErrors] -H8.5in -W8.5in -pletter \
		$p_outputs("left") $p_outputs("left_merge")
	set mergeStr "$mergeStr $p_outputs(\"left_merge\")"
	
	if { $rightExec == "" } {
		set rightExec "$p_outputs(\"static_blank\") $p_outputs(\"static_blank\")"
	}
	eval "$p_outputs(\"gs_str\")$p_outputs(\"right\") $rightExec"
	exec /usr/bin/psnup -q -[expr 2 * $maxErrors] -H8.5in -W8.5in -pletter \
			$p_outputs("right") $p_outputs("right_merge")
	set mergeStr "$mergeStr $p_outputs(\"right_merge\")"
	
	# combine the left and right outputs to form the bottom
	eval "$p_outputs(\"gs_str\")$p_outputs(\"bot_merge\") $mergeStr"
	exec /usr/bin/psnup -q -2 -pletter $p_outputs("bot_merge") $p_outputs("bot")
}

# go through the mistake lists
# and add stuff to the psnup execute command
proc makeExec { mistakeList maxErrors} {
	global p_outputs
	set size [llength $mistakeList]
	set ret ""
	# trim mistakes to the worst ones
	if { $size > $maxErrors } {
		# get list of the worst mistakes
		set mistakeList [findWorst $mistakeList]
		# truncate to maxErrors
		set mistakeList [lrange $mistakeList 0 [expr $maxErrors - 1]]
	}

	# iterate over the mistake lists
	# taking the old positions value moves
	# and the actual in game result
	foreach mistake $mistakeList {
		set oldPos [lindex $mistake 6]
		set badMove [lindex $mistake 1]
		set badPos [C_DoMove $oldPos $badMove]
		set old [makePath $oldPos true]
		set new [makePath $badPos false]
    # position was not captured
    # use blank place holder for now
    if { ![file exists $new]} {
      set new $p_outputs("static_blank") 
    }
		set ret "$ret \"$old\" \"$new\""
	}
	
	return $ret
}

# find the worst mistakes defined from worst to "best"
# 1 possible lose -> win
# 2 possible lose -> tie
# 3 possible tie -> win
# state change mistakes are bad
proc findWorst { mistakeList } {
	set worst [list]
	# sort by increasing significance of key
	# what did we end up at?
	# giving them a win is the worst
	foreach mistake $mistakeList {
		set type [lindex $mistake 0]
		if { $type  == "Lose"} {
			set val 3
		} elseif { $type == "Tie" } {
			set val 2
		} elseif { $type == "Win" } {
			set val 1
		}
		lappend worst [list $val $mistake]
	}
	
	set worst [stripKey [lsort -integer -index 0 $worst]]

	set mistakeList [list]
	# what did we give up?
	# giving up a losing position is the worst
	foreach mistake $worst {
		set type [lindex $mistake 3]
		if { $type  == "Lose"} {
			set val 1
		} elseif { $type == "Tie" } {
			set val 2
		} elseif { $type == "Win" } {
			set val 3
		}
		lappend mistakeList [list $val $mistake]
	}
	
	set worst [stripKey [lsort -integer -index 0 $mistakeList]]
	
	return $worst
}

# strips the key used to sort lst
# returns lst without the key
proc stripKey { lst } {
	set ret [list]
	for {set i 0} {$i < [llength $lst] } {incr i } {
		lappend ret [lindex [lindex $lst $i] 1]
	}
	
	return $ret
}

# do the setup required for a screenshot
# only change Move type if we need to ie
# the position we need to capture hasn't
# been capture yet
proc doCapture { c moveType position theMoves value} {
	set path [makePath $position $value]
	if { $path != "" } {
		set type "all"
		if {$value == true} {
			set type "value"
		}

		# hide old and display new
		GS_HideMoves $c $moveType $position $theMoves

		if {$value == true} {
			GS_ShowMoves $c $type $position $theMoves		
		}
		update idletasks
			
		# capture screen shot
		capture $c $position $value $path
			
		# hide new and display old
		if {$value == true} {
			GS_HideMoves $c $type $position $theMoves
		}

		GS_ShowMoves $c $moveType $position $theMoves
		update idletasks
	}
}

# capture a screen shot
# give it the correct name depending on options
proc capture { c pos value path} {
	# capture to game directory
	if { $path != "" } {
		# hide background to reduce drawing time
		$c itemconfigure background -state hidden

		$c postscript -file "$path" -pagewidth 8.0i -pagey 0.0i -pageanchor s

		# show it again
		$c itemconfigure background -state normal
	}
}

# make path with given arguments
# do the folder checking in here, since other
# functions should call this function to make the
# path
proc makePath { pos valueMove } {
	global kGameName p_outputs
	checkFolders $kGameName
	
	set path "$p_outputs(\"ps_path\")/$kGameName/$kGameName\_$pos"

	if { $valueMove == true} {
		set path "$path\_v"
	}
	
	return "$path.ps"
}

# make string to use for pdf generation
# then call it
proc makePDF { input output} {
	global p_outputs
	if { ![file exists "pdf/"] } {
		file mkdir "pdf/"
	}
	eval "exec /usr/bin/gs -q -dNOPAUSE -dBATCH -sOutputFile=$output \
			-sDEVICE=pdfwrite -c .setpdfwrite -f $input"
}

# check for folders we need
proc checkFolders { game } {
	# check for directories
	global p_outputs
	if {![file exists "$p_outputs(\"ps_path\")/"] } {
		file mkdir "$p_outputs(\"ps_path\")"
	}
	
	if {![file exists "$p_outputs(\"ps_path\")/$game"] } {
		file mkdir "$p_outputs(\"ps_path\")/$game"
	}
}

proc max { a b } {
	if { $a > $b } {
		return $a
	}
	return $b
}

proc min { a b } {
	if { $a < $b } {
		return $a
	}
	return $b
}
