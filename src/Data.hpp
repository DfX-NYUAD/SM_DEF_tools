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
		struct Component {
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
			std::vector<Component*> components;
			std::vector<Segment> segments;
		};

		struct Pin {
		};

		// actual data container
		//
		std::vector<Component> components;
		std::vector<Pin> terminals;
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
		} DEF_items;
};

#endif
