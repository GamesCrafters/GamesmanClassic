
dnl
dnl  Autoconf file for GAMESMAN
dnl  The Finite, Two-person Perfect-Information Game Generator
dnl


###
### Initialize
###

AC_INIT([gamesman],[3.0])
AC_CONFIG_SRCDIR(src/core/gamesman.h)


###
### Variables that we'll use
###

#WARNGC="DO NOT EDIT THIS FILE!  Instead, edit"				# Warning text, deprecated
OUTLIBSUFFIX=".so"							# Shared library extension
COMPILEGRAPHICS=""							# Whether we should build graphics
OUTCFLAGS="$CFLAGS -Wall -Wextra -DNDEBUG -g"		    # Compiler flags
OUTLDFLAGS=""								# Linker flags
OUTTCLCFLAGS=""								# Tcl-related compiler flags
OUTTCLSOFLAGS=""							# Tcl-related linker flags (for shared objects)
OUTTCLEXEFLAGS=""							# Tcl-related linker flags (for executables)
OUTEXESUFFIX=""								# Executable file extension
OUTOBJSUFFIX=""								# Object file extension
OUTRMOBJS=""								# How to remove all object files
#OUTWISHLOC=""								# Location of wish, deprecated
OUTPYTHONCFLAGS="-fno-strict-aliasing -DNDEBUG -fPIC -c -g"	# The flags for compiling pygamesman

###
### Set up arguments to configure
###

AC_ARG_ENABLE(graphics, AS_HELP_STRING([--enable-graphics],[enable Tcl/Tk graphical frontends to games (default: yes)]),
		graphics="$enable_graphics",
		graphics="yes")
AC_ARG_WITH(tcl, AS_HELP_STRING([--with-tcl=path],[use the specified tclConfig.sh file to configure Tcl (default: look in common locations)]),
		tcl="$with_tcl",
		tcl="")
AC_ARG_WITH(tk, AS_HELP_STRING([--with-tk=path],[use the specified tkConfig.sh file to configure Tk (default: look in common locations)]),
		tk="$with_tk",
		tk="")
#wish will have to be in the path from now on. deprecated.
#AC_ARG_WITH(wish, AS_HELP_STRING([--with-wish=path],[use the specified wish shell to run Gamesman (default: look in common locations)]),
#		wish="$with_wish",
#		wish="")
AC_ARG_WITH(aqua, AS_HELP_STRING([--with-aqua],[use Aqua Tcl/Tk instead of X for graphics (Mac OS X only) (default: yes)]),
		aqua="yes",
		aqua="no")
AC_PREFIX_DEFAULT(.)


###
### Check system type
###

AC_MSG_CHECKING(for operating system type)
os=`uname -s`
version=`uname -r`
machine=`uname -m`

case $os in
  Linux*) 
          case $version in
            1.*) version=1.X;;
            2.*) version=2.X;;
          esac
		  ;;
  FreeBSD*)   
		  ;;
  SunOS*)   
          case $version in
            4.1.*) version=4.1.X;;
          esac
          case $machine in
            sun4*) machine=sun4;;
          esac;;
  IRIX*) 
          case $version in
            5.*) version=5.X;;
            6.*) version=6.X;;
          esac
          case $machine in
           IP*) machine=IPxx;;
          esac;;
  HP-UX*)
          case $version in
            A.09.*) version=9.X;;
            A.10.*) version=10.X;;
          esac
          case $machine in
            9000/7*) machine=9000s700;;
          esac;;
  CYGWIN*)
          ;;
  Darwin*)
	case $machine in
	      i*86) machine=$machine;;
	      *) machine=powerpc;;
	esac;;
esac

case $machine in
  i*86)
  		  OUTCFLAGS="$OUTCFLAGS -march=$machine";;
  powerpc)
          OUTCFLAGS="$OUTCFLAGS ";;
  *)
          AC_MSG_WARN(Unusual processor architecture detected. Skipping architecture specifiers for gcc.);;
esac

MACHINE=$os-$version-$machine

case $MACHINE in
  SunOS-4*)  OS=SUNOS4;;
  SunOS-5*)  OS=SUNOS5;;
  OSF1*)     OS=OSF1;;
  ULTRIX*)   OS=ULTRIX;;  
  HP*)       OS=HPUX
  			 OUTCFLAGS="$OUTCFLAGS -Dhpux"
  			 OUTLIBSUFFIX='.sl';;
  Linux*)    OS=LINUX_ELF;;
  NetBSD-1*) OS=NETBSD1;; 
  FreeBSD*)  OS=FREEBSD;;
  IRIX-5*)   OS=IRIX5;;
  IRIX*-6*)  OS=IRIX5;;
  CYGWIN*)   OS=WIN32
			 OUTLIBSUFFIX=".dll";;
  Darwin*)   OS=MACOSX
  			 OUTLIBSUFFIX=".dylib";;
  *)         OS=UNKNOWN
  			 AC_MSG_WARN(This operating system is not supported, will use defaults that may or may not work.) ;;
esac

OUTCFLAGS="$OUTCFLAGS -D$OS"

AC_MSG_RESULT($MACHINE)


###
### Check programs
###

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CC_C_O
AC_PROG_RANLIB
AC_CHECK_TOOL(AR, ar, :)
AC_SEARCH_LIBS([strerror],[cposix])

###
### Check for integer types
###
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

###
### Check for libraries
###

AC_SEARCH_LIBS(malloc, c)
AC_SEARCH_LIBS(cos, m)
AC_SEARCH_LIBS(connect, socket)
AC_SEARCH_LIBS(gethostbyname, nsl)
AC_SEARCH_LIBS(gzopen, z,,AC_MSG_ERROR([install zlib (http://www.zlib.org/)]))

OUTLDFLAGS="$OUTLDFLAGS $LIBS"


###
### Check for header files
###

AC_CHECK_HEADERS([errno.h fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h])

# Make sure GNU MP is installed as bignums are needed for univt
AC_CHECK_LIB(gmp, __gmpz_init, [
	OUTGMPLIBFLAGS="-lgmp"
	OUTGMPCFLAGS="-DHAVE_GMP"
	], [AC_MSG_WARN([GNU MP not found, univdb option will not be supported])])
	
# Make sure XML Parser is installed as it is required for Static Evaluator
AC_CHECK_LIB(expat, XML_ParserCreate,
    [expat_found=yes],
    [AC_MSG_WARN([Expat (libexpat.org) not found, saving static evaluator will not be supported.])])
AC_CHECK_LIB(scew, scew_attribute_create,
    [scew_found=yes],
	  [AC_MSG_WARN([SCEW (http://www.nongnu.org/scew/) not found, saving static evaluator will not be supported.])])
OUTXMLLIBFLAGS=""
OUTXMLCFLAGS=""
if test "$expat_found" = "yes" -a "$scew_found" = "yes"
then
	  OUTXMLLIBFLAGS="-lscew -lexpat"
	  OUTXMLCFLAGS="-DHAVE_XML"
fi


###
### An attempt to detect python
###

### go through the common locations for the header
PYTHON_INCLUDE_PATH=""
if test "$OS" = "MACOSX"
then
  _python_locations="/Library/Frameworks/Python.framework/include"
else
  _python_locations="/usr/include/python2.4 /usr/include/python2.3 /usr/local/include/python2.4 /usr/local/include/python2.3"
fi

for testpath in $_python_locations
do
  if test "$PYTHON_INCLUDE_PATH" = ""
    then AC_CHECK_HEADER([$testpath/Python.h],[PYTHON_INCLUDE_PATH=$testpath])
  fi
done

### see if we have it
if test "$PYTHON_INCLUDE_PATH" = ""
then
  AC_MSG_WARN([Python headers not found. You may not be able to run the python stuff here.])
else
  OUTPYTHONCFLAGS="$OUTPYTHONCFLAGS -I$PYTHON_INCLUDE_PATH"
fi



###
### Check compiler characteristics
###

# TODO: We need to *do* something depending on these variables

AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

AC_CHECK_HEADERS_ONCE([sys/time.h])


AC_C_PROTOTYPES


###
### Check common library functions
###

AC_FUNC_FORK
AC_FUNC_MALLOC


###
### Extension stuff
###

if test "$EXEEXT" != ""
then
  OUTEXESUFFIX="$EXEEXT"
else
  OUTEXESUFFIX=""
fi

if test "$OBJEXT" != ""
then
  OUTOBJSUFFIX=".$OBJEXT"
  OUTRMOBJS="*.$OBJEXT"
fi



###
### Look for Tcl, Tk, and wish if necessary
###

if test "$graphics" != "no"
then
  tclconf=""
  tkconf=""
  _common_tcl_locations="/usr/lib /usr/lib64 /usr/local/lib /lib /sw/lib /usr/sww/lib /usr/lib/tcl8.4 /usr/lib/tcl8.5"
  _common_tk_locations="/usr/lib /usr/lib64 /usr/local/lib /lib /sw/lib /usr/sww/lib /usr/lib/tk8.4 /usr/lib/tk8.5"
  _mac_locations="/Library/Frameworks /System/Library/Frameworks"
  
  ### Look for tclConfig.sh and tkConfig.sh
  
  # Check user-provided files, if any
  if test "$tcl" != ""
  then
    AC_CHECK_FILE($tcl, tclconf=$tcl)
  fi
  if test "$tk" != ""
  then
    AC_CHECK_FILE($tk, tkconf=$tk)
  fi
#  if test "$wish" != ""
#  then
#    AC_CHECK_FILE($wish, OUTWISHLOC=$wish)
#  fi
  
  # Otherwise search.
  # On Macs, we want to clearly distinguish X11 Tcl/Tk/Wish from Aqua Tcl/Tk/Wish
  # so the procedure is much more complicated...
  if test "$OS" = "MACOSX"
  then
    # First, see if we can find the Aqua stuff.  If we're not looking for Aqua, then this
    # will help us determine what to *not* search for.
    flocs=$_mac_locations
    aquatcl=""
    aquatk=""
    
    for fcheck in $flocs
    do
      if test "$aquatcl" = ""
      then
        AC_CHECK_FILE([$fcheck/Tcl.framework/tclConfig.sh], aquatcl="$fcheck/Tcl.framework/tclConfig.sh")
      fi
      if test "$aquatk" = ""
      then
        AC_CHECK_FILE([$fcheck/Tk.framework/tkConfig.sh], aquatk="$fcheck/Tk.framework/tkConfig.sh")
      fi
    done
    
    if test "$aqua" = "yes"
    then
      # Use what we found
      if test "$aquatcl" != "" -a "$tclconf" = ""
      then
        tclconf=$aquatcl
      fi
      if test "$aquatk" != "" -a "$tkconf" = ""
      then
        tkconf=$aquatk
      fi
    else
      _avoid_aqua=yes
      # Otherwise, use the T*_LIB_SPEC variable to filter out aqua results.
      if test "$tclconf" = "" -a "$aquatcl" != ""
      then
        . $aquatcl
        AQUA_TCL_LIB_SPEC=$TCL_LIB_SPEC
        TCL_LIB_SPEC=""
      fi
      if test "$tkconf" = "" -a "$aquatk" != ""
      then
        . $aquatk
        AQUA_TK_LIB_SPEC=$TK_LIB_SPEC
        TK_LIB_SPEC=""
      fi
      
      # Now search
      flocs=$_common_tcl_locations
      
      for fcheck in $flocs
      do
        if test "$tclconf" = ""
        then
          AC_CHECK_FILE([$fcheck/tclConfig.sh], tclconf="$fcheck/tclConfig.sh")
          if test "$tclconf" != ""
          then
            . $tclconf
            if test "$TCL_LIB_SPEC" = "$AQUA_TCL_LIB_SPEC"
            then
              tclconf=""
            fi
          fi
        fi
        if test "$tkconf" = ""
        then
          AC_CHECK_FILE([$fcheck/tkConfig.sh], tkconf="$fcheck/tkConfig.sh")
          if test "$tkconf" != ""
          then
            . $tkconf
            if test "$TK_LIB_SPEC" = "$AQUA_TK_LIB_SPEC"
            then
              tkconf=""
            fi
          fi
        fi
      done
      
      # Reset stuff we're going to use (since we loaded it in this branch)
      TCL_LIBS=""
      TK_LIBS=""
      TK_LIB_SPEC=""
      TCL_LIB_SPEC=""
      TCL_INCLUDE_SPEC=""
      TK_XINCLUDES=""
      TCL_EXTRA_CFLAGS=""
      TCL_SHLIB_CFLAGS=""
    fi
    OUTTCLSOFLAGS="$OUTTCLSOFLAGS -dynamiclib -flat_namespace"
  else
    flocs=$_common_tcl_locations
    
    for fcheck in $flocs
    do
      if test "$tclconf" = ""
      then
        AC_CHECK_FILE([$fcheck/tclConfig.sh], tclconf="$fcheck/tclConfig.sh")
      fi
    done

    flocs=$_common_tk_locations
    for fcheck in $flocs
    do
      if test "$tkconf" = ""
      then
        AC_CHECK_FILE([$fcheck/tkConfig.sh], tkconf="$fcheck/tkConfig.sh")
      fi
    done
    # Added -static-
    OUTTCLSOFLAGS="$OUTTCLSOFLAGS -shared -static-libgcc"
  fi
  
  ### Final tcl/tk checks
  
  AC_MSG_CHECKING(for Tcl)
  
  if test "$tclconf" = ""
  then
    AC_MSG_RESULT(no)
    if test "$aquatcl" = ""
    then
      AC_MSG_ERROR([Tcl not found.  Specify the tclConfig.sh file to use with the --with-tcl=/path/to/tclConfig.sh option])
    else
      AC_MSG_ERROR([Non-aqua Tcl not found.  Specify the tclConfig.sh file to use with the --with-tcl=/path/to/tclConfig.sh option, or use --with-aqua (not recommended)])
    fi
  fi
  
  . $tclconf
  
  AC_MSG_RESULT($TCL_VERSION$TCL_PATCH_LEVEL)
  
  if test "$OS" = "MACOSX" -a "$TCL_VERSION" = "8.3"
  then
    AC_MSG_ERROR([Tcl/Tk 8.3 is not supported for Mac.  Please upgrade.  Visit http://tcl.activestate.com])
  fi
  
  AC_MSG_CHECKING(for Tk)
  
  if test "$tkconf" = ""
  then
    AC_MSG_RESULT(no)
    if test "$aquatk" = ""
    then
      AC_MSG_ERROR([Tk not found.  Specify the tkConfig.sh file to use with the --with-tk=/path/to/tkConfig.sh option])
    else
      AC_MSG_ERROR([Non-aqua Tk not found.  Specify the tkConfig.sh file to use with the --with-tk=/path/to/tkConfig.sh option, or use --with-aqua (not recommended)])
    fi
  fi
  
  . $tkconf
  
  AC_MSG_RESULT($TK_VERSION$TK_PATCH_LEVEL)
  
  ### Now look for wish... decprecated
#  flocs="/bin /usr/bin /usr/local/bin /usr/sww/bin /sw/bin"
#  for fcheck in $flocs
#  do
#    if test "$OUTWISHLOC" = ""
#    then
#      AC_CHECK_FILE([$fcheck/wish$TK_VERSION$OUTEXESUFFIX], OUTWISHLOC="$fcheck/wish$TK_VERSION$OUTEXESUFFIX")
#      if test "$OUTWISHLOC" != "" -a "$_avoid_aqua" = "yes"
#      then
#        _tst=`grep "#!/bin/sh" $OUTWISHLOC`
#        if test "$_tst" != ""
#        then
#          OUTWISHLOC=""
#        fi
#      fi
#    fi
#    if test "$OUTWISHLOC" = ""
#    then
#      AC_CHECK_FILE([$fcheck/wish$OUTEXESUFFIX], OUTWISHLOC="$fcheck/wish$OUTEXESUFFIX")
#      if test "$OUTWISHLOC" != "" -a "$_avoid_aqua" = "yes"
#      then
#        _tst=`grep "#!/bin/sh" $OUTWISHLOC`
#        if test "$_tst" != ""
#        then
#          OUTWISHLOC=""
#        fi
#      fi
#    fi
#  done
#  
#  AC_MSG_CHECKING(for wish)
#  
#  if test "$OUTWISHLOC" = ""
#  then
#    AC_MSG_RESULT(no)
#    if test "$_avoid_aqua" != "yes" -o "$aquatk" = ""
#    then
#      AC_MSG_ERROR([Wish not found.  Specify its location using the --with-wish=/path/to/wish])
#      AC_MSG_ERROR([option.])
#    else
#      AC_MSG_ERROR([Non-Aqua wish not found.  Specify the wish shell to use with the])
#      AC_MSG_ERROR([--with-wish=/path/to/wish option, or use --with-aqua (not recommended)])
#    fi
#  fi
  
#  AC_MSG_RESULT($OUTWISHLOC)
  
  # Cygwin is pretty screwed up.. but this seems to get it to work
  if test "$OS" = "WIN32"
  then
    OUTTCLSOFLAGS="$OUTTCLSOFLAGS -ltcl -ltk"
    OUTTCLEXEFLAGS="$OUTTCLEXEFLAGS -ltcl -ltk"
    OUTTCLCFLAGS="$OUTTCLCFLAGS $TCL_EXTRA_CFLAGS"
  else
    OUTTCLSOFLAGS="$OUTTCLSOFLAGS $TK_LIB_SPEC $TCL_LIB_SPEC"
    OUTTCLCFLAGS="$OUTTCLCFLAGS $TCL_INCLUDE_SPEC $TK_XINCLUDES $TCL_SHLIB_CFLAGS"
    OUTTCLEXEFLAGS="$OUTTCLEXEFLAGS $TCL_LIB_SPEC"
    OUTLIBSUFFIX="$TCL_SHLIB_SUFFIX"
  fi
  
  COMPILEGRAPHICS="so_all"
else
  OUTCFLAGS="$OUTCFLAGS -DNO_GRAPHICS"
fi


###
### More crazy cygwin stuff
###

if test "$OS" != "WIN32"
then  #added because cygwin gcc does not know about -pthread
  OUTPYTHONCFLAGS="-pthread $OUTPYTHONCFLAGS"
  OUTPYTHONLIBFLAGS="-pthread $OUTPYTHONLIBFLAGS"
fi

#Macros:
#AC_C_BIGENDIAN

###
### Export some stuff
###

#AC_SUBST(WARNGC, $WARNGC)
AC_SUBST(LIBSUFFIX, $OUTLIBSUFFIX)
AC_SUBST(COMPILEGRAPHICS, $COMPILEGRAPHICS)
AC_SUBST(CFLAGS, $OUTCFLAGS)
AC_SUBST(LDFLAGS, $OUTLDFLAGS)
AC_SUBST(TCLSOFLAGS, $OUTTCLSOFLAGS)
AC_SUBST(TCLCFLAGS, $OUTTCLCFLAGS)
AC_SUBST(TCLEXEFLAGS, $OUTTCLEXEFLAGS)
AC_SUBST(EXESUFFIX, $OUTEXESUFFIX)
AC_SUBST(OBJSUFFIX, $OUTOBJSUFFIX)
AC_SUBST(RMOBJS, $OUTRMOBJS)
AC_SUBST(TCLDBGX, $TCLDBGX)
#AC_SUBST(WISHLOC, $OUTWISHLOC)
AC_SUBST(PYTHONCFLAGS, $OUTPYTHONCFLAGS)
AC_SUBST(PYTHONLIBFLAGS, $OUTPYTHONLIBFLAGS)
AC_SUBST(GMPCFLAGS, $OUTGMPCFLAGS)
AC_SUBST(GMPLIBFLAGS, $OUTGMPLIBFLAGS)
AC_SUBST(XMLCFLAGS, $OUTXMLCFLAGS)
AC_SUBST(XMLLIBFLAGS, $OUTXMLLIBFLAGS)

###
### Do output
###

AC_CONFIG_FILES([Makefile src/Makefile src/core/Makefile src/core/filedb/Makefile])
AC_OUTPUT