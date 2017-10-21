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
	split.parseParameters(argc, argv);

	// parse DEF/LEF
	split.parseDEF();
	split.parseLEF();

	// TODO split DEF into FEOL/BEOL
	// TODO write back FEOL part
}

void DEF_split::parseParameters(int const& argc, char** argv) {
	std::ifstream in;

	// print command-line parameters
	//
	if (argc < 4) {
		std::cout << "IO> Usage: " << argv[0] << " DEF_file LEF_file split_layer" << std::endl;
		std::cout << "IO> " << std::endl;
		std::cout << "IO> Mandatory parameter ``DEF_file'': the DEF file to be split" << std::endl;
		std::cout << "IO> Mandatory parameter ``LEF_file'': the LEF file related to the DEF file" << std::endl;
		std::cout << "IO> Mandatory parameter ``split_layer'': the layer after which to split the DEF file -- provide string, e.g., metal2" << std::endl;

		exit(1);
	}

	this->DEF_file = argv[1];
	this->LEF_file = argv[2];
	this->split_layer = argv[3];

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

void DEF_split::parseDEF() {
	FILE *DEF;

	std::cout << "DEF> Start parsing DEF file ..." << std::endl;

	// pointer to user data; made available in all parser callbacks
	Data* userData = &this->data;

	DEF = fopen(this->DEF_file.c_str(), "r");

	defrInit();
	defrSetUserData(userData);

	//TODO for new Si2 parser
	//defrInitSession(1);

	// define callback functions
	//

	//nets
	defrSetNetStartCbk((defrIntegerCbkFnType) parseNetsStart);
	defrSetNetEndCbk((defrVoidCbkFnType) parseNetsEnd);
//	defrSetNetCbk((defrNetCbkFnType) parseNets);

	// trigger parser; read DEF sections of interes
	//
	// 4th parameter: 1 -- specifies that the data is case sensitive
	int status = defrRead(DEF, this->DEF_file.c_str(), userData, 1);
	if (status != 0) {
		std::cout << "DEF> Error in parser; abort" << std::endl;
		exit (1);
	}

	defrClear();
	fclose(DEF);

	std::cout << "DEF> End parsing DEF file" << std::endl;
}

void DEF_split::parseLEF() {
}

int DEF_split::parseNetsStart(defrCallbackType_e typ, int nets, defiUserData* userData) {

	std::cout << "DEF>  Parsing NETS ..." << std::endl;

	Data* data = reinterpret_cast<Data*>(userData);

	data->DEF_items.nets = static_cast<unsigned>(nets);

	std::cout << "DEF>   " << nets << " nets to be parsed ..." << std::endl;

	return 0;
}

int DEF_split::parseNetsEnd(defrCallbackType_e typ, void* variable, defiUserData* userData) {

	Data* data = reinterpret_cast<Data*>(userData);

	if (data->nets.size() != data->DEF_items.nets) {

		std::cout << "DEF>   Error; only " << data->nets.size() << " nets have been parsed ..." << std::endl;
		return 1;
	}
	else {
		std::cout << "DEF>   Done" << std::endl;
		return 0;
	}
}
