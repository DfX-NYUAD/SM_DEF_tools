#!/bin/bash

if [ $# -lt 3 ]; then
	echo "Parameters required:"
	echo "1) DEF file to run crouting attack on"
	echo "2) Generate RT files? (y/n)"
	echo "3) Bounding box divisor"
	echo "4) Split layer; note that the crouting attacks supports only layer 8 at max"
	exit
fi

# parameters
def=$1

# runtime variables
benchmark=${def%.def}
rt_file=$benchmark.split2VpinLvl_$4.out

# generate RT files if asked for
if [ $2 == "y" ]; then
	./DEF-RT $def
fi

# clean old log
rm $rt_file.log

# initial info
echo "Original DEF: $def" >> $rt_file.log
echo "RT file: $rt_file" >> $rt_file.log
echo "Bound box divisor: $3" >> $rt_file.log
echo "Split layer: $4" >> $rt_file.log
echo "" >> $rt_file.log

# calculate bounding box parameter for attack
## the first two words in the RT file contain the X and Y dimensions, so we calculate the average of those first
avg=`head -n 1 $rt_file | awk '{print ($1+$2)/2}'`
echo "Baseline for bounding box, derived from RT file by '(X+Y)/2': $avg" >> $rt_file.log
## now, also consider the provided divisor
bb_factor=`echo $avg/$3 | bc`
echo "Bounding box factor (baseline/divisor): $bb_factor" >> $rt_file.log
echo "" >> $rt_file.log
echo "Attack output:" >> $rt_file.log

# run the attack
## the <<< serves to provide $bb_factor as terminal input for Run.jar; see also	https://en.wikipedia.org/wiki/Here_document#Here_strings
begin=`date +%s`

java -Xmx8g -jar Run.jar $rt_file >> $rt_file.log <<< $bb_factor

end=`date +%s`
runtime=`echo $end-$begin | bc`
echo "Attack runtime: $runtime s" >> $rt_file.log

# print the result
cat $rt_file.log
