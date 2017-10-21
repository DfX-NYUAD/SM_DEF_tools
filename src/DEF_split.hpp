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
// LEF/DEF
//
// ignore warnings related to LEF/DEF code
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wswitch"
#include "defrReader.hpp"
#include "lefrReader.hpp"
#include "defwWriter.hpp"

// other includes, if any
#include "Data.hpp"
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
		static constexpr bool DBG = true;

	// private data, functions
	private:
		std::string DEF_file, LEF_file;
		std::string split_layer;
		Data data;

		// parser callbacks
		static int parseNetsStart(defrCallbackType_e typ, int nets, defiUserData* userData);
		static int parseNetsEnd(defrCallbackType_e typ, void* variable, defiUserData* userData);
		static int parseNets(defrCallbackType_e typ, defiNet* net, defiUserData* userData);

	// constructors, destructors, if any non-implicit
	public:

	// public data, functions
	public:
		void parseParameters(int const& argc, char** argv);
		void parseDEF();
		void parseLEF();
};

#endif
