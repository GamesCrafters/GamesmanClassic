#!/bin/sh
# here is a sample html viewer to demonstrate the library usage
# Copyright (c) 1995 by Sun Microsystems
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# This REQUIRES Tk4.0 -- make sure "wish" on the next line is a 4.0 version
# The next line is a TK comment, but a shell command \
#  exec wish4.0 -f "$0" "$@" & exit 0

if {$tk_version < 4.0 || [regexp {b[123]} $tk_patchLevel] } {
	puts stderr "This library requires TK4.0, this is only $tk_version, \
			patchlevel $tk_patchLevel"
	exit 1
}
if {[catch {array get env *}]} {
	puts stderr "This library requires tcl7.4, this version is too old!"
	exit 1
}

source ../tcl/html_library.tcl


# Go render a page.  We have to make sure we don't render one page while
# still rendering the previous one.  If we get here from a recursive 
# invocation of the event loop, cancel whatever we were rendering when
# we were called.
# If we have a fragment name, try to go there.

proc render {file text} {
	global HM.text Url 

	set fragment ""
	regexp {([^#]*)#(.+)} $file dummy file fragment
	if {$file == "" && $fragment != ""} {
		HMgoto $text $fragment
		return
	}
	HMreset_win $text
	update idletasks
	if {$fragment != ""} {
		HMgoto $text $fragment
	}
	set Url $file
		  HMparse_html [get_html $file] {HMrender $text} $text
	HMset_state $text -stop 1	;# stop rendering previous page if busy

}

# given a file name, return its html, or invent some html if the file can't
# be opened.

proc get_html {file} {
	if {[catch {set fd [open $file]} msg]} {
		return "
			<title>Bad file $file</title>
			<h1>Error reading $file</h1><p>
			$msg<hr>
		"
	}
	set result [read $fd]
	close $fd
	return $result
}



# Supply an image callback function
# Read in an image if we don't already have one
# callback to library for display

proc HMset_image {win handle src} {
	global Url
	if {[string match /* $src]} {
		set image $src
	} else {
		set image [file dirname $Url]/$src
	}
	update
	if {[string first " $image " " [image names] "] >= 0} {
		HMgot_image $handle $image
	} else {
	    set type photo
	    if {[file extension $image] == ".bmp"} {set type bitmap}
	    if {[file extension $image] == ".jpg"} {
		exec convert $image [file rootname $image].gif
		set image [file rootname $image].gif
	    }
	    catch {image create $type $image -file $image} image
	    HMgot_image $handle $image
	}
}

# Handle base tags.  This breaks if more than 1 base tag is in the document

proc HMtag_base {win param text} {
	global Url
	upvar #0 HM$win var
	HMextract_param $param href Url
}

# downloading fonts can take a long time.  We'll override the default
# font-setting routine to permit better user feedback on fonts.  We'll
# keep our own list of installed fonts on the side, to guess when delays
# are likely

proc HMset_font {win tag font} {
	global Fonts
	if {![info exists Fonts($font)]} {
		set Fonts($font) 1
		update
	}
	catch {$win tag configure $tag -font $font}
}

# Lets invent a new HTML tag, just for fun.
# Change the color of the text. Use html tags of the form:
# <color value=blue> ... </color>
# We can invent a new tag for the display stack.  If it starts with "T"
# it will automatically get mapped directly to a text widget tag.

proc HMtag_color {win param text} {
	upvar #0 HM$win var
	set value bad_color
	HMextract_param $param value
	$win tag configure $value -foreground $value
	HMstack $win "" "Tcolor $value"
}

proc HMtag_/color {win param text} {
	upvar #0 HM$win var
	HMstack $win / "Tcolor {}"
}

# Add a font size manipulation primitive, so we can use this sample program
# for on-line presentations.  sizes prefixed with + or - are relative.
#  <font size=[+-]3>  ..... </font>.  Note that this is not the same as
# Netscape's <font> tag.

proc HMtag_font {win param text} {
	upvar #0 HM$win var
	set size 0; set sign ""
	HMextract_param $param size
	regexp {([+-])? *([0-9]+)} $size dummy sign size
	if {$sign != ""} {
		set size [expr [lindex $var(size) end] $sign $size]
	}
	HMstack $win {} "size $size"
}

# This version is closer to what Netscape does

proc HMtag_font {win param text} {
	upvar #0 HM$win var
	set size 0; set sign ""
	HMextract_param $param size
	regexp {([+-])? *([0-9]+)} $size dummy sign size
	if {$sign != ""} {
		set size [expr [lindex $var(size) end] $sign  $size*2]
		HMstack $win {} "size $size"
	} else {
		HMstack $win {} "size [expr 10 + 2 * $size]"
	}
}

proc HMtag_/font {win param text} {
	upvar #0 HM$win var
	HMstack $win / "size {}"
}

proc HMtag_style {win param text} {
	upvar $text data
	set data ""
}

proc convert_html_tcl { Home c fontcolor bgcolor text scrollbar} {
    # set initial values
    set Size 4					;# font size adjustment
    set Indent 1.2				;# tab spacing (cm)
    
    # make the interface and render the home page
 
    HMinit_win $text
    HMset_state $text -size $Size
    HMset_indent $text $Indent
    render $Home $text
    $text configure -foreground $fontcolor -background $bgcolor
    return $text
    
    
}
