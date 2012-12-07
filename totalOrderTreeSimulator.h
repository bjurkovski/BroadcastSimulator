#ifndef TO_TREE_SIMULATOR_H
#define TO_TREE_SIMULATOR_H

#include "treeSimulator.h"

class TO_TreeSimulator : public TreeSimulator {
	public:
		TO_TreeSimulator(std::string configFile) : TreeSimulator(configFile) {
			for(int i=0; i<numProcs; i++)
				remainingAcks.push_back(std::map<int, int>());
		}

		std::vector< std::map<int, int> > remainingAcks;
		bool send(int sender, int receiver, Message message);
		Message receive(int receiver);
};

#endif
