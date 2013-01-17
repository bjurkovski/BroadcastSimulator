#ifndef TREE_POLICY_HPP
#define TREE_POLICY_HPP

//#include "broadcastSimulator.h"
#include <cmath>
#include "message.h"

class TreePolicy {
	protected:
		template <class SimType>
		std::queue<std::pair<int, int> > generateMsgDestinations(SimType& sim, int sender);

		template <class SimType>
		bool broadcast(SimType& sim);
};

using namespace std;

template <class SimType>
queue<pair<int, int> > TreePolicy::generateMsgDestinations(SimType& sim, int sender) {
	int level=0;
	int senderOffset=0;
	queue< pair<int, int> > msgDest = queue< pair<int, int> >();
	for(int j=0; j<sim.numProcs-1; j++) {
		int p = (sender + j + 1) % sim.numProcs;
		int msender = (sender + senderOffset) % sim.numProcs;
		msgDest.push(make_pair(p, msender));

		senderOffset++;
		if(senderOffset == pow(2, level)) {
			senderOffset = 0;
			level++;
		}
	}
	return msgDest;
}

template <class SimType>
bool TreePolicy::broadcast(SimType& sim) {
	bool running = false;
	for(int i=0; i<sim.numProcs; i++) {
		Message msgReceived = sim.receive(i);
		if(!msgReceived.isNull())
			//sim.isSending[i].push_back(msgReceived);
			sim.isSending[i].push(msgReceived);

		bool msgInPool = sim.messageInPool(i);
		if(msgInPool) {
			sim.sendNewMessage(i);
		}

		if(!sim.isSending[i].empty())  {
			//Message m = sim.isSending[i].front();
			Message m = sim.isSending[i].top();
			if(m.content == 'A') {
				if(sim.send(i, -1, m))
					sim.isSending[i].pop();
			}
			else if(!sim.hasNextDestination(i, m.getId())) {
				//sim.isSending[i].pop_front();
				sim.isSending[i].pop();
			}
			else {
				running = true;
				int receiver = sim.getNextDestination(i, m.getId());
				if(sim.send(i, receiver, m)) {
					sim.removeDestination(m.getId(), receiver);
				}
			}
		}
	}
	return running;
}

#endif
