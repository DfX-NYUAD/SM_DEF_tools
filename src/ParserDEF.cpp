/*
 * =====================================================================================
 *
 *    Description:  DEF parser 
 *
 *    Copyright (C) 2017 Johann Knechtel, johann aett jknechtel dot de
 *
 * =====================================================================================
 */

// own header
#include "ParserDEF.hpp"

// other includes
#include "Data.hpp"

void ParserDEF::read(std::string& DEF_file, Data& data) {
	FILE *DEF;

	std::cout << "DEF> Start parsing DEF file ..." << std::endl;

	// pointer to user data; made available in all parser callbacks
	Data* userData = &data;

	DEF = fopen(DEF_file.c_str(), "r");

	defrInit();
	defrSetUserData(userData);

	//TODO for new Si2 parser
	//defrInitSession(1);

	// define callback functions
	//
	// TRACKS section; currently parses only names and order of metal layers
	defrSetTrackCbk((defrTrackCbkFnType) ParserDEF::parseLayers);

	// trigger parser; read only TRACKS section (metal layers) now
	// the resulting data is leveraged below
	//
	// 4th parameter: 1 -- specifies that the data is case sensitive
	int status = defrRead(DEF, DEF_file.c_str(), userData, 1);
	if (status != 0) {
		std::cout << "DEF> Error in parser; abort" << std::endl;
		exit (1);
	}
	defrClear();
	fclose(DEF);

	// post-processing of metal layers which have been read in from metal10, metal9, ..., to metal1
	// map to respective indices here, i.e., metal9 has index 9
	//
	for (std::size_t i = 0; i < data.metal_layers_.size(); i++) {
		data.metal_layers.emplace( std::make_pair(
					// key: the string of the metal layer
					data.metal_layers_[i],
					// value: the int id, to be calculated from reversed order of metal_layers_, and also considering that each metal layers is parsed twice
					(data.metal_layers_.size() - i) / 2)
				);
	}

	std::cout << "DEF>  Metal layers (view not sorted):" << std::endl;
	for (auto const& layer : data.metal_layers) {
		std::cout << "DEF>   " << layer.first << "; index = " << layer.second << std::endl;
	}

	// 2nd parsing session
	//
	DEF = fopen(DEF_file.c_str(), "r");

	defrInit();
	defrSetUserData(userData);

	// define callback functions
	//

	// regular nets
	defrSetNetStartCbk((defrIntegerCbkFnType) ParserDEF::parseNetsStart);
	defrSetNetEndCbk((defrVoidCbkFnType) ParserDEF::parseNetsEnd);
	defrSetNetCbk((defrNetCbkFnType) ParserDEF::parseNets);
	// augment nets with path data
	defrSetAddPathToNet();
	
	// not required for the RT conversion; there only the wires and vias are relevant
	//
	//// components
	//defrSetComponentStartCbk((defrIntegerCbkFnType) ParserDEF::parseComponentsStart);
	//defrSetComponentCbk((defrComponentCbkFnType) ParserDEF::parseComponents);
	//defrSetComponentEndCbk((defrVoidCbkFnType) ParserDEF::parseComponentsEnd);
	//
	//// terminals
	//defrSetStartPinsCbk((defrIntegerCbkFnType) ParserDEF::parseTerminalsStart);
	//defrSetPinCbk((defrPinCbkFnType) ParserDEF::parseTerminals);
	//defrSetPinEndCbk((defrVoidCbkFnType) ParserDEF::parseTerminalsEnd);

	// trigger parser; read DEF sections of interest
	//
	// 4th parameter: 1 -- specifies that the data is case sensitive
	status = defrRead(DEF, DEF_file.c_str(), userData, 1);
	if (status != 0) {
		std::cout << "DEF> Error in parser; abort" << std::endl;
		exit (1);
	}
	defrClear();
	fclose(DEF);

	std::cout << "DEF> End parsing DEF file" << std::endl;
}

int ParserDEF::parseNetsStart(defrCallbackType_e typ, int nets, defiUserData* userData) {

	std::cout << "DEF>  Parsing NETS ..." << std::endl;

	Data* data = reinterpret_cast<Data*>(userData);

	data->DEF_items.nets = static_cast<std::size_t>(nets);

	std::cout << "DEF>   " << nets << " nets to be parsed ..." << std::endl;

	return 0;
}

int ParserDEF::parseComponentsStart(defrCallbackType_e typ, int components, defiUserData* userData) {

	std::cout << "DEF>  Parsing COMPONENTS ..." << std::endl;

	Data* data = reinterpret_cast<Data*>(userData);

	data->DEF_items.components = static_cast<std::size_t>(components);

	std::cout << "DEF>   " << components << " components to be parsed ..." << std::endl;

	return 0;
}

int ParserDEF::parseTerminalsStart(defrCallbackType_e typ, int terminals, defiUserData* userData) {

	std::cout << "DEF>  Parsing TERMINALS ..." << std::endl;

	Data* data = reinterpret_cast<Data*>(userData);

	data->DEF_items.terminals = static_cast<std::size_t>(terminals);

	std::cout << "DEF>   " << terminals << " terminals to be parsed ..." << std::endl;

	return 0;
}

int ParserDEF::parseNetsEnd(defrCallbackType_e typ, void* variable, defiUserData* userData) {

	Data* data = reinterpret_cast<Data*>(userData);

	if (data->nets.size() != data->DEF_items.nets) {

		std::cout << "DEF>   Error; only " << data->nets.size() << " nets have been parsed ..." << std::endl;
		return 1;
	}
	else {
		std::cout << "DEF>   Done" << std::endl;

		if (ParserDEF::DBG_DATA) {

			for (auto const& n : data->nets) {
				std::cout << "DEF>    Net: " << n.name << std::endl;

				for (auto const& s : n.segments) {
					std::cout << "DEF>     Segment: layer = " << s.metal_layer_ << " (" << s.metal_layer << ")";
					std::cout << "; wire = (" << bp::xl(s.wire) << ", " << bp::yl(s.wire);
					std::cout << "; " << bp::xh(s.wire) << ", " << bp::yh(s.wire) << ")";
					std::cout << "; via = " << s.via << " (" << s.via_layer << ")";
					std::cout << "; via (X, Y) = (" << bp::xl(s.via_rect) << ", " << bp::yl(s.via_rect) << ")";
					std::cout << std::endl;
				}

				for (auto const* t : n.terminals) {
					std::cout << "DEF>     Terminal: layer = " << t->metal_layer_ << " (" << t->metal_layer << ")";
					std::cout << "; X = " << t->x << ", Y = " << t->y;
					std::cout << std::endl;
				}

				for (auto const* c : n.components) {
					std::cout << "DEF>     Component: name = " << c->name;
					std::cout << "; X = " << c->x << ", Y = " << c->y;
					std::cout << std::endl;
				}
			}
		}

		return 0;
	}
}

int ParserDEF::parseTerminalsEnd(defrCallbackType_e typ, void* variable, defiUserData* userData) {

	Data* data = reinterpret_cast<Data*>(userData);

	if (data->terminals.size() != data->DEF_items.terminals) {

		std::cout << "DEF>   Error; only " << data->terminals.size() << " terminals have been parsed ..." << std::endl;
		return 1;
	}
	else {
		std::cout << "DEF>   Done" << std::endl;

		if (ParserDEF::DBG_DATA) {

			for (auto const& t : data->terminals) {
				auto const& term = t.second;

				std::cout << "DEF>    Terminal: " << term.name << std::endl;
				std::cout << "DEF>     X = " << term.x << "; Y = " << term.y << std::endl;
				std::cout << "DEF>     Metal layer = " << term.metal_layer_ << "(" << term.metal_layer << ")" << std::endl;
				std::cout << "DEF>     Orientation = " << term.orientation << std::endl;
			}
		}

		return 0;
	}
}

int ParserDEF::parseComponentsEnd(defrCallbackType_e typ, void* variable, defiUserData* userData) {

	Data* data = reinterpret_cast<Data*>(userData);

	if (data->components.size() != data->DEF_items.components) {

		std::cout << "DEF>   Error; only " << data->components.size() << " components have been parsed ..." << std::endl;
		return 1;
	}
	else {
		std::cout << "DEF>   Done" << std::endl;

		if (ParserDEF::DBG_DATA) {

			for (auto const& comp : data->components) {
				auto const& c = comp.second;

				std::cout << "DEF>    Component: " << c.name << std::endl;
				std::cout << "DEF>     X = " << c.x << "; Y = " << c.y << std::endl;
				std::cout << "DEF>     Orientation = " << c.orientation << std::endl;
				std::cout << "DEF>     Macro = " << c.macro->name << std::endl;
			}
		}

		return 0;
	}
}

int ParserDEF::parseComponents(defrCallbackType_e typ, defiComponent* component, defiUserData* userData) {
	Data::Component new_component;

	Data* data = reinterpret_cast<Data*>(userData);

	new_component.name = component->id();

	if (ParserDEF::DBG) {
		std::cout << "DEF>    Parsing component " << new_component.name << std::endl;
	}

	// link the underlying macro to the component
	new_component.macro = &(data->macros[component->name()]);

	// parse coordinate and orientation
	new_component.x = component->placementX();
	new_component.y = component->placementY();
	new_component.orientation = component->placementOrientStr();

	// TODO instantiate pins from macro with actual coordinates of component

	data->components.emplace( std::make_pair(
				new_component.name,
				new_component)
			);

	return 0;
}

int ParserDEF::parseTerminals(defrCallbackType_e typ, defiPin* pin, defiUserData* userData) {
	Data::Pin new_terminal;

	Data* data = reinterpret_cast<Data*>(userData);

	new_terminal.name = pin->pinName();

	if (ParserDEF::DBG) {
		std::cout << "DEF>    Parsing terminal " << new_terminal.name << std::endl;
	}

	// parse coordinate and orientation
	new_terminal.x = pin->placementX();
	new_terminal.y = pin->placementY();
	new_terminal.orientation = pin->orientStr();

	// parse metal layer and memorize also related index
	new_terminal.metal_layer_ = pin->layer(0);
	new_terminal.metal_layer = data->metal_layers[pin->layer(0)];

	data->terminals.emplace( std::make_pair(
				new_terminal.name,
				new_terminal)
			);

	return 0;
}

int ParserDEF::parseNets(defrCallbackType_e typ, defiNet* net, defiUserData* userData) {
	Data::Net new_net;
	defiPath* p;
	defiWire* wire;
	int x, y, z;
	int path;

	Data* data = reinterpret_cast<Data*>(userData);

	new_net.name = net->name();

	if (ParserDEF::DBG) {
		std::cout << "DEF>    Parsing net " << new_net.name << std::endl;
	}

	// parsing of wires
	//
	for (int i = 0; i < net->numWires(); i++) {

		wire = net->wire(i);

		if (ParserDEF::DBG) {
			std::cout << "DEF>     wire(" << i << ")" << std::endl;
			std::cout << "DEF>     wire(" << i << ")->wireType(): " << wire->wireType() << std::endl;
		}

		for (int j = 0; j < wire->numPaths(); j++) {

			Data::Segment new_segment;
			// temporary variables for wire coordinates
			int x_lower, x_upper, y_lower, y_upper;
			x_lower = x_upper = y_lower = y_upper = -1;
			// temporary variables for coordinates of last/previous element
			int last_coord_x, last_coord_y;
			last_coord_x = last_coord_y = -1;

			// init parsing of path
			p = wire->path(j);
			p->initTraverse();

			if (ParserDEF::DBG) {
				std::cout << "DEF>     wire(" << i << ")->path(" << j << "):	";
			}

			// parse all elements in path
			while ((path = static_cast<int>(p->next())) != DEFIPATH_DONE) {

				switch (path) {
					case DEFIPATH_LAYER:

						new_segment.metal_layer_ = p->getLayer();
						new_segment.metal_layer = data->metal_layers[p->getLayer()];

						if (ParserDEF::DBG) {
							printf("LAYER %s ", p->getLayer());
						}

						break;

					case DEFIPATH_VIA:

						new_segment.via = p->getVia();
						// the expected syntax for the via is, e.g., "via3_1" for a via in metal3
						new_segment.via_layer = std::stoi(new_segment.via.substr(3,1));

						// since via statements/paths refer to the previous coordinates
						// (http://edi.truevue.org/edi/14.17/lefdefref/DEFSyntax.html#RegularWiringStatement), we leverage this from the temporary variables
						// used to track the previous coordinates
						//
						new_segment.via_rect = bp_rect(last_coord_x, last_coord_y, last_coord_x, last_coord_y);

						if (ParserDEF::DBG) {
							printf("VIA %s ", p->getVia());
						}
						break;

					case DEFIPATH_WIDTH:

						if (ParserDEF::DBG) {
							printf("WIDTH %d ", p->getWidth());
						}
						break;

					case DEFIPATH_POINT:
						p->getPoint(&x, &y);

						// track lower/upper coordinates of wires
						ParserDEF::lowerValue(x_lower, x);
						ParserDEF::lowerValue(y_lower, y);
						ParserDEF::upperValue(x_upper, x);
						ParserDEF::upperValue(y_upper, y);
						// also track current coordinates, to be leveraged for via coordinates
						last_coord_x = x;
						last_coord_y = y;

						if (ParserDEF::DBG) {
							printf("POINT ( %d %d ) ", x, y);
						}
						break;

					// TODO what's a flushpoint?
					case DEFIPATH_FLUSHPOINT:
						p->defiPath::getFlushPoint(&x, &y, &z);

						// track lower/upper coordinates of wires
						ParserDEF::lowerValue(x_lower, x);
						ParserDEF::lowerValue(y_lower, y);
						ParserDEF::upperValue(x_upper, x);
						ParserDEF::upperValue(y_upper, y);
						// also track current coordinates, to be leveraged for via coordinates
						last_coord_x = x;
						last_coord_y = y;

						if (ParserDEF::DBG) {
							printf("FLUSHPOINT ( %d %d %d ) ", x, y, z);
						}
						break;

					// other items in paths are ignored
					default:

						if (ParserDEF::DBG) {
							printf("ITEM_IGNORED ");
						}
						break;
				}
			}

			// init boost rect for wire segment
			new_segment.wire = bp_rect(x_lower, y_lower, x_upper, y_upper);

			// all elements of this segment are parsed; store it
			//
			new_net.segments.push_back(new_segment);

			if (ParserDEF::DBG) {
				printf("\n");
			}
		}
	}

	// not required for RT conversion
	//
	//// parsing of pins
	////
	//for (int i = 0; i < net->numConnections(); i++) {

	//	std::string instance = net->instance(i);
	//	std::string pin = net->pin(i);

	//	if (ParserDEF::DBG) {
	//		std::cout << "DEF>     Pin: " << instance << " " << pin << std::endl;
	//	}

	//	// terminals
	//	//
	//	if (instance == "PIN") {
	//		// simple mapping to terminals data structure
	//		new_net.terminals.push_back(&(data->terminals[pin]));
	//	}
	//	// component pins
	//	else {
	//		// keep both the pointers to the actual pin as well as to the component
	//		new_net.pins_components.push_back(&(data->components[instance].pins[pin]));
	//		new_net.components.push_back(&(data->components[instance]));
	//	}
	//}

	data->nets.push_back(new_net);

	return 0;
}

int ParserDEF::parseLayers(defrCallbackType_e typ, defiTrack* track, defiUserData* userData) {

	Data* data = reinterpret_cast<Data*>(userData);

	if (ParserDEF::DBG) {
		std::cout << "DEF>  Parsing TRACK" << std::endl;
		std::cout << "DEF>   Metal layer: " << track->layer(0) << std::endl;
	}

	// not needed, since tracks are parsed individually; there's only one layer for each call
	//
	//for (int i = 0; i < track->numLayers(); i++) {

	//	data->metal_layers_.push_back(track->layer(i));

	//	if (ParserDEF::DBG) {
	//		std::cout << "DEF>   Metal layer: " << track->layer(i) << std::endl;
	//	}
	//}

	data->metal_layers_.push_back(track->layer(0));

	if (ParserDEF::DBG) {
		std::cout << "DEF>  Done" << std::endl;
	}

	return 0;
}
