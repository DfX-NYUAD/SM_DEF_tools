#!/bin/bash

if [ $# -lt 3 ]; then
       echo "Parameters required:"
       echo "1) Log file for run on original DEF"
       echo "2) Log file for run on protected DEF"
       echo "3) The CSV file to be generated"
       exit
fi

orig=$1
protected=$2
CSV=$3

# first, generate temp files to work on
cat $orig | grep LOG_DISTANCES > $orig.tmp
cat $protected | grep LOG_DISTANCES > $protected.tmp

# now, check whether the nets are indeed in same order and of same count
## for that, it seems easier to work on yet another set of temp files
cat $orig.tmp | awk '{print $2}' > $orig.tmp.tmp
cat $protected.tmp | awk '{print $2}' > $protected.tmp.tmp

diff -q $orig.tmp.tmp $protected.tmp.tmp

if [ $? != 0 ]; then

	echo "The nets differ across the log files; double-check also the DEF files"

	# cleanup
	rm $orig.tmp* $protected.tmp*

	exit
fi

# since the nets are indeed the same, build up the CSV file
echo "NET, ORIGINAL DISTANCE, PROTECTED DISTANCE" > $CSV
# orig.tmp.tmp contains still the net names, which we'll also use below; so use yet another tmp file for distances
cat $orig.tmp | awk '{print $4}' > $orig.tmp.tmp.tmp
cat $protected.tmp | awk '{print $4}' > $protected.tmp.tmp

# now, paste all the data into the CSV file
paste -d "," $orig.tmp.tmp $orig.tmp.tmp.tmp $protected.tmp.tmp >> $CSV

# cleanup
rm $orig.tmp* $protected.tmp*
