/*
 * =====================================================================================
 *
 *    Description:  Data handler 
 *
 *    Copyright (C) 2017 Johann Knechtel, johann aett jknechtel dot de
 *
 * =====================================================================================
 */
#ifndef _DEF_SPLIT_DATA
#define _DEF_SPLIT_DATA

// library includes
// boost
#include <boost/polygon/polygon.hpp>
// STL
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdlib>

// namespaces
// boost
// TODO update for connectivity extraction
//
namespace bp = boost::polygon;
//using namespace bp::operators;
typedef bp::rectangle_data<int> bp_rect;

class Data {
	private:
		/// debugging code switch (private)
		static constexpr bool DBG = false;

	// private data, functions
	private:

	// constructors, destructors, if any non-implicit
	public:

	// public data, functions
	public:
		// PODs for data
		//
		struct Pin {
			std::string name;
			int x, y;
			std::string orientation;
			std::string metal_layer_;
			unsigned metal_layer;
		};

		struct Macro {
			std::string name;
			std::unordered_map<std::string, Pin> pin_offsets;
		};

		struct Component {
			std::string name;
			Macro* macro;
			int x, y;
			std::string orientation;

			// this data structure should hold the actual pin instances, i.e., with their respective coordinates in the layout
			std::unordered_map<std::string, Pin> pins;
		};

		struct Segment {
			std::string metal_layer_;
			unsigned metal_layer;
			bp_rect wire;
			std::string via;
			int via_layer = -1;
			bool only_via = false;
			bool only_wire = true;
			bp_rect via_rect;

		};

		struct Net {
			std::string name;
			std::vector<Pin*> terminals;
			std::vector<Segment> segments;
			std::vector<Component*> components;
			std::vector<Pin*> pins_components;
		};

		// actual data container
		//
		std::unordered_map<std::string, Macro> macros;
		std::unordered_map<std::string, Component> components;
		std::unordered_map<std::string, Pin> terminals;
		std::vector<Net> nets;
		// container to hold data temporarily during parsing
		std::vector<std::string> metal_layers_;
		// container for actual access to data; std::set to simplify access to unsigned index via string name
		std::unordered_map<std::string, unsigned> metal_layers;

		// other DEF data
		//
		struct DEF_DATA {
			// the number of items according to the declarations in the DEF
			std::size_t nets;
			std::size_t components;
			std::size_t terminals;

			// the units per micron
			double units_per_micron;
			// the die outline
			bp_rect die_outline;
		} DEF_data;

		// split layer
		//
		std::string split_layer_;
		unsigned split_layer = 0;
};

#endif
