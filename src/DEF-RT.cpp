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
	std::cout << "-------------------------------------------------------------------------" << std::endl << std::endl;

	// parse program parameters, and test for DEF/LEF files
	converter.parseParameters(argc, argv);

	// parse in DEF/LEF
	ParserDEF::read(converter.DEF_file, converter.data);

	// write out RT file
	converter.write();
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

void DEF_RT::write() {
	std::ofstream rt;
	std::size_t segments;
	unsigned layer_lower, layer_upper;

	// build up output file name
	std::stringstream out_name;
	out_name << this->DEF_file << ".rt";

	std::cout << "IO> Writing output to " << out_name.str() << " ..." << std::endl;

	// init file stream
	rt.open(out_name.str().c_str());

	// write out nets individually
	//
	for (std::size_t i = 0; i < data.nets.size(); i++) {
		auto const& net = data.nets[i];

		// syntax for heading for each net: "NAME RUNNING_ID NUMBER_SEGMENTS"
		//
		// NUMBER_SEGMENTS should count wires and vias separately, i.e., segments with both wires and vias are counted twice
		segments = net.segments.size();
		for (auto const& seg : net.segments) {
			if (!seg.only_via && !seg.only_wire) {
				segments++;
			}
		}

		//rt << net.name << " " << i << " " << segments << std::endl;
		// Jonathon's parser accepts only netnames like n0, n1, etc. 
		//
		rt << "n" << i << " " << i << " " << segments << std::endl;

		// write out each segment
		for (auto const& seg : net.segments) {

			if (seg.only_wire) {

				rt << "(" << bp::xl(seg.wire) << "," << bp::yl(seg.wire) << "," << seg.metal_layer << ")";
				rt << "-";
				rt << "(" << bp::xh(seg.wire) << "," << bp::yh(seg.wire) << "," << seg.metal_layer << ")";
				rt << std::endl;
			}
			// (only via) or (wire and via)
			else {

				// as there's a via, we should differentiate between the lower and upper layer
				//
				if (static_cast<unsigned>(seg.via_layer) == seg.metal_layer) {
					layer_lower = seg.metal_layer;
					layer_upper = seg.metal_layer + 1;
				}
				else {
					layer_lower = std::min(static_cast<unsigned>(seg.via_layer), seg.metal_layer);
					layer_upper = std::max(static_cast<unsigned>(seg.via_layer), seg.metal_layer);
				}

				// only via
				if (seg.only_via) {
					rt << "(" << bp::xl(seg.via_rect) << "," << bp::yl(seg.via_rect) << "," << layer_lower << ")";
					rt << "-";
					rt << "(" << bp::xh(seg.via_rect) << "," << bp::yh(seg.via_rect) << "," << layer_upper << ")";
					rt << std::endl;
				}
				// both wire and via; decompose those segments into two lines
				else {
					// wire
					rt << "(" << bp::xl(seg.wire) << "," << bp::yl(seg.wire) << "," << seg.metal_layer << ")";
					rt << "-";
					rt << "(" << bp::xh(seg.wire) << "," << bp::yh(seg.wire) << "," << seg.metal_layer << ")";
					rt << std::endl;

					// via
					rt << "(" << bp::xl(seg.via_rect) << "," << bp::yl(seg.via_rect) << "," << layer_lower << ")";
					rt << "-";
					rt << "(" << bp::xh(seg.via_rect) << "," << bp::yh(seg.via_rect) << "," << layer_upper << ")";
					rt << std::endl;
				}
			}
		}

		// syntax for footer for each net: "!"
		rt << "!" << std::endl;
	}

	// close file stream
	rt.close();

	std::cout << "IO> Done" << std::endl;
}
