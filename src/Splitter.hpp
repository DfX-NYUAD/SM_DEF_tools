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
//
// STL
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdlib>

// other includes, if any
#include "Data.hpp"
// forward declarations, if any

class Splitter {
	private:
		/// debugging code switch (private)
		static constexpr bool DBG = true;

	// private data, functions
	private:

	// constructors, destructors, if any non-implicit
	public:

	// public data, functions
	public:
		std::string DEF_file, LEF_file;
		std::string split_layer;
		Data data;

		void parseParameters(int const& argc, char** argv);
};

#endif
