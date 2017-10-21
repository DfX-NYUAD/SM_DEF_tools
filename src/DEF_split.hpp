/*
 * =====================================================================================
 *
 *    Description:  DEF splitting handler
 *
 *    Copyright (C) 2017 Johann Knechtel, johann aett jknechtel dot de
 *
 * =====================================================================================
 */
#ifndef _DEF_SPLIT
#define _DEF_SPLIT

// library includes
#include <boost/polygon/polygon.hpp>
// STL
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <vector>
#include <cstdlib>

// other includes, if any
// forward declarations, if any

// boost namespaces
// TODO update for connectivity extraction
//
namespace bp = boost::polygon;
using namespace bp::operators;
typedef bp::rectangle_data<double> BoostRect;
typedef bp::polygon_90_set_data<double> BoostPolygonSet;

class DEF_split {
	private:
		/// debugging code switch (private)
		static constexpr bool DBG = false;

	// private data, functions
	private:
		std::string DEF_file, LEF_file;
		int split_layer;

	// constructors, destructors, if any non-implicit
	public:

	// public data, functions
	public:
		void parseParametersFiles(int const& argc, char** argv);
};

#endif
