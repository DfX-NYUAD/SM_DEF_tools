#!/bin/bash

if [ $# -lt 1 ]; then
	echo "Parameters required:"
	echo "1) DEF file to clean up for"
	exit
fi

# parameters
file=$1

# runtime variables
def=${file%.def}

# remove previous attack and RT files
rm $def*.out*
rm $def*.log
