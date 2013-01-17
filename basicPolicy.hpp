#ifndef BASIC_POLICY_HPP
#define BASIC_POLICY_HPP

//#include "broadcastSimulator.h"
#include <queue>
#include "message.h"

class BasicPolicy {
	protected:
		template <class SimType>
		std::queue<std::pair<int, int> > generateMsgDestinations(SimType& sim, int sender);

		template <class SimType>
		bool broadcast(SimType& sim);
};

using namespace std;

template <class SimType>
queue<pair<int, int> > BasicPolicy::generateMsgDestinations(SimType& sim, int sender) {
	queue< pair<int, int> > msgDest = queue< pair<int, int> >();
	for(int j=0; j<sim.numProcs-1; j++) {
		int p = (sender + j + 1) % sim.numProcs;
		msgDest.push(make_pair(p, sender));
	}
	return msgDest;
}

template <class SimType>
bool BasicPolicy::broadcast(SimType& sim) {
	bool running = false;
	for(int i=0; i<sim.numProcs; i++) {
		bool msgInPool = sim.messageInPool(i);
		if(msgInPool) {
			sim.sendNewMessage(i);
		}
		sim.receive(i);

		if(!sim.isSending[i].empty()) {
			//Message m = sim.isSending[i].front();
			Message m = sim.isSending[i].top();
			if(sim.hasNextDestination(i, m.getId())) {
				running = true;
				int receiver = sim.getNextDestination(i, m.getId());
				if(sim.send(i, receiver, m)) {
					sim.removeDestination(m.getId(), receiver);
					if(!sim.hasNextDestination(i, m.getId()))
						//sim.isSending[i].pop_front();
						sim.isSending[i].pop();
				}
			}
		}
	}
	return running;
}

#endif
