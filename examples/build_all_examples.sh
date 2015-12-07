#!/bin/sh
#
# Build all the projects examples
#

for d in */ ; do
	echo
	echo "-------------------------------------"
	echo "Building project $d"
	echo "-------------------------------------"
	cd $d
	make
	cd ..
done



