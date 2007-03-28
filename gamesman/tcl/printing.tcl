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
set outputs("gsStr") "exec /usr/bin/gs -q -dNOPAUSE -dBATCH -sDEVICE=pswrite -sOutputFile="
set outputs("left_name") "ps/left_name.ps"
set outputs("right_name") "ps/right_name.ps"
set outputs("vs") "ps/vs.ps"
canvas .printing -width 1000 -height 1000

# do the printing
proc doPrinting {c position winner} {
	global outputs kGameName
	set path [makePathOnce $position false true]
	# capture the last position
	capture $c $position false true $path
	makeTags
	makeTop $c $position $winner
	makeBottom
	combine
	# use gs to generate a pdf...
	# only needed for debugging
	makePDF $outputs("output") $outputs("outputPDF")
	cleanTemp
}

proc makeTags { } {
	global gLeftName gRightName outputs gFrameWidth

	# pack the printing canvas
	# do some creation and capturing
	pack .printing
	.printing create text [expr $gFrameWidth / 2] 75 -justify center -text $gLeftName -font {Helvetica 128} -tag __printing
	update idletasks
	
	# reconfigure the text and capture again
	.printing postscript -file $outputs("left_name") -pagewidth 8.0i -rotate true
	.printing itemconfigure __printing -text $gRightName
	update idletasks
	
	# again
	.printing postscript -file $outputs("right_name") -pagewidth 8.0i -rotate true
	pack forget .printing
	.printing delete __printing
	update
}

# setup the top part
proc makeTop { c position nameOfWinner} {
	global gFrameWidth outputs kGameName
	# combine column major ones...
	# then combine row major
	# should have rotated way we want...
	set winPath [makePath $position false true]
	# combine vs and winning pos
	eval "$outputs(\"gsStr\")$outputs(\"top_merge\") \"$winPath\""
	exec /usr/bin/psnup -q -2 -pletter $outputs("top_merge") $outputs("top")
	
	# combine left name and possible history?
	eval "$outputs(\"gsStr\")$outputs(\"top_merge\") $outputs(\"left_name\")"
	exec /usr/bin/psnup -q -2 -pletter $outputs("top_merge") $outputs("left_name")
	
	# combine right side
	eval "$outputs(\"gsStr\")$outputs(\"top_merge\") $outputs(\"right_name\")"
	exec /usr/bin/psnup -q -2 -pletter $outputs("top_merge") $outputs("right_name")
	
	# make top
	set topStr "$outputs(\"right_name\") $outputs(\"top\") $outputs(\"left_name\")"
	#set botStr "$outputs(\"static_blank\") $outputs(\"static_blank\") $outputs(\"static_blank\")"
	eval "$outputs(\"gsStr\")$outputs(\"top_merge\") $topStr"
	exec /usr/bin/psnup -q -3 -pletter $outputs("top_merge") $outputs("top")
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
	
	eval "$outputs(\"gsStr\")$outputs(\"output_merge\") $outputs(\"bot\") $outputs(\"top\")"
	exec /usr/bin/psnup -q -2 -pletter $outputs("output_merge") $outputs("output")
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

proc generateBottom { } {
	global leftMistakes rightMistakes outputs

	puts $leftMistakes
	puts $rightMistakes
	set maxErrors 4
	set leftExec [makeExec $leftMistakes $maxErrors]
	set rightExec [makeExec $rightMistakes $maxErrors]
	set mergeStr ""

	# if we added something then merge and psnup
	# then combine the pages, else we point to a blank page
	# do not erase spaces in mergeStr
	# need to point to blank page to preserve ordering
	if { $rightExec != "" } {
		eval "$outputs(\"gsStr\")$outputs(\"right\") $rightExec"
		exec /usr/bin/psnup -c -q -[expr 2 * $maxErrors] -pletter $outputs("right") $outputs("right_merge")
		set mergeStr "$mergeStr $outputs(\"right_merge\")"
	} else {
		set mergeStr "$mergeStr $outputs(\"static_blank\")"
	}
	
	if { $leftExec != "" } {
		eval "$outputs(\"gsStr\")$outputs(\"left\") $leftExec"
		exec /usr/bin/psnup -c -q -[expr 2 * $maxErrors] -pletter $outputs("left") $outputs("left_merge")
		set mergeStr "$mergeStr $outputs(\"left_merge\")"
	} else {
		set mergeStr "$mergeStr $outputs(\"static_blank\")"
	}
	
	# combine the left and right outputs to form the bottom
	eval "$outputs(\"gsStr\")$outputs(\"bot_merge\") $mergeStr"
	exec /usr/bin/psnup -q -2 -d -pletter $outputs("bot_merge") $outputs("bot")
}

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
		set mistakeList [lrange $mistakeList 0 3]
	}
	
	foreach mistake $mistakeList {
		set oldPos [lindex $mistake 6]
		set badMove [lindex $mistake 1]
		set badPos [C_DoMove $oldPos $badMove]
		set ret "$ret \"$pathStr\_$badPos.ps\" \"$pathStr\_$oldPos\_v.ps\""
	}
	
	return $ret
}

proc findWorst { mistakeList } {
	set worst $mistakeList
	
	return $worst
}

# do the setup required for a screenshot
# only change Move type if we need to ie
# the position we need to capture hasn't
# been capture yet
proc doCapture { c moveType position theMoves value} {
	set path [makePathOnce $position $value true]
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
		capture $c $position $value true $path
			
		# hide new and display old
		GS_HideMoves $c $type $position $theMoves
		GS_ShowMoves $c $moveType $position $theMoves
		update idletasks
	}
}

# capture a screen shot
# give it the correct name depending on options
proc capture { c pos value rotate path} {
	# capture to game directory
	if { $path != "" } {
		# hide background to reduce drawing time
		$c itemconfigure background -state hidden
		$c postscript -file "$path" -pagewidth 8.0i -rotate $rotate
		# show it again
		$c itemconfigure background -state normal
	}
}

# will generate the path for the given arguments
# but will only return if the path doesn't already
# exist, if it does, then return ""
proc makePathOnce { pos value rotate} {
	set path [makePath $pos $value $rotate]
	
	if { [file exists $path] } {
		return ""
	}
	
	return $path
}

# make path with given arguments
# do the folder checking in here, since other
# functins should call this function to make the
# path
proc makePath { pos value rotate } {
	global kGameName
	checkFolders $kGameName
	
	set path "ps/$kGameName/$kGameName\_$pos"
	if { $value == true} {
		set path "$path\_v"
	}
	
	if { $rotate == false } {
		set path "$path\_nr.ps"
	} else {
		set path "$path.ps"
	}
	
	return $path
}

proc cleanTemp {} {
	global outputs
	# delete temp files
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

# make string to use for eval
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