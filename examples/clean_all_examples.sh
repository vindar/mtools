#!/bin/sh
#
# clean all examples
#

for d in */ ; do
	echo "Cleaning project $d"
	cd $d
	make clean
	cd ..
done

