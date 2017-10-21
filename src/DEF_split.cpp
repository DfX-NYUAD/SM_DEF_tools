/*
 * =====================================================================================
 *
 *    Description:  DEF splitting handler
 *
 *    Copyright (C) 2017 Johann Knechtel, johann aett jknechtel dot de
 *
 * =====================================================================================
 */

// own header
#include "DEF_split.hpp"

int main (int argc, char** argv) {
	DEF_split split;

	std::cout << std::endl;
	std::cout << "DEF_split: splitting a DEF file into FEOL/BEOL, after a give metal layer" << std::endl;
	std::cout << "------------------------------------------------------------------------" << std::endl << std::endl;

	// parse program parameters, and test for DEF/LEF files
	split.parseParameters(argc, argv);

	// parse DEF/LEF
	split.parseDEF();
	split.parseLEF();

	// TODO split DEF into FEOL/BEOL
	// TODO write back FEOL part
}

void DEF_split::parseParameters(int const& argc, char** argv) {
	std::ifstream in;

	// print command-line parameters
	//
	if (argc < 4) {
		std::cout << "IO> Usage: " << argv[0] << " DEF_file LEF_file split_layer" << std::endl;
		std::cout << "IO> " << std::endl;
		std::cout << "IO> Mandatory parameter ``DEF_file'': the DEF file to be split" << std::endl;
		std::cout << "IO> Mandatory parameter ``LEF_file'': the LEF file related to the DEF file" << std::endl;
		std::cout << "IO> Mandatory parameter ``split_layer'': the layer after which to split the DEF file -- provide string, e.g., metal2" << std::endl;

		exit(1);
	}

	this->DEF_file = argv[1];
	this->LEF_file = argv[2];
	this->split_layer = argv[3];

	// test files
	//
	in.open(this->DEF_file.c_str());
	if (!in.good()) {
		std::cout << "IO> ";
		std::cout << "No such file: " << this->DEF_file << std::endl;
		exit(1);
	}
	in.close();

	in.open(LEF_file.c_str());
	if (!in.good()) {
		std::cout << "IO> ";
		std::cout << "No such file: " << LEF_file << std::endl;
		exit(1);
	}
	in.close();

	std::cout << "IO> DEF file: " << this->DEF_file << std::endl;
	std::cout << "IO> LEF file: " << this->LEF_file << std::endl;
	std::cout << "IO> Metal layer to split after: " << this->split_layer << std::endl;
	std::cout << std::endl;
}

void DEF_split::parseDEF() {
	FILE *DEF;

	std::cout << "DEF> Start parsing DEF file ..." << std::endl;

	// pointer to user data; made available in all parser callbacks
	Data* userData = &this->data;

	DEF = fopen(this->DEF_file.c_str(), "r");

	defrInit();
	defrSetUserData(userData);

	//TODO for new Si2 parser
	//defrInitSession(1);

	// define callback functions
	//
	// regular nets
	defrSetNetStartCbk((defrIntegerCbkFnType) parseNetsStart);
	defrSetNetEndCbk((defrVoidCbkFnType) parseNetsEnd);
	defrSetNetCbk((defrNetCbkFnType) parseNets);
	// augment nets with path data
	defrSetAddPathToNet();

	// trigger parser; read DEF sections of interes
	//
	// 4th parameter: 1 -- specifies that the data is case sensitive
	int status = defrRead(DEF, this->DEF_file.c_str(), userData, 1);
	if (status != 0) {
		std::cout << "DEF> Error in parser; abort" << std::endl;
		exit (1);
	}

	defrClear();
	fclose(DEF);

	std::cout << "DEF> End parsing DEF file" << std::endl;
}

void DEF_split::parseLEF() {
}

int DEF_split::parseNetsStart(defrCallbackType_e typ, int nets, defiUserData* userData) {

	std::cout << "DEF>  Parsing NETS ..." << std::endl;

	Data* data = reinterpret_cast<Data*>(userData);

	data->DEF_items.nets = static_cast<unsigned>(nets);

	std::cout << "DEF>   " << nets << " nets to be parsed ..." << std::endl;

	return 0;
}

int DEF_split::parseNetsEnd(defrCallbackType_e typ, void* variable, defiUserData* userData) {

	Data* data = reinterpret_cast<Data*>(userData);

	if (data->nets.size() != data->DEF_items.nets) {

		std::cout << "DEF>   Error; only " << data->nets.size() << " nets have been parsed ..." << std::endl;
		return 1;
	}
	else {
		std::cout << "DEF>   Done" << std::endl;

		if (DEF_split::DBG) {

			for (Data::Net& n : data->nets) {
				std::cout << "DEF>    Net: " << n.name << std::endl;

				for (Data::Segment& s : n.segments) {
					std::cout << "DEF>     Segment: " << s.metal_layer << std::endl;
				}
			}
		}

		return 0;
	}
}

int DEF_split::parseNets(defrCallbackType_e typ, defiNet* net, defiUserData* userData) {
	Data::Net new_net;
	defiPath* p;
	defiWire* wire;
	int x, y, z;
	int path;

	Data* data = reinterpret_cast<Data*>(userData);

	new_net.name = net->name();

	if (DEF_split::DBG) {
		std::cout << "DEF>    Parsing net " << new_net.name << std::endl;
	}

	for (int i = 0; i < net->numWires(); i++) {

		wire = net->wire(i);

		if (DEF_split::DBG) {
			std::cout << "DEF>     wire(" << i << ")" << std::endl;
			std::cout << "DEF>     wire(" << i << ")->wireType(): " << wire->wireType() << std::endl;
		}

		for (int j = 0; j < wire->numPaths(); j++) {

			p = wire->path(j);
			p->initTraverse();

			if (DEF_split::DBG) {
				std::cout << "DEF>     wire(" << i << ")->path(" << j << "):	";
			}

			// my representation of the paths are called segments
			//
			Data::Segment new_segment;

			while ((path = static_cast<int>(p->next())) != DEFIPATH_DONE) {

				switch (path) {
					case DEFIPATH_LAYER:

						new_segment.metal_layer = p->getLayer();

						if (DEF_split::DBG) {
							printf("%s ", p->getLayer());
						}

						break;

					case DEFIPATH_VIA:

						if (DEF_split::DBG) {
							printf("%s ", p->getVia());
						}
						break;

					//case DEFIPATH_WIDTH:
					//	printf("%d ", p->getWidth());
					//	break;

					case DEFIPATH_POINT:
						p->getPoint(&x, &y);

						if (DEF_split::DBG) {
							printf("( %d %d ) ", x, y);
						}
						break;

					// TODO what's a flushpoint?
					case DEFIPATH_FLUSHPOINT:
						p->defiPath::getFlushPoint(&x, &y, &z);

						if (DEF_split::DBG) {
							printf("( %d %d %d ) ", x, y, z);
						}
						break;

					// other data of paths can be ignored
					default:

						if (DEF_split::DBG) {
							printf("NOT HANDLED ");
						}
						break;
				}
			}

			// all elements of this segment are parsed; store it
			//
			new_net.segments.push_back(new_segment);

			if (DEF_split::DBG) {
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
