#ifndef PIPELINE_POLICY_HPP
#define PIPELINE_POLICY_HPP

#include "message.h"

class PipelinePolicy {
	protected:
		template <class SimType>
		std::queue<std::pair<int, int> > generateMsgDestinations(SimType& sim, int sender);

		template <class SimType>
		bool broadcast(SimType& sim);
};

using namespace std;

template <class SimType>
queue<pair<int, int> > PipelinePolicy::generateMsgDestinations(SimType& sim, int sender) {
	queue< pair<int, int> > msgDest = queue< pair<int, int> >();
	for(int j=0; j<sim.numProcs-1; j++) {
		int p = (sender + j + 1) % sim.numProcs;
		msgDest.push(make_pair(p, (p-1) % sim.numProcs));
	}
	return msgDest;
}

template <class SimType>
bool PipelinePolicy::broadcast(SimType& sim) {
	bool running = false;
	for(int i=0; i<sim.numProcs; i++) {
		bool msgInPool = sim.messageInPool(i);
		Message msgReceived = sim.receive(i);

		if(msgReceived.isNull() && msgInPool) {
			sim.sendNewMessage(i);
		}
		else if(!msgReceived.isNull()) {
			//sim.isSending[i].push_back(msgReceived);
			sim.isSending[i].push(msgReceived);
		}

		if(!sim.isSending[i].empty()) {
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
					//sim.isSending[i].pop_front();
					sim.isSending[i].pop();
				}
			}
		}
	}
	return running;
}

#endif
