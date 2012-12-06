#ifndef TREE_SIMULATOR_H
#define TREE_SIMULATOR_H

#include "basicSimulator.h"

class TreeSimulator : public BasicSimulator {
	public:
		TreeSimulator(std::string configFile) : BasicSimulator(configFile) {
		}

		void run();
};

#endif
