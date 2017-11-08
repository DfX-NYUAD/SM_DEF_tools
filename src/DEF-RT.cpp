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
#include "DEF-RT.hpp"

// other includes
#include "ParserDEF.hpp"
//#include "ParserLEF.hpp"

int main (int argc, char** argv) {
	DEF_RT converter;

	std::cout << std::endl;
	std::cout << "DEF_RT: converting all the regular nets of a DEF file into the .rt format" << std::endl;
	std::cout << "--------------------------------------------------------------------" << std::endl << std::endl;

	// parse program parameters, and test for DEF/LEF files
	converter.parseParameters(argc, argv);

	// parse in DEF/LEF
	ParserDEF::read(converter.DEF_file, converter.data);
}

void DEF_RT::parseParameters(int const& argc, char** argv) {
	std::ifstream in;

	// print command-line parameters
	//
	if (argc < 2) {
		std::cout << "IO> Usage: " << argv[0] << " DEF_file " << std::endl;
		std::cout << "IO> " << std::endl;
		std::cout << "IO> Mandatory parameter ``DEF_file'': the DEF file to be converted" << std::endl;

		exit(1);
	}

	this->DEF_file = argv[1];

	// test files
	//
	in.open(this->DEF_file.c_str());
	if (!in.good()) {
		std::cout << "IO> ";
		std::cout << "No such file: " << this->DEF_file << std::endl;
		exit(1);
	}
	in.close();

	std::cout << "IO> DEF file: " << this->DEF_file << std::endl;
	std::cout << std::endl;
}
