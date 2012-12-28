#ifndef TO_TREE_SIMULATOR_H
#define TO_TREE_SIMULATOR_H

#include "treeSimulator.h"

class TO_TreeSimulator : public TreeSimulator {
	public:
		TO_TreeSimulator() : TreeSimulator() {
		}

		void initialize(std::string configFile) {
			TreeSimulator::initialize(configFile);
			remainingAcks.clear();
			waitingForAcks.clear();
			sentMsg.clear();
			for(int i=0; i<numProcs; i++) {
				remainingAcks.push_back(std::map<int, int>());
				waitingForAcks.push_back(MessageQueue());
				sentMsg.push_back(false);
			}
		}
	protected:
		std::vector< std::map<int, int> > remainingAcks;
		std::vector<MessageQueue> waitingForAcks;
		std::vector<bool> sentMsg;
		bool hasMessageToReceive();
		bool send(int sender, int receiver, Message message);
		Message receive(int receiver);
};

#endif
