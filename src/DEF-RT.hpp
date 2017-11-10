/*
 * =====================================================================================
 *
 *    Description:  DEF splitting handler
 *
 *    Copyright (C) 2017 Johann Knechtel, johann aett jknechtel dot de
 *
 * =====================================================================================
 */
#ifndef _DEF_RT
#define _DEF_RT

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
#include <unordered_set>
#include <cstdlib>

// other includes, if any
#include "Data.hpp"
// forward declarations, if any

class DEF_RT {
	private:
		/// debugging code switch (private)
		static constexpr bool DBG = true;

	// private data, functions
	private:

	// constructors, destructors, if any non-implicit
	public:

	// public data, functions
	public:
		std::string DEF_file;

		std::string nets_file;
		std::unordered_set<std::string> nets_to_consider;

		Data data;

		void parseParameters(int const& argc, char** argv);
		void splitAndStore(unsigned split_layer);
		void readNetsFile();

};

#endif
