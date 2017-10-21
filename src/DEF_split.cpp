/*
 * =====================================================================================
 *
 *    Description:  DEF splitting handler
 *
 *    Copyright (C) 2017 Johann Knechtel, johann aett jknechtel dot de
 *
 * =====================================================================================
 */

// own header
#include "DEF_split.hpp"

int main (int argc, char** argv) {
	DEF_split split;

	std::cout << std::endl;
	std::cout << "DEF_split: splitting a DEF file into FEOL/BEOL, after a give metal layer" << std::endl;
	std::cout << "------------------------------------------------------------------------" << std::endl << std::endl;

	// parse program parameters, and test for DEF/LEF files
	split.parseParametersFiles(argc, argv);

	// TODO parse DEF/LEF
	// TODO split DEF into FEOL/BEOL
	// TODO write back FEOL part
}

void DEF_split::parseParametersFiles(int const& argc, char** argv) {
	std::ifstream in;

	// print command-line parameters
	if (argc < 4) {
		std::cout << "IO> Usage: " << argv[0] << " DEF_file LEF_file split_layer" << std::endl;
		std::cout << "IO> " << std::endl;
		std::cout << "IO> Mandatory parameter ``DEF_file'': the DEF file to be split" << std::endl;
		std::cout << "IO> Mandatory parameter ``LEF_file'': the LEF file related to the DEF file" << std::endl;
		std::cout << "IO> Mandatory parameter ``split_layer'': the layer after which to split the DEF file" << std::endl;

		exit(1);
	}

	this->DEF_file = argv[1];
	this->LEF_file = argv[2];
	this->split_layer = atoi(argv[3]);

	// test files
	//
	in.open(this->DEF_file.c_str());
	if (!in.good()) {
		std::cout << "IO> ";
		std::cout << "No such file: " << this->DEF_file << std::endl;
		exit(1);
	}
	in.close();

	in.open(LEF_file.c_str());
	if (!in.good()) {
		std::cout << "IO> ";
		std::cout << "No such file: " << LEF_file << std::endl;
		exit(1);
	}
	in.close();


	std::cout << "IO> DEF file: " << this->DEF_file << std::endl;
	std::cout << "IO> LEF file: " << this->LEF_file << std::endl;
	std::cout << "IO> Metal layer to split after: " << this->split_layer << std::endl;
	std::cout << std::endl;
}
