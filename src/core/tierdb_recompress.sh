#!/bin/bash
############################################################
# Author: Cameron Cheung
# 
# This shell script is used for recompressing tierdb directories to a new compression
# that allows for fast random access through a lookup table.
# For more information, see this wiki page: ...
# 
# Usage: tierdb_recompress.sh <sourcedir> <destdir>
# sourcedir: a tierdb directory generated prior to the introduction of the new compression
# destdir: where the recompressed files and lookup table should reside. This will be the new tierdb directory.
# 
# This script will do the following:
# 
# Create destdir if not exists and create a lookup directory in it if not exists.
# For each file in sourcedir:
#	 Uncompress it
#	 Split it into 1MB chunks
#	 Compress each 1MB chunk
#	 Concatenate the compressed chunks into a new .gz file to be put into destdir under the same name
#	 Record the sizes of each compressed chunk in order and save to a file (this is a lookup table for this tier). Put this file in the lookup directory.
#

echo $1
echo $2

if [ $# -ne 2 ]
then
	echo "Usage: tierdb_recompress.sh <sourcedir> <destdir>"
	exit
fi

if [ $1 -ef $2 ]
then
	echo "ERROR: sourcedir must be different from destdir."
	exit
fi

if [ ! -d $1 ]
then
	echo "Source directory $1 does not exist."
	exit
fi

if [ -d $1/lookup ]
then
	echo "Lookup directory already exists for $1."
	exit
fi

if [ ! -d $2 ]
then
	mkdir $2
fi

if [ ! -d $2/lookup ]
then
	mkdir $2/lookup
fi

if [[ $2 == /* ]]
then
	destdir=$2
else
	destdir=$PWD/$2
fi

for filename in $1/*.gz
do
	f=`basename $filename`
	if [ ! -f $2/$f ]
	then
		echo "Processing  $f..."
		t=`mktemp -d -t recompXXXXX`
		gunzip -c $filename > $t/intermediate.dat
		cd $t
		split -b1M intermediate.dat splt
		gzip splt*
		cat splt*.gz > final.gz
		stat -c "%s" splt* > final.idx
		mv final.gz $destdir/$f
		mv final.idx $destdir/lookup/$f.idx
		rm splt*
		rm intermediate.dat
		cd - > /dev/null
		rmdir $t
	fi
done
