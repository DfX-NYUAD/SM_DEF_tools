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
	for (unsigned i = 0; i < data.metal_layers_.size(); i++) {
		data.metal_layers.emplace( std::make_pair(
					// key: the string of the metal layer
					data.metal_layers_[i],
					// value: the int id, to be calculated from reversed order of metal_layers_, and also considering that each metal layers is parsed twice
					(data.metal_layers_.size() - i) / 2)
				);
	}

	if (ParserDEF::DBG_DATA) {

		std::cout << "DEF>  Metal layers (view not sorted):" << std::endl;

		for (auto const& layer : data.metal_layers) {
			std::cout << "DEF>   " << layer.first << "; index = " << layer.second << std::endl;
		}
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

	data->DEF_items.nets = static_cast<unsigned>(nets);

	std::cout << "DEF>   " << nets << " nets to be parsed ..." << std::endl;

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

			for (Data::Net& n : data->nets) {
				std::cout << "DEF>    Net: " << n.name << std::endl;

				for (Data::Segment& s : n.segments) {
					std::cout << "DEF>     Segment: layer = " << s.metal_layer;
					std::cout << "(" << data->metal_layers[s.metal_layer] << ")";
					std::cout << "; wire = (" << bp::xl(s.wire) << ", " << bp::yl(s.wire);
					std::cout << "; " << bp::xh(s.wire) << ", " << bp::yh(s.wire) << ")";
					std::cout << "; via = " << s.via;
					std::cout << std::endl;
				}
			}
		}

		return 0;
	}
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

						new_segment.metal_layer = p->getLayer();

						if (ParserDEF::DBG) {
							printf("LAYER %s ", p->getLayer());
						}

						break;

					case DEFIPATH_VIA:

						new_segment.via = p->getVia();
						new_segment.via_ = true;

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

//	string tmpName = net->name();
//	string tmpNdrName;
//
//	Net tmpNet(tmpName);
//
//	vector <Component*> tmpComps;
//
//	int numConnection = net->numConnections();
//	for (int i = 0; i < numConnection; ++i)
//	{
//		string componentName = net->instance(i);
//		string pinName = net->pin(i);
//
//		Pin* tmpPin;
//
//		if (componentName=="PIN") {						//if terminal
//			tmpPin = data->findTerminal(pinName);
//			tmpNet.addTerminal(tmpPin);					//separate terminals
//		}
//		else {
//			tmpComps.push_back(data->findComponent(componentName));		//save pointer to component in temp vector
//			tmpPin = data->findPin(componentName+pinName);
//		}
//
//		tmpNet.addPin(tmpPin);
//	}
//
//	//delete duplicated pointers (if any)
//	set<Component*> s( tmpComps.begin(), tmpComps.end() );
//	tmpComps.assign( s.begin(), s.end() );
//	//add pointer to new net
//	for (Component* comp : tmpComps) {
//		tmpNet.addComponent(comp);
//	}
//
//	if (net->hasNonDefaultRule()) {
//		tmpNdrName = string (net->nonDefaultRule());
//
//		Ndr* tmpNdrPtr = data->findNdr(tmpNdrName);
//		tmpNet.addNdr(tmpNdrPtr);
//	}
//
//	data->nets.push_back(tmpNet);
//	//test = del
//	//data->nets.back().printInfo();

	data->nets.push_back(new_net);

	return 0;
}

int ParserDEF::parseLayers(defrCallbackType_e typ, defiTrack* track, defiUserData* userData) {

	Data* data = reinterpret_cast<Data*>(userData);

	if (ParserDEF::DBG) {
		std::cout << "DEF>  Parsing TRACKS (metal layers) ..." << std::endl;
	}

	for (int i = 0; i < track->numLayers(); i++) {

		data->metal_layers_.push_back(track->layer(i));

		if (ParserDEF::DBG) {
			std::cout << "   Layer: " << track->layer(i) << std::endl;
		}
	}

	if (ParserDEF::DBG) {
		std::cout << "DEF>   Done" << std::endl;
	}

	return 0;
}
