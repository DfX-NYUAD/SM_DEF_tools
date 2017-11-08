/*
 * =====================================================================================
 *
 *    Description:  DEF parser 
 *
 *    Copyright (C) 2017 Johann Knechtel, johann aett jknechtel dot de
 *
 * =====================================================================================
 */
#ifndef _DEF_SPLIT_PARSER_DEF
#define _DEF_SPLIT_PARSER_DEF

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

// namespaces
// boost
namespace bp = boost::polygon;

// other includes, if any
// forward declarations
class Data;

class ParserDEF{
	private:
		/// debugging code switch (private)
		static constexpr bool DBG = false;
		static constexpr bool DBG_DATA = false;

	// private data, functions
	private:
		// parser callbacks
		static int parseNetsStart(defrCallbackType_e typ, int nets, defiUserData* userData);
		static int parseNetsEnd(defrCallbackType_e typ, void* variable, defiUserData* userData);
		static int parseNets(defrCallbackType_e typ, defiNet* net, defiUserData* userData);

		static int parseLayers(defrCallbackType_e typ, defiTrack* track, defiUserData* userData);

		static int parseUnits(defrCallbackType_e typ, double number, defiUserData* userData);
		static int parseDieArea(defrCallbackType_e typ, defiBox* box, defiUserData* userData);

		static int parseComponentsStart(defrCallbackType_e typ, int components, defiUserData* userData);
		static int parseComponentsEnd(defrCallbackType_e typ, void* variable, defiUserData* userData);
		static int parseComponents(defrCallbackType_e typ, defiComponent* component, defiUserData* userData);

		static int parseTerminalsStart(defrCallbackType_e typ, int terminals, defiUserData* userData);
		static int parseTerminalsEnd(defrCallbackType_e typ, void* variable, defiUserData* userData);
		static int parseTerminals(defrCallbackType_e typ, defiPin* pin, defiUserData* userData);


		// helper functions
		inline static void lowerValue(int& lower, int other) {
			if (lower == -1) {
				lower = other;
			}
			else {
				lower = std::min(lower, other);
			}
		}
		inline static void upperValue(int& upper, int other) {
			if (upper == -1) {
				upper = other;
			}
			else {
				upper = std::max(upper, other);
			}
		}

	// constructors, destructors, if any non-implicit
	public:

	// public data, functions
	public:
		static void read(std::string& DEF_file, Data& data);
};

#endif
