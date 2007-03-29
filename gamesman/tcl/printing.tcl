global outputs
set outputs("left") "ps/left.ps"
set outputs("top") "ps/top.ps"
set outputs("right") "ps/right.ps"
set outputs("top_merge") "ps/top_merge.ps"
set outputs("left_merge") "ps/left_merge.ps"
set outputs("right_merge") "ps/right_merge.ps"
set outputs("output") "ps/output.ps"
set outputs("bot") "ps/bot.ps"
set outputs("bot_merge") "ps/bot_merge.ps"
set outputs("output_merge") "ps/output_merge.ps"
set outputs("outputPDF") "ps/output.pdf"
set outputs("static_blank") "ps/static/static_blank.ps"
set outputs("static_win_banner") "ps/static/static_win_banner.ps"
set outputs("gs_str") "exec /usr/bin/gs -q -dNOPAUSE -dBATCH -sDEVICE=pswrite \
	-sOutputFile="
set outputs("left_name") "ps/left_name.ps"
set outputs("right_name") "ps/right_name.ps"
set outputs("vs") "ps/vs.ps"
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
	# use gs to generate a pdf...
	# only needed for debugging
	makePDF $outputs("output") $outputs("outputPDF")
	cleanTemp
}

# setup the top part
proc makeTop { c position winningSide} {
	global outputs
	# make the name tags
	makeTags $winningSide
	set winPath [makePath $position false]
	# make top
	set topStr "$outputs(\"left_name\") \"$winPath\" $outputs(\"right_name\")"
	#set botStr "$outputs(\"static_blank\") $outputs(\"static_blank\") $outputs(\"static_blank\") "
	eval "$outputs(\"gs_str\")$outputs(\"top_merge\") $topStr"
	exec /usr/bin/psnup -q -6 -pletter $outputs("top_merge") $outputs("top")
}

# make the pages for the names
proc makeTags { winningSide } {
	global gLeftName gRightName outputs gFrameWidth gLeftPiece gRightPiece
	set maxLen [max [max [string length $gLeftName] [string length $gRightName]] 1]
	# don't want to divide by zero...
	# leave a buffer
	set maxPixels [expr [tk scaling] * 8.5 * 72 - 10]
	# make sure fontsize is an int
	# also don't make font too big
	set fontSize [min 128 [expr {int($maxPixels / $maxLen)}]]
	# pack the printing canvas
	# do some creation and capturing
	pack .printing
	.printing create text [expr $gFrameWidth / 2] 300 -justify center \
		-text "WINNER" -font {Courier 136} -tag __winner -state hidden
	.printing create text [expr $gFrameWidth / 2] 75 -justify center \
		-text $gLeftName -font "Courier $fontSize" -tag __printing
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
	.printing itemconfigure __printing -text $gRightName
	update idletasks
	# again
	.printing postscript -file $outputs("right_name") -pagewidth 8.5i
	pack forget .printing
	.printing delete __printing __winner
	update
}

# generate mistake lists for the bottom
# then make the bottom
proc makeBottom {} {
	setMistakesLists
	generateBottom
}

# combine bottom and top outputs
proc combine {} {
	global outputs
	
	eval "$outputs(\"gs_str\")$outputs(\"output_merge\") $outputs(\"top\") \
		$outputs(\"bot\")"
	exec /usr/bin/psnup  -q -2 -pletter $outputs("output_merge") $outputs("output")
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
	global kGameName outputs
	set pathStr "ps/$kGameName/$kGameName"
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
		set ret "$ret \"$pathStr\_$oldPos\_v.ps\" \"$pathStr\_$badPos.ps\""
	}
	
	return $ret
}

# find the worst mistakes defined from worst to "best"
# 1 possible lose -> win
# 2 possible lose -> tie
# 3 possible tie -> win
# 4 possible lose -> lose
# 4 possible tie -> tie
# 4 possible win -> win
# state change mistakes are bad, staying in same
# level is not as bad
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
		GS_ShowMoves $c $type $position $theMoves		
		update idletasks
			
		# capture screen shot
		capture $c $position $value $path
			
		# hide new and display old
		GS_HideMoves $c $type $position $theMoves
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
	global kGameName
	checkFolders $kGameName
	
	set path "ps/$kGameName/$kGameName\_$pos"
	if { $value == true} {
		set path "$path\_v"
	}
	
	set path "$path.ps"
	
	return $path
}

# delete temp files
# technically not needed
# since will be overwritted on next iteration
proc cleanTemp {} {
	global outputs
	
	file delete $outputs("left")
	file delete $outputs("right")
	file delete $outputs("top")
	file delete $outputs("left_merge")
	file delete $outputs("right_merge")
	file delete $outputs("bot")
	file delete $outputs("bot_merge")
	file delete $outputs("output_merge")
	file delete $outputs("top_merge")
	file delete $outputs("left_name")
	file delete $outputs("right_name")
	file delete $outputs("vs")
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
	if { ![file exists "ps/"] } {
		file mkdir "ps"
	}
	
	if { ![file exists "ps/$game"] } {
		file mkdir "ps/$game"
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