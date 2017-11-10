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
	std::cout << "DEF_RT:  performs also splitting" << std::endl;
	std::cout << "-------------------------------------------------------------------------" << std::endl << std::endl;

	// parse program parameters, and test for DEF/LEF files
	converter.parseParameters(argc, argv);

	// parse in nets file, if available
	converter.readNetsFile();

	// parse in DEF/LEF
	ParserDEF::read(converter.DEF_file, converter.data);

	// split and write out file for all the metal layers
	for (auto const& metal_layer : converter.data.metal_layers) {

		std::cout << std::endl;
		std::cout << "DEF_RT> Split and write out file for splitting after " <<  metal_layer.first << " ..." << std::endl;
		converter.splitAndStore(metal_layer.second);
	}
}

void DEF_RT::parseParameters(int const& argc, char** argv) {
	std::ifstream in;

	// print command-line parameters
	//
	if (argc < 2) {
		std::cout << "IO> Usage: " << argv[0] << " DEF_file " << std::endl;
		std::cout << "IO> " << std::endl;
		std::cout << "IO> Mandatory parameter ``DEF_file'': the DEF file to be converted" << std::endl;
		std::cout << "IO> Optional parameter ``nets_file'': the file containing the nets to be considered; if none given, all nets are considered" << std::endl;

		exit(1);
	}

	this->DEF_file = argv[1];
	if (argc == 3) {
		this->nets_file = argv[2];
	}

	// test files
	//
	in.open(this->DEF_file.c_str());
	if (!in.good()) {
		std::cout << "IO> ";
		std::cout << "No such file: " << this->DEF_file << std::endl;
		exit(1);
	}
	in.close();

	if (!this->nets_file.empty()) {
		in.open(this->nets_file.c_str());
		if (!in.good()) {
			std::cout << "IO> ";
			std::cout << "No such file: " << this->nets_file << std::endl;
			exit(1);
		}
		in.close();
	}

	std::cout << "IO> DEF file: " << this->DEF_file << std::endl;
	std::cout << "IO> File for nets to be considered: " << this->nets_file << std::endl;
	std::cout << std::endl;
}

void DEF_RT::readNetsFile() {
	std::ifstream in;
	std::string tmpstr;

	if (this->nets_file.empty()) {
		return;
	}

	std::cout << "IO> Reading in nets, word by word, from nets_file: " << this->nets_file << " ..." << std::endl;
	std::cout << "IO>  Note that simply all words of the file are read in as nets, also any other comments/commands etc." << std::endl;
	std::cout << "IO>  This is no issue -- and allows for flexible syntax/format of the nets_file -- as long as those other words are not coincidentally matching with some other nets which should not be considered." << std::endl;
	std::cout << "IO>" << std::endl;

	in.open(this->nets_file.c_str());

	while (!in.eof()) {
		in >> tmpstr;
		this->nets_to_consider.emplace(tmpstr);
	}
	in.close();

	std::cout << "IO>  Nets (plus other words in file) read in: " << this->nets_to_consider.size() << std::endl;
	std::cout << "IO> Done" << std::endl;
	std::cout << std::endl;
}

void DEF_RT::splitAndStore(unsigned split_layer) {
	std::ofstream rt_splitted;
	std::size_t segments;
	std::size_t net_counter;
	unsigned layer_lower, layer_upper;
	unsigned vias_split_layer;
	int counter_virtual_pins;

	// build up output file name
	std::stringstream out_name;
	// drop .def
	out_name << this->DEF_file.substr(0, this->DEF_file.size() - 4);
	out_name << ".split2VpinLvl_" << split_layer;
	out_name << ".out";

	std::cout << "IO> Writing output to " << out_name.str() << " ..." << std::endl;

	// init file stream
	rt_splitted.open(out_name.str().c_str());

	// header: grid_x grid_y grid_x tile_x tile_y
	rt_splitted << std::ceil(bp::xh(this->data.DEF_data.die_outline) / this->data.DEF_data.units_per_micron);
	rt_splitted << " " << std::ceil(bp::yh(this->data.DEF_data.die_outline) / this->data.DEF_data.units_per_micron);
	rt_splitted << " " << this->data.metal_layers.size();
	rt_splitted << " " << this->data.DEF_data.units_per_micron;
	rt_splitted << " " << this->data.DEF_data.units_per_micron;
	rt_splitted << "\n";

	// write out nets individually
	//
	net_counter = 0;
	counter_virtual_pins = 0;
	for (std::size_t i = 0; i < this->data.nets.size(); i++) {
		auto const& net = this->data.nets[i];
		vias_split_layer = 0;

		// if some nets_to_consider are given, only write out those nets
		//
		if (!this->nets_to_consider.empty()) {
			if (this->nets_to_consider.find(net.name) == this->nets_to_consider.end()) {
				continue;
			}
		}
		// as of now, the attack by Jonathon supports only nets with two dangling pins; hence, we count the vias connecting from the split layer to above first, and we skip
		// nets with different numbers of dangling pins
		for (auto const& seg : net.segments) {
			// skip wires
			if (seg.only_wire) {
				continue;
			}
			// count the vias and wires with vias connecting upward from the split layer
			if (split_layer == static_cast<unsigned>(seg.via_layer)) {
				vias_split_layer++;
			}
		}
		if (vias_split_layer != 2) {
			continue;
		}

		// syntax for heading for each net: "NAME RUNNING_ID NUMBER_SEGMENTS"
		// counting scheme follows same handling of segments as code below for outpu
		//
		segments = 0;
		for (auto const& seg : net.segments) {

			if (seg.only_wire) {

				if (seg.metal_layer <= split_layer) {
					segments++;
				}
			}
			else {
				// both wire and via; count wire statement separately
				if (!seg.only_via) {

					if (seg.metal_layer <= split_layer) {
						segments++;
					}
				}

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

				// the via connects from the split layer upwards; count as virtual pin
				if (layer_lower == split_layer) {
					segments++;
				}
				// the via is below, count it as regular via
				else if (layer_lower < split_layer) {
					segments++;
				}
			}
		}

		//rt_splitted << net.name << " " << i << " " << segments << "\n";
		// Jonathon's parser accepts only netnames like n0, n1, etc. 
		//
		rt_splitted << "n" << i << " " << net_counter << " " << segments << "\n";

		// write out each segment
		for (auto const& seg : net.segments) {

			if (seg.only_wire) {

				if (seg.metal_layer <= split_layer) {
					rt_splitted << "(" << bp::xl(seg.wire) << "," << bp::yl(seg.wire) << "," << seg.metal_layer << ")";
					rt_splitted << "-";
					rt_splitted << "(" << bp::xh(seg.wire) << "," << bp::yh(seg.wire) << "," << seg.metal_layer << ")";
					rt_splitted << "\n";
				}
			}
			else {
				// both wire and via; generate wire statement separately
				if (!seg.only_via) {

					if (seg.metal_layer <= split_layer) {
						rt_splitted << "(" << bp::xl(seg.wire) << "," << bp::yl(seg.wire) << "," << seg.metal_layer << ")";
						rt_splitted << "-";
						rt_splitted << "(" << bp::xh(seg.wire) << "," << bp::yh(seg.wire) << "," << seg.metal_layer << ")";
						rt_splitted << "\n";
					}
				}

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

				// the via connects from the split layer upwards; output as virtual pin
				if (layer_lower == split_layer) {
					rt_splitted << "(" << bp::xl(seg.via_rect) << "," << bp::yl(seg.via_rect) << "," << layer_lower << ")";
					rt_splitted << "-";
					rt_splitted << "(S" << counter_virtual_pins << ")";
					rt_splitted << "\n";

					counter_virtual_pins++;
				}
				// the via is below, keep it as regular via
				else if (layer_lower < split_layer) {
					rt_splitted << "(" << bp::xl(seg.via_rect) << "," << bp::yl(seg.via_rect) << "," << layer_lower << ")";
					rt_splitted << "-";
					rt_splitted << "(" << bp::xh(seg.via_rect) << "," << bp::yh(seg.via_rect) << "," << layer_upper << ")";
					rt_splitted << "\n";
				}
			}
		}

		// syntax for footer for each net: "!"
		rt_splitted << "!\n";

		// update net counter
		net_counter++;
	}

	// close file stream
	rt_splitted.close();

	std::cout << "IO> Done" << std::endl;
	std::cout << "IO>  Nets written out: " << net_counter << std::endl;
}
