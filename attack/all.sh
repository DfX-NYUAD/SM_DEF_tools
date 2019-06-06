#!/bin/bash

BB_DIVISOR=8
FILES="*/*.def"
#FILES="DEF/original/*.def"

# for 1st run; initialize RT files and split layer 1
for def in `ls $FILES`; do ./clean.sh $def; done
for def in `ls $FILES`; do ./batch.sh $def y $BB_DIVISOR 1; done

## for repeating runs; just run on split layer 1
#for def in `ls $FILES`; do ./batch.sh $def n $BB_DIVISOR 1; done

# remaining split layers
for i in {2..4}
do
	for def in `ls $FILES`; do ./batch.sh $def n $BB_DIVISOR $i; done
done
