#!/bin/bash

if [ $# -lt 3 ]; then
	echo "Parameters required:"
	echo "1) Bounding box divisor"
	echo "2) Top-most split layer; note that the crouting attacks supports only layer 8 at max"
	echo "3) Folder containing the DEF files"
	echo "4) Optional; cleanup previous RT files and attack files? (y/n)"
	exit
fi

BB_DIVISOR=$1
TOP=$2
FILES="$3/*.def"

for def in `ls $FILES`
do
	# cleanup, if requested for
	if [ "$4" == "y" ]; then
		file=${def%.def}
		rm $file*.out*
	fi

	# initialize RT files, if not present yet
	benchmark=${def%.def}
	rt_file=$benchmark".split2VpinLvl_1.out"

	if [ ! -f "$rt_file" ]; then
		./DEF-RT $def
	fi
done

# handle all split layers
for ((layer = 1; layer <= TOP; layer++))
do
	for def in `ls $FILES`; do ./batch.sh $def $BB_DIVISOR $layer; done
done
