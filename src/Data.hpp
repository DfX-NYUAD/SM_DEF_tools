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
			std::string via;
			int via_layer = -1;
			bp_rect wire;

		};

		struct Net {
			std::string name;
			std::vector<Pin*> pins_components;
			std::vector<Pin*> pins_terminals;
			std::vector<Segment> segments;
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

		// other data, such as control flags
		//
		// the number of items according to the declarations in the DEF
		struct DEF_Items {
			unsigned nets;
			unsigned components;
			unsigned terminals;
		} DEF_items;
};

#endif
