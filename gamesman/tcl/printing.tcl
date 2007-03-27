global outputs
set outputs("left") "ps/left.ps"
set outputs("top") "ps/top.ps"
set outputs("right") "ps/right.ps"
set outputs("leftMerge") "ps/leftMerge.ps"
set outputs("rightMerge") "ps/rightMerge.ps"
set outputs("output") "ps/output.ps"
set outputs("bot") "ps/bot.ps"
set outputs("botMerge") "ps/botMerge.ps"
set outputs("outputMerge") "ps/outputMerge.ps"
set outputs("outputPDF") "ps/output.pdf"
set outputs("blank") "ps/blank.ps"
set outputs("gsStr") "exec /usr/bin/gs -q -dNOPAUSE -dBATCH -sDEVICE=pswrite -sOutputFile="

proc doPrinting {c position winner} {
	global outputs
	# capture the last position
	# there are no value moves for that...
	capture $c $position false
	makeTop $c $position $winner
	makeBottom
	combine
	# use gs to generate a pdf...
	# only needed for debugging
	makePDF $outputs("output") $outputs("outputPDF")
	#cleanTemp
}

proc prepare { c scale buffer xOffset yOffset nameOfWinner items} {
	# hide oxy background
	$c itemconfigure background -state hidden
	createNew $c $scale $buffer $yOffset $nameOfWinner
	scaleItems $c $items $scale $xOffset $yOffset false
}

proc createNew { c scale buffer yOffset nameOfWinner} {
	global gLeftName gRightName gFrameWidth kGameName
	set players [GS_NameOfPieces]
	set left [lindex $players 0]
	set right [lindex $players 1]
	# create new stuff
	$c create rectangle 0 0 $gFrameWidth $gFrameWidth \
		-fill white -outline "" -tag __printing
	# lower the temp background below the base
	$c lower __printing base
	# create names
	$c create text [expr $gFrameWidth / 2] $buffer -anchor center -text \
		"$gLeftName vs $gRightName" -font {Helvetica 24} -tag __printing
	
	update idletasks
}

proc scaleObjs { c items scale } {
	foreach item $items {
		$c scale $item 0 0 $scale $scale
		#ugly hack to to configure only if item
		#has that property
		catch {
			set wValue [$c itemcget $item -width]
			$c itemconfigure $item -width [expr $wValue * $scale]
		}
	}
}

proc moveObjs { c items newX newY } {
	foreach item $items {
		$c move $item $newX $newY
	}
}

proc scaleItems { c items scale xOffset yOffset undo} {	
	if { $undo  == true } {
		moveObjs $c $items $xOffset $yOffset
		scaleObjs $c $items $scale
	} else {
		scaleObjs $c $items $scale
		moveObjs $c $items $xOffset $yOffset	
	}
	update idletasks
}

# clean up stuff
# undo the scaling
# delete the temporary items
# redisplay oxy
proc cleanUp { c items scale xOffset yOffset} {
	scaleItems $c $items $scale $xOffset $yOffset true
	$c delete __printing
	$c itemconfigure background -state normal
}


# dumps the canvas c to a file called name.ps
# automatically appends ps extension to file
# if no ps directory exists, then it creates it
proc makeTop { c position nameOfWinner} {
	# check for dir
	checkFolders
	global gFrameWidth outputs kGameName
	set outputs("leftMerge") "ps/leftMerge.ps"
	set outputs("rightMerge") "ps/rightMerge.ps"

	# combine 2 column major ones...
	# then combine row major 2
	# should have rotated way we want...
	exec /usr/bin/cp "ps/$kGameName/$kGameName\_$position.ps" $outputs("top")
	set mergeExec "$outputs(\"gsStr\")$outputs(\"leftMerge\") $outputs(\"top\") $outputs(\"top\") $outputs(\"top\") $outputs(\"top\") $outputs(\"top\") $outputs(\"top\") $outputs(\"top\") $outputs(\"top\")"
	eval $mergeExec
	set mergeExec "$outputs(\"gsStr\")$outputs(\"rightMerge\") $outputs(\"top\") $outputs(\"top\") $outputs(\"top\") $outputs(\"top\") $outputs(\"top\") $outputs(\"top\") $outputs(\"top\") $outputs(\"top\")"
	eval $mergeExec
	exec /usr/bin/psnup -c -q -8 -pletter $outputs("leftMerge") $outputs("left")
	exec /usr/bin/psnup -c -q -8 -pletter $outputs("rightMerge") $outputs("right")
	set mergeExec "$outputs(\"gsStr\")$outputs(\"outputMerge\") $outputs(\"left\") $outputs(\"right\")"
	eval $mergeExec
	exec /usr/bin/psnup -q -2 -pletter $outputs("outputMerge") $outputs("top")
	set scale 0.5
	set xOffset [expr $gFrameWidth / 2 -  $gFrameWidth * $scale / 2]
	set yOffset 50
	set buffer 25
	#foreach item [$c gettags base] {
	#	lappend items $item
	#}
	# make the stuff that we need
	#prepare $c $scale $buffer $xOffset $yOffset $nameOfWinner $items
	# dump ps file
	#$c postscript -file $outputs("top") -pagewidth 7.5i -rotate true
	# cleanup stuff that we made
	#cleanUp $c $items [expr 1 / $scale] [expr -$xOffset] [expr -$yOffset]
}

proc makeBottom {} {
	setMistakesLists
	generateMistakeExec
}

proc combine {} {
	global outputs
	# combine bottom and top outputs
	set mergeExec "$outputs(\"gsStr\")$outputs(\"outputMerge\") $outputs(\"bot\") $outputs(\"top\") "
	eval $mergeExec
	exec /usr/bin/psnup -q -2 -pletter $outputs("outputMerge") $outputs("output")
}

proc cleanTemp {} {
	global outputs
	# delete temp files
	# check to see if we made any first
	# don't want to delete the blank.ps file
	file delete $outputs("left")
	file delete $outputs("right")
	file delete $outputs("top")
	if { $outputs("leftMerge") != $outputs("blank") } {
		file delete $outputs("leftMerge")
	}
	if { $outputs("rightMerge") != $outputs("blank") } {
		file delete $outputs("rightMerge")
	}
	file delete $outputs("bot")
	file delete $outputs("botMerge")
	file delete $outputs("outputMerge")
}

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

proc generateMistakeExec { } {
	checkFolders
	
	global leftMistakes rightMistakes outputs
	
	set maxErrors 4
	set outputs("leftMerge") "ps/leftMerge.ps"
	set outputs("rightMerge") "ps/rightMerge.ps"
	set leftExec [makeExec $leftMistakes $maxErrors]
	set rightExec [makeExec $rightMistakes $maxErrors]
	# if we added something then merge and psnup
	# else we change to point to a blank page
	if { $leftExec != "" } {
		set leftExec "$outputs(\"gsStr\")$outputs(\"left\") $leftExec"
		eval $leftExec
		exec /usr/bin/psnup -c -q -[expr 2 * $maxErrors] -pletter $outputs("left") $outputs("leftMerge")
	} else {
		set $outputs("leftMerge") $outputs($outputs("blank"))
	}
	
	if { $rightExec != "" } {
		set rightExec "$outputs(\"gsStr\")$outputs(\"right\") $rightExec"
		eval $rightExec
		exec /usr/bin/psnup -c -q -[expr 2 * $maxErrors] -pletter $outputs("right") $outputs("rightMerge")
	} else {
		set $outputs("rightMerge") $outputs($outputs("blank"))
	}
	
	# combine the left and right outputs
	set mergeExec "$outputs(\"gsStr\")$outputs(\"botMerge\") $outputs(\"rightMerge\") $outputs(\"leftMerge\")"
	eval $mergeExec
	exec /usr/bin/psnup -q -2 -d -pletter $outputs("botMerge") $outputs("bot")
}

proc makeExec { mistakeList maxErrors} {
	global kGameName
	set pathStr "ps/$kGameName/$kGameName"
	set size [llength $mistakeList]
	set ret ""
	# trim mistakes to the worst ones
	if { $size > $maxErrors } {
		#findWorst $leftMistakes
		# truncating for now
		set mistakeList [lrange $mistakeList 0 3]
	}
	
	foreach mistake $mistakeList {
		set oldPos [lindex $mistake 6]
		set badMove [lindex $mistake 1]
		set badPos [C_DoMove $oldPos $badMove]
		set ret "$ret $pathStr\_$badPos.ps $pathStr\_$oldPos\_v.ps"
	}
	
	# pad to 4 errors if needed
	for {set i $size} {$i < $maxErrors } {incr i} {
		set ret "$ret ps/blank.ps ps/blank.ps"
	}
	
	return $ret
}

proc findWorst { mistakeList } {
	set worst [list]
	
	return $worst
}

# make string to use for eval
# then call it
proc makePDF { input output} {
	set gsStr "exec /usr/bin/gs -q -dNOPAUSE -dBATCH -sOutputFile=$output"
	eval "$gsStr -sDEVICE=pdfwrite -c .setpdfwrite -f $input"
}

# do the setup required for capturing a move
# 
proc doCapture { c moveType position theMoves value} {
	# hide old and display new
	GS_HideMoves $c $moveType $position $theMoves
	if {$value == true} {
		GS_ShowMoves $c "value" $position $theMoves
	} else {
		GS_ShowMoves $c "all" $position $theMoves		
	}
	update idletasks
		
	# capture screen shot
	capture $c $position $value
		
	# hide new and display old
	if {$value == true} {
		GS_HideMoves $c "value" $position $theMoves
	} else {
		GS_HideMoves $c "all" $position $theMoves
	}
	GS_ShowMoves $c $moveType $position $theMoves
	update idletasks
}

proc capture { c pos value} {
	checkFolders
	
	global kGameName
	set name "$kGameName\_$pos"
	if { $value == true} {
		set name "$name\_v.ps"
	} else {
		set name "$name.ps"		
	}
	
	# capture to game directory
	if { ![file exists "ps/$kGameName/$name"] } {
		# hide background to reduce drawing time
		$c itemconfigure background -state hidden
		$c postscript -file "ps/$kGameName/$name" -pagewidth 7.5i -rotate true
		# show it again
		$c itemconfigure background -state normal
	}
}

proc checkFolders {} {
	global kGameName
	# check for directories
	if { ![file exists "ps/"] } {
		file mkdir "ps"
	}
	
	if { ![file exists "ps/$kGameName"] } {
		file mkdir "ps/$kGameName"
	}
}

proc makeWinnerBanner {} {
}