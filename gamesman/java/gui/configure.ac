



dnl
dnl  Autoconf file for GAMESMAN
dnl  The Finite, Two-person Perfect-Information Game Generator
dnl

# Functions used to determine the right paths for JNI

AC_DEFUN([AC_JNI_INCLUDE_DIR],[

JNI_INCLUDE_DIRS=""

test "x$JAVAC" = x && AC_MSG_ERROR(['$JAVAC' undefined])
AC_PATH_PROG(_ACJNI_JAVAC, $JAVAC, no)
AC_PATH_PROG(_ACJNI_JAVAH, $JAVAH, no)
test "x$_ACJNI_JAVAC" = xno && AC_MSG_ERROR([$JAVAC could not be found in path])

#_ACJNI_FOLLOW_SYMLINKS("$_ACJNI_JAVAC")
#_JTOPDIR=`echo "$_ACJNI_FOLLOWED" | sed -e 's://*:/:g' -e 's:/[[^/]]*$::'`
_JTOPDIR=$JAVA_HOME
_JHOME=$JAVA_HOME



case "$os" in
        Darwin)        _JTOPDIR="/System/Library/Frameworks/JavaVM.framework"
			_JHOME=$_JTOPDIR
			#_JTOPDIR=`echo "$_JTOPDIR" | sed -e 's:/[[^/]]*$::'`
                        _JINC="$_JTOPDIR/Headers"
			_JHOME_INC="$_JHOME/Headers";;

        *)              _JINC="$_JTOPDIR/include"
			_JHOME_INC="$_JHOME/include";;
esac

if test -f "$_JINC/jni.h"; then
        JNI_INCLUDE_DIRS="$JNI_INCLUDE_DIRS $_JINC"
else
	_JTOPDIR=`echo "$_JTOPDIR" | sed -e 's:/[[^/]]*$::'`
	if test	-f "$_JTOPDIR/include/jni.h"; then
	JNI_INCLUDE_DIRS="$JNI_INCLUDE_DIRS $_JTOPDIR/include"
		else
				AC_MSG_ERROR([cannot find java include files])
		fi
fi

# get the likely subdirectories for system specific java includes
case "$os" in
bsdi*)          _JNI_INC_SUBDIRS="bsdos";;
linux*)         _JNI_INC_SUBDIRS="linux genunix";;
osf*)           _JNI_INC_SUBDIRS="alpha";;
solaris*)       _JNI_INC_SUBDIRS="solaris";;
mingw*)			_JNI_INC_SUBDIRS="win32";;
cygwin*)		_JNI_INC_SUBDIRS="win32";;
*)              _JNI_INC_SUBDIRS="genunix linux solaris win32";;
esac

# add any subdirectories that are present
for JINCSUBDIR in $_JNI_INC_SUBDIRS
do
        if test -d "$_JTOPDIR/include/$JINCSUBDIR"; then
                JNI_INCLUDE_DIRS="$JNI_INCLUDE_DIRS $_JTOPDIR/include/$JINCSUBDIR"
        fi
done
])

# _ACJNI_FOLLOW_SYMLINKS <path>
# Follows symbolic links on <path>,
# finally setting variable _ACJNI_FOLLOWED
# --------------------
AC_DEFUN([_ACJNI_FOLLOW_SYMLINKS],[
# find the include directory relative to the javac executable
_cur="$1"
while ls -ld "$_cur" 2>/dev/null | grep " -> " >/dev/null; do
        AC_MSG_CHECKING(symlink for $_cur)
        _slink=`ls -ld "$_cur" | sed 's/.* -> //'`
        case "$_slink" in
        /*) _cur="$_slink";;
        # 'X' avoids triggering unwanted echo options.
        *) _cur=`echo "X$_cur" | sed -e 's/^X//' -e 's:[[^/]]*$::'`"$_slink";;
        esac
        AC_MSG_RESULT($_cur)
done
_ACJNI_FOLLOWED="$_cur"
])# _ACJNI

# End of JNI functions


###
### Initialize
###

AC_INIT(Gamesman Generic Gui, 3.0)
AC_CONFIG_SRCDIR(main/Game.java)


###
### Variables that we'll use
###

OUTLIBSUFFIX=".so"							# Shared library extension
OUTCFLAGS="$CFLAGS -Wall -g -fPIC -c"					# Compiler flags
OUTEXESUFFIX=""								# Executable file extension
OUTOBJSUFFIX=""								# Object file extension
OUTLIBARGS="-shared"

JAVAC=javac
JAVAH=javah

###
### Set up arguments to configure
###

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
          case $machine in
            i*86) machine=ix86;;
          esac;;
  FreeBSD*)   
          case $machine in
            i*86) machine=ix86;;
          esac;;
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
      machine=win32 ;;
  Darwin*)
      machine=ix86; OUTLIBSUFFIX=.jnilib;
       OUTLIBARGS="-bundle -framework JavaVM -lz -L/opt/local/lib -ltcl8.4 -ltk8.4" ;;
esac

MACHINE=$os-$version-$machine

case $MACHINE in
  SunOS-4*)  OS=SUNOS4;;
  SunOS-5*)  OS=SUNOS5;;
  OSF1*)     OS=OSF1;;
  ULTRIX*)   OS=ULTRIX;;  
  HP*)       OS=HPUX; OUTCFLAGS="$OUTCFLAGS -Dhpux"; OUTLIBSUFFIX='.sl';;
  Linux*)    OS=LINUX_ELF;;
  NetBSD-1*) OS=NETBSD1;; 
  FreeBSD*)  OS=FREEBSD;;
  IRIX-5*)   OS=IRIX5;;
  IRIX*-6*)  OS=IRIX5;;
  CYGWIN*)   OS=WIN32; OUTCFLAGS="$OUTCFLAGS -DCYGWIN32";;
  Darwin*)   OS=MACOSX; OUTLIBSUFFIX=".jnilib";;
  *)         AC_MSG_ERROR(this operating system is not supported) ;;
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
AC_ISC_POSIX

###
### Check for header files
###

AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h])



###
### Check for and include the Java headers and programs
###

AC_JNI_INCLUDE_DIR

for JNI_INCLUDE_DIR in $JNI_INCLUDE_DIRS
do
  OUTCFLAGS="$OUTCFLAGS -I$JNI_INCLUDE_DIR"
done

JAVAC=$_ACJNI_JAVAC
JAVAH=$_ACJNI_JAVAH

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
### More crazy cygwin stuff
###

if test "$OS" = "WIN32"
then
  OUTCFLAGS="-Wall -g"
  OUTLDFLAGS="-lm -lc -lz"
  OUTLIBSUFFIX=".dll"
else  #added because cygwin gcc does not know about -pthread
  OUTPYTHONCFLAGS="-pthread $OUTPYTHONCFLAGS"
  OUTPYTHONLIBFLAGS="-pthread $OUTPYTHONLIBFLAGS"
fi


###
### Export some stuff
###

AC_SUBST(LIBSUFFIX, $OUTLIBSUFFIX)
AC_SUBST(CFLAGS, $OUTCFLAGS)
AC_SUBST(EXESUFFIX, $OUTEXESUFFIX)
AC_SUBST(OBJSUFFIX, $OUTOBJSUFFIX)
AC_SUBST(LIBARGS, $OUTLIBARGS)
AC_SUBST(JAVAC, $JAVAC)
AC_SUBST(JAVAH, $JAVAH)


###
### Do output
###

AC_CONFIG_FILES([Makefile]) #$x_scripts])
#AC_CONFIG_COMMANDS([chmod-X-scripts],[./chmod-X-scripts])
AC_OUTPUT

