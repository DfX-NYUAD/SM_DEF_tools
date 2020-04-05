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
#include "Splitter.hpp"

// other includes
#include "ParserDEF.hpp"
//#include "ParserLEF.hpp"
#define COMPILE_C
#include "MIToolbox/MutualInformation.h"
#include "MIToolbox/RenyiMutualInformation.h"

int main (int argc, char** argv) {
	Splitter splitter;

	std::cout << std::endl;
	std::cout << "Splitter: splitting a DEF file into FEOL/BEOL, beyond a given metal layer" << std::endl;
	std::cout << "-------------------------------------------------------------------------" << std::endl << std::endl;

	// parse program parameters, and test for DEF/LEF files
	splitter.parseParameters(argc, argv);

	// parse in LEF/DEF
	// (TODO -- only required for handling cell pin coordinates, using pin offsets from LEF, as DEF only describes LL corners of cell, AFAIK)
	//ParserLEF::read();
	ParserDEF::read(splitter.DEF_file, splitter.data);

	// obtain split layer as numeric value; possible only after parsing all metal layers present in the DEF 
	// also perform sanity check whether provided split layer can be handled at all
	if (splitter.data.metal_layers.find(splitter.split_layer_) != splitter.data.metal_layers.end()) {
		splitter.split_layer = splitter.data.metal_layers.at(splitter.split_layer_);
	}
	else {
		std::cout << "IO> Error: provided split layer \"" << splitter.split_layer_ << "\" not found in DEF -- check for typo in the split layer, and then check the DEF itself." << std::endl;
		std::cout << "IO> Exiting ..." << std::endl;
		exit(1);
	}

	std::cout << std::endl;
	std::cout << "MI> Derive connectivity and distance vectors for all open pin pairs across all nets ..." << std::endl;

	// first, extract all the relevant open pins for each net and store them in the nets
	for (auto& n : splitter.data.nets) {

		if (Splitter::DBG) {
			std::cout << "Splitter_DBG>    Extracting open pin from net: " << n.name << std::endl;
		}

		// find relevant vias, from segments
		//
		for (auto const& s : n.segments) {

			if (s.via_layer == static_cast<int>(splitter.split_layer)) {

				n.open_pins.emplace_back( std::make_pair(bp::xl(s.via_rect), bp::yl(s.via_rect)) );

				// dbg log for original segment
				if (Splitter::DBG) {

					std::cout << "Splitter_DBG>     Relevant segment: layer = " << s.metal_layer_ << " (" << s.metal_layer << ")";

					if (!s.only_via) {
						std::cout << "; wire = (" << bp::xl(s.wire) << ", " << bp::yl(s.wire);
						std::cout << "; " << bp::xh(s.wire) << ", " << bp::yh(s.wire) << ")";
					}

					if (!s.only_wire) {
						std::cout << "; via = " << s.via << " (" << s.via_layer << ")";
						std::cout << "; via (X, Y) = (" << bp::xl(s.via_rect) << ", " << bp::yl(s.via_rect) << ")";
					}
					std::cout << std::endl;
				}
			}
		}

		// if there's only one via, that means the open segment has to connect to a terminal
		if (n.open_pins.size() == 1) {

			for (auto const* t : n.terminals) {

				// sanity check that the terminal is above the split layer
				if (t->metal_layer > splitter.split_layer) {

					n.open_pins.emplace_back( std::make_pair(t->x, t->y) );

					// dbg log
					if (Splitter::DBG) {

						for (auto const* t : n.terminals) {
							std::cout << "Splitter_DBG>     Relevant terminal: layer = " << t->metal_layer_ << " (" << t->metal_layer << ")";
							std::cout << "; X = " << t->x << ", Y = " << t->y;
							std::cout << std::endl;
						}
					}
				}
			}
		}
	}

	// second, derive Manhattan distances for all possible open pins across all nets
	//
	for (auto const& n1 : splitter.data.nets) {
		for (auto const& n2 : splitter.data.nets) {

			bool same_net = (n1.name == n2.name);

			if (Splitter::DBG) {
				std::cout << "Splitter_DBG>    Handling net pair: " << n1.name << ", " << n2.name << std::endl;
				if (same_net) {
					std::cout << "Splitter_DBG>     Same net, all open pins are connected by definition" << std::endl;
				}
				else {
					std::cout << "Splitter_DBG>     Different net, all open pins are dis-connected by definition" << std::endl;
				}
			}

			// same net, all open pins are connected
			if (same_net) {

				for (auto const& p1 : n1.open_pins) {
					for (auto const& p2 : n2.open_pins) {

						// sanity check for same net, same point; ignore those
						if ((p1.first == p2.first) && (p1.second == p2.second)) {
							continue;
						}

						splitter.data.connectivity.emplace_back(1);
						// Manhattan distance
						splitter.data.distances.emplace_back(
								// for std::abs, cast to int temporarily, but store all data only in unsigned to
								// limit meory usages
								std::abs(static_cast<int>(p1.first - p2.first))
								+ std::abs(static_cast<int>(p1.first - p2.first))
							);
					}
				}
			}
			// different nets, all open are disconnected
			else {
				for (auto const& p1 : n1.open_pins) {
					for (auto const& p2 : n2.open_pins) {

						splitter.data.connectivity.emplace_back(0);
						splitter.data.distances.emplace_back(
								std::abs(static_cast<int>(p1.first - p2.first))
								+ std::abs(static_cast<int>(p1.first - p2.first))
							);
					}
				}
			}
		}
	}

	std::string numWithCommas = std::to_string(splitter.data.connectivity.size());
	int insertPosition = static_cast<int>(numWithCommas.length() - 3);
	while (insertPosition > 0) {
		    numWithCommas.insert(static_cast<size_t>(insertPosition), ",");
		        insertPosition-=3;
	}
	std::cout << "MI> Done; " << numWithCommas << " data points stored for both the vectors" << std::endl;

	std::cout << std::endl;
	std::cout << "MI> For distance vector D and connectivity vector C, calculate the mutual information I(D;C) and the Renyi mutual information I_{\\alpha}(D;C) ..." << std::endl;

	std::cout << "MI>  I(D;C) = " << calcMutualInformation(splitter.data.distances.data(), splitter.data.connectivity.data(), static_cast<int>(splitter.data.connectivity.size())) << std::endl;
	std::cout << "MI>  I_0 = " << calcRenyiMIDivergence(0, splitter.data.distances.data(), splitter.data.connectivity.data(), static_cast<int>(splitter.data.connectivity.size())) << std::endl;
	std::cout << "MI>  I_0.5 = " << calcRenyiMIDivergence(0.5, splitter.data.distances.data(), splitter.data.connectivity.data(), static_cast<int>(splitter.data.connectivity.size())) << std::endl;
	// defined as regul MI, using KL divergence
	//std::cout << "MI>  I_1 = " << calcRenyiMIDivergence(1, splitter.data.distances.data(), splitter.data.connectivity.data(), static_cast<int>(splitter.data.connectivity.size())) << std::endl;
	std::cout << "MI>  I_2 = " << calcRenyiMIDivergence(2, splitter.data.distances.data(), splitter.data.connectivity.data(), static_cast<int>(splitter.data.connectivity.size())) << std::endl;
}

void Splitter::parseParameters(int const& argc, char** argv) {
	std::ifstream in;

	// print command-line parameters
	//
	if (argc < 4) {
		std::cout << "IO> Usage: " << argv[0] << " DEF_file LEF_file split_layer" << std::endl;
		std::cout << "IO> " << std::endl;
		std::cout << "IO> Mandatory parameter ``DEF_file'': the DEF file to be split" << std::endl;
		std::cout << "IO> Mandatory parameter ``LEF_file'': the LEF file related to the DEF file" << std::endl;
		std::cout << "IO> Mandatory parameter ``split_layer'': the layer after which to split the DEF file -- provide string, e.g., metal2" << std::endl;

		std::cout << "IO> Exiting ..." << std::endl;
		exit(1);
	}

	this->DEF_file = argv[1];
	this->LEF_file = argv[2];
	this->split_layer_ = argv[3];

	// test files
	//
	in.open(this->DEF_file.c_str());
	if (!in.good()) {
		std::cout << "IO> ";
		std::cout << "No such file: " << this->DEF_file << std::endl;
		std::cout << "IO> Exiting ..." << std::endl;
		exit(1);
	}
	in.close();

	in.open(LEF_file.c_str());
	if (!in.good()) {
		std::cout << "IO> ";
		std::cout << "No such file: " << LEF_file << std::endl;
		std::cout << "IO> Exiting ..." << std::endl;
		exit(1);
	}
	in.close();

	std::cout << "IO> DEF file: " << this->DEF_file << std::endl;
	std::cout << "IO> LEF file: " << this->LEF_file << std::endl;
	std::cout << "IO> Metal layer to split after: " << this->split_layer_ << std::endl;
	std::cout << std::endl;
}
