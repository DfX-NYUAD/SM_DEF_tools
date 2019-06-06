This folder relates to J. Magaña, D. Shi, and A. Davoodi. Are proximity attacks a threat to the security of split manufacturing of integrated
circuits? In International Conf. on Computer-Aided Design, pages 1-7, 2016.

The attack executable, Run.jar, is as provided by https://jonathoncmagana.github.io/magana/index.html

The parsing/preparation of files is done in our own code, provided in that repository.

See also below for some hints on the file format for the attack Run.jar:

STEP 1, Parser
--------------
Usage: java -Xmx4g -jar Parser.jar superblue1CGRIP 8
	
	where the arguments after Parser.jar are: 
	inFile[.rt excluded] splitLVL_at_and_below
	in the example above, superblue1CGRIP.rt is in the same directory as Parser.jar

	output: superblue1CGRIP.split2VpinLvl_8.out (etc.)

Parser.jar uses input values for various CGRIP benchmarks stored in the Parser.conf file. Additional benchmarks can be added as additional lines:
704 516 9 32 32 superblue1CGRIP
774 713 9 32 32 superblue5CGRIP
638 968 9 32 32 superblue10CGRIP
444 518 9 32 40 superblue12CGRIP
381 404 9 32 40 superblue18CGRIP

where each line is (information from .route file):
gridX gridY gridZ tileX tileY BenchmarkName


STEP 2, Run
-----------
usage: 	java -Xmx8g -jar Run.jar superblue1CGRIP.split2VpinLvl_8.out 

	where the arguments after Run.jar are:
	the file created in step 1
	*note the program may run with a smaller max_ram amount (than -Xmx8g)

	output: superblue1CGRIP.split2VpinLvl_8.out.candidatesPerNetSplitLvl_8.out

	The output file contains a list of nets, and below each net a list of candidate nets in the crouting proximity candidate list. 

	Run.jar also prints statistics for that benchmark/split level such as average list size and %match found in list. Note these statics are generated with an optimized version of our code which processes additional 2 vpin nets, and so the number of vpins is slightly higher than the ICCAD paper results in reported in Table I. 



