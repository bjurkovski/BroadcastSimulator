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
		// For each process, a table which contains the number of
		// acks this process is still waiting to deliver a message M
		std::vector< std::map<int, int> > remainingAcks;
		// Each process contains a buffer of messages that are just
		// waiting for some acks to be able to be delivered
		std::vector<MessageQueue> waitingForAcks;
		// Each process has a flag to indicate if it has already sent
		// a message (acknowledgement) in this turn, because this is 
		// automatically done in the 'receive()' method. This is used
		// to avoid that a process sends two messages in the same round
		std::vector<bool> sentMsg;

		// Overload of the original method: also checks if there's
		// a message in the 'waitingForAcks' buffer
		bool hasMessageToReceive();

		// Overload of the original methods: 
		// - send() is able to send acks (message whose content = 'A')
		// in multicast
		// - receive() automatically multicasts an ack when a new 
		// message is received
		bool send(int sender, int receiver, Message message);
		Message receive(int receiver);
};

#endif
