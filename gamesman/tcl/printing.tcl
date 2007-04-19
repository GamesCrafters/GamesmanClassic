global outputs
set outputs("ps_path") "ps"
set outputs("left") "/tmp/left.ps"
set outputs("top") "/tmp/top.ps"
set outputs("right") "/tmp/right.ps"
set outputs("top_merge") "/tmp/top_merge.ps"
set outputs("left_merge") "/tmp/left_merge.ps"
set outputs("right_merge") "/tmp/right_merge.ps"
set outputs("output") "ps/output.ps"
set outputs("bot") "/tmp/bot.ps"
set outputs("bot_merge") "/tmp/bot_merge.ps"
set outputs("left_moves_merge") "/tmp/left_moves_merge.ps"
set outputs("left_moves") "/tmp/left_moves.ps"
set outputs("right_moves_merge") "/tmp/right_moves_merge.ps"
set outputs("right_moves") "/tmp/right_moves.ps"
set outputs("output_merge") "/tmp/output_merge.ps"
set outputs("outputPDF") "ps/output.pdf"
set outputs("static_oxy") "../bitmaps/static_oxy.ps"
set outputs("static_legend") "../bitmaps/static_legend.ps"
set outputs("static_blank") "../bitmaps/static_blank.ps"
set outputs("gs_str") "exec /usr/bin/gs -q -dNOPAUSE -dBATCH -sDEVICE=pswrite \
	-sOutputFile="
set outputs("left_name") "/tmp/left_name.ps"
set outputs("right_name") "/tmp/right_name.ps"
# if you ever need to find the font_str
# to a search for the following array value
# and replace the occurences with the new path
# need to find a way to pass it to exec w/o the
# substitutions
set outputs("font_str") "(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf)"
canvas .printing -width 500 -height 500

# do the printing
proc doPrinting {c position winningSide} {
	global outputs
	set path [makePathOnce $position false]
	# capture the last position
	capture $c $position false $path
	makeTop $c $position $winningSide
	makeBottom
	combine
	# omg... at this command
	# add titles for the bottom columns
	# also add the gamescrafters website location
	exec /usr/bin/sed -i -r -e '/gsave mark/N' \
		-e '/gsave mark\[\[:space:]]+Q q/N' \
		-e 's/gsave mark\[\[:space:]]+Q q\[\[:space:]]+497\.645/\
		%Added postscript\\n \
		90 rotate\\n \
		(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf) findfont\\n \
		200 scalefont\\n \
		setfont\\n \
		newpath\\n \
		700 -400 moveto\\n \
		(Before) show\\n \
		2750 -400 moveto\\n \
		(After) show\\n \
		4650 -400 moveto\\n \
		(Before) show\\n \
		6700 -400 moveto\\n \
		(After) show\\n \
		1980 -5800 moveto\\n \
		(GamesCrafters) dup stringwidth pop 2 div neg 0 rmoveto show\\n \
		5940 -5800 moveto\\n \
		(http:\\/\\/gamescrafters.berkeley.edu\\/) dup stringwidth pop 2 div neg 0 rmoveto show\\n \
		-90 rotate\\n&/' $outputs("output")
	# use gs to generate a pdf...
	# only needed for debugging
	makePDF $outputs("output") $outputs("outputPDF")
}

# replace references to /Courier with
# the vag rounded bt font...
proc replaceFont { } {
	global outputs
	# right name
	exec /usr/bin/sed -i -r -e 's/\\/Courier/(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf)/' $outputs("right_name")
	# left name
	exec /usr/bin/sed -i -r -e 's/\\/Courier/(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf)/' $outputs("left_name") 
}

# setup the top part
proc makeTop { c position winningSide} {
	global outputs kGameName
	# make the name tags
	makeTags $winningSide
	set maxMoves 6
	set winPath [makePath $position false]
	set moves [makeMoveList $maxMoves]
	eval "$outputs(\"gs_str\")$outputs(\"left_moves_merge\") $moves"
	exec /usr/bin/psnup -q -m0.6in -$maxMoves -H8.5in -W8.5in -pletter $outputs("left_moves_merge") $outputs("left_moves")
	# make top
	set topStr "$outputs(\"left_name\") \"$winPath\" $outputs(\"right_name\") "
	set topStr "$topStr $outputs(\"left_moves\") $outputs(\"static_legend\") $outputs(\"static_blank\")"
	eval "$outputs(\"gs_str\")$outputs(\"top_merge\") $topStr"
	exec /usr/bin/psnup -q -6 -Pletter -pletter $outputs("top_merge") $outputs("top") 
}

# make the pages for the names
proc makeTags { winningSide } {
	global gLeftName gRightName outputs gFrameWidth gLeftPiece gRightPiece
	set colors [GS_ColorOfPlayers]
	set maxLen [max [max [string length $gLeftName] \
	 					[string length $gRightName]] 1]
	# don't want to divide by zero...
	# leave a buffer
	set yOffset 250
	set maxPixels [expr [tk scaling] * 8.5 * 72 - 10]
	# make sure fontsize is an int
	# also don't make font too big
	set fontSize [min 128 [expr {int($maxPixels / $maxLen)}]]
	# pack the printing canvas
	# do some creation and capturing
	# courier is just used as a placeholder... we will replace it
	# with the desired font
	pack .printing
	.printing create text [expr $gFrameWidth / 2] [expr $yOffset + 210] \
		-justify center -text "WINNER" -font {Courier 128} \
		-tag __winner -state hidden
	.printing create text [expr $gFrameWidth / 2] $yOffset -justify center \
		-text "LEFT\n$gLeftName" -font "Courier $fontSize" -tag __printing -fill [lindex $colors 0]
	if { $winningSide == $gLeftPiece } {
		.printing itemconfigure __winner -state normal
	}
	update idletasks
	
	# reconfigure the text and capture again
	.printing postscript -file $outputs("left_name") -pagewidth 8.5i
	if { $winningSide == $gRightPiece } {
		.printing itemconfigure __winner -state normal
	} else {
		.printing itemconfigure __winner -state hidden
	}
	.printing itemconfigure __printing -text "RIGHT\n$gRightName" -fill [lindex $colors 1]
	update idletasks
	# again
	.printing postscript -file $outputs("right_name") -pagewidth 8.5i
	pack forget .printing
	.printing delete __printing __winner
	update
	# replace the Courier font with
	# the vag rounded bt
	replaceFont
}

proc makeMoveList { maxMoves } {
	global gGameSoFar
	set moves []
	set moveList ""
	set len [llength $gGameSoFar]
	if { $len <= $maxMoves } {
		set index 0
	} else {
		set index [expr $len - $maxMoves - 1]
	}
	# don't get position 0
	set newList [lrange $gGameSoFar $index [expr $len - 2]]
	for {set i [expr [llength $newList] - 1]} {$i >= 0} {incr i -1} {
		lappend moves [lindex $newList $i]
	}
	foreach move $moves {
		set path [makePath $move false]
		set moveList "$moveList $path"
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
	global outputs kGameName
	eval "$outputs(\"gs_str\")$outputs(\"output_merge\") $outputs(\"top\") \
		$outputs(\"bot\")"
	exec /usr/bin/psnup -q -2 -pletter -W8.25in $outputs("output_merge") $outputs("output")
	# add the gamescrafters logo
	# center on 396 -75
	# add the time and the date
	# we need to escape the / in date
	# otherwise sed complains
	# get the hostname too
	# and escape incase it has /
	set t [clock format [clock seconds] -format %T]
	set date [clock format [clock seconds] -format "%Y-%m-%d"]
	set host [exec /usr/bin/hostname]
	regsub -all {\/} $host "\\\/" host
	exec /usr/bin/sed -i -r -e '/f/N' \
		-e 's/f\[\[:space:]]+cleartomark end end pagesave restore showpage/&\\n \
		%Added postscript\\n \
		90 rotate\\n \
		(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf) findfont\\n \
		40 scalefont\\n \
		setfont\\n \
		newpath\\n \
		396 -35 moveto\\n \
		(Post Game Analysis) dup stringwidth pop 2 div neg 0 rmoveto show\\n \
		396 -75 moveto\\n \
		($kGameName) dup stringwidth pop 2 div neg 0 rmoveto show\\n \
		(\\/usr\\/share\\/ghostscript\\/fonts\\/Vag Rounded BT.ttf) findfont\\n \
		20 scalefont\\n \
		setfont\\n \
		newpath\\n \
		50 -45 moveto\\n \
		($date) dup stringwidth pop 2 div neg 0 rmoveto show\\n \
		50 -75 moveto\\n \
		($t) dup stringwidth pop 2 div neg 0 rmoveto show\\n \
		750 -60 moveto\\n \
		($host) dup stringwidth pop 2 div neg 0 rmoveto show\\n \
		-90 rotate/' $outputs("output")
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
	global leftMistakes rightMistakes outputs

	set maxErrors 4
	set leftExec [makeExec $leftMistakes $maxErrors]
	set rightExec [makeExec $rightMistakes $maxErrors]
	set mergeStr ""

	
	# if we added something then merge and psnup
	# then combine the pages, else we point to a blank page
	# need to point to blank page to preserve ordering
	if { $leftExec != "" } {
		eval "$outputs(\"gs_str\")$outputs(\"left\") $leftExec"
		exec /usr/bin/psnup -q -[expr 2 * $maxErrors] -H8.5in -W8.5in -pletter \
			$outputs("left") $outputs("left_merge")
		set mergeStr "$mergeStr $outputs(\"left_merge\")"
	} else {
		set mergeStr "$mergeStr $outputs(\"static_blank\")"
	}
	
	if { $rightExec != "" } {
		eval "$outputs(\"gs_str\")$outputs(\"right\") $rightExec"
		exec /usr/bin/psnup -q -[expr 2 * $maxErrors] -H8.5in -W8.5in -pletter \
			$outputs("right") $outputs("right_merge")
		set mergeStr "$mergeStr $outputs(\"right_merge\")"
	} else {
		set mergeStr "$mergeStr $outputs(\"static_blank\")"
	}
	
	# combine the left and right outputs to form the bottom
	eval "$outputs(\"gs_str\")$outputs(\"bot_merge\") $mergeStr"
	exec /usr/bin/psnup -q -2 -d -pletter $outputs("bot_merge") $outputs("bot")
}

# go through the mistake lists
# and add stuff to the psnup execute command
proc makeExec { mistakeList maxErrors} {
	global outputs
	set size [llength $mistakeList]
	set ret ""
	# trim mistakes to the worst ones
	if { $size > $maxErrors } {
		# get list of the worst mistakes
		set mistakeList [findWorst $mistakeList]
		# truncate to 4 mistakes
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
		set ret "$ret $old $new"
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
	set path [makePathOnce $position $value]
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

# will generate the path for the given arguments
# but will only return if the path doesn't already
# exist, if it does, then return ""
proc makePathOnce { pos value} {
	set path [makePath $pos $value]
	
	if { [file exists $path] } {
		return ""
	}
	
	return $path
}

# make path with given arguments
# do the folder checking in here, since other
# functions should call this function to make the
# path
proc makePath { pos value } {
	global kGameName outputs
	checkFolders $kGameName
	
	set path "$outputs(\"ps_path\")/$kGameName/$kGameName\_$pos"
	if { $value == true} {
		set path "$path\_v"
	}
	
	return "$path.ps"
}

# make string to use for pdf generation
# then call it
proc makePDF { input output} {
	eval "exec /usr/bin/gs -q -dNOPAUSE -dBATCH -sOutputFile=$output \
			-sDEVICE=pdfwrite -c .setpdfwrite -f $input"
}

# check for folders we need
proc checkFolders { game } {
	# check for directories
	global outputs
	if { ![file exists "$outputs(\"ps_path\")/"] } {
		file mkdir "$outputs(\"ps_path\")"
	}
	
	if { ![file exists "$outputs(\"ps_path\")/$game"] } {
		file mkdir "$outputs(\"ps_path\")/$game"
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