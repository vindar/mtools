#!/bin/sh
#
# Run the make command on all the sub-directories

cd "${0%/*}"
DIR=$PWD

for d in */ ; do
	echo
	echo "-------------------------------------"
	echo "Project $d"
	cd $d
	make $1
	cd "$DIR"
done



