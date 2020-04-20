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
			std::cout << "Splitter_DBG>    Extracting open pins from net: " << n.name << std::endl;
		}

		// find relevant vias, from segments
		//
		for (auto const& s : n.segments) {

			if (s.via_layer == static_cast<int>(splitter.split_layer)) {

				n.open_pins.emplace_back( bp::xl(s.via_rect), bp::yl(s.via_rect) );

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

					n.open_pins.emplace_back( t->x, t->y );

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
	for (auto n1_ptr = splitter.data.nets.begin(); n1_ptr < splitter.data.nets.end(); n1_ptr++) {

		if (n1_ptr->open_pins.empty()) {
			continue;
		}

		// first, handle the special case for consideration of the same net
		//
		auto n2_ptr = n1_ptr;

		if (Splitter::DBG) {
			std::cout << "Splitter_DBG>    Handling relevant net pair: " << n1_ptr->name << ", " << n2_ptr->name << std::endl;
			std::cout << "Splitter_DBG>     Same net, all open pins are connected by definition" << std::endl;
		}

		// handle pin pairs of same net
		//
		// avoid considering pairs twice; inner loop starts with the very next pin as the outer loop, also to avoid pairing pins with
		// themselves
		//
		// only used for dbg logging
		unsigned p1_index = 0;
		unsigned p2_index = 0;

		for (auto p1_ptr = n1_ptr->open_pins.begin(); p1_ptr < n1_ptr->open_pins.end(); p1_ptr++) {

			if (Splitter::DBG) {
				p2_index = p1_index + 1;
			}

			for (auto p2_ptr = (p1_ptr + 1); p2_ptr < n2_ptr->open_pins.end(); p2_ptr++) {

				if (Splitter::DBG) {
					std::cout << "Splitter_DBG>      Handling pin pair: " << p1_index << ", " << p2_index << std::endl;
				}

				// pin pairs are connected by definition
				splitter.data.connectivity.emplace_back(1);

				// Manhattan distance
				splitter.data.distances.emplace_back(
						// for std::abs, cast to int temporarily, but store all data only in unsigned to
						// limit meory usages
						std::abs(static_cast<int>(p1_ptr->x - p2_ptr->x))
						+ std::abs(static_cast<int>(p1_ptr->y - p2_ptr->y))
					);

				if (Splitter::DBG) {
					std::cout << "Splitter_DBG>       Manhattan distance: " << splitter.data.distances.back() << std::endl;
				}

				if (Splitter::DBG) {
					p2_index++;
				}
			}

			if (Splitter::DBG) {
				p1_index++;
			}
		}

		// handle all other net pairings (with dis-connected pins)
		//
		// avoid considering pairs twice; inner loop starts with the very next net as the outer loop, not with the very first net again,
		// and also not with the same net (handled already above)
		//
		for (auto n2_ptr =  (n1_ptr + 1); n2_ptr < splitter.data.nets.end(); n2_ptr++) {

			if (n2_ptr->open_pins.empty()) {
				continue;
			}

			if (Splitter::DBG) {
				std::cout << "Splitter_DBG>    Handling relevant net pair: " << n1_ptr->name << ", " << n2_ptr->name << std::endl;
				std::cout << "Splitter_DBG>     Different net, all open pins are dis-connected by definition" << std::endl;
			}

			// for pairings across different nets, we want to consider all possible pairs
			//
			// only used for dbg logging
			unsigned p1_index = 0;
			unsigned p2_index = 0;

			for (auto const& p1 : n1_ptr->open_pins) {

				if (Splitter::DBG) {
					p2_index = 0;
				}

				for (auto const& p2 : n2_ptr->open_pins) {

					if (Splitter::DBG) {
						std::cout << "Splitter_DBG>      Handling pin pair: " << p1_index << ", " << p2_index << std::endl;
					}

					// pin pairs are dis-connected by definition
					splitter.data.connectivity.emplace_back(0);

					// Manhattan distance
					splitter.data.distances.emplace_back(
							// for std::abs, cast to int temporarily, but store all data only in unsigned to
							// limit meory usages
							std::abs(static_cast<int>(p1.x - p2.x))
							+ std::abs(static_cast<int>(p1.y - p2.y))
						);

					if (Splitter::DBG) {
						std::cout << "Splitter_DBG>       Manhattan distance: " << splitter.data.distances.back() << std::endl;
					}

					if (Splitter::DBG) {
						p2_index++;
					}
				}

				if (Splitter::DBG) {
					p1_index++;
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
