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

		struct Wire {
			std::string metal_layer;
		};

		struct Net {
			std::string name;
			std::vector<Component*> components;
			std::vector<Wire> wires;
		};

		struct Pin {
		};

		// actual data container
		//
		std::vector<Component> components;
		std::vector<Pin> terminals;
		std::vector<Net> nets;

		// other data, such as control flags
		//
		// the number of items according to the declarations in the DEF
		struct DEF_Items {
			unsigned nets;
		} DEF_items;
};

#endif
