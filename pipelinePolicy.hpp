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
	bool hasMessageToSend = false;
	for(int i=0; i<sim.numProcs; i++) {
		bool msgInPool = sim.messageInPool(i);
		Message msgReceived = sim.receive(i);

		if(msgReceived.isNull() && msgInPool) {
			sim.sendNewMessage(i);
		}
		else if(!msgReceived.isNull()) {
			sim.isSending[i].push(msgReceived);
		}

		if(!sim.isSending[i].empty()) {
			Message m = sim.isSending[i].front();
			 //if(msgDestinations[m.getId()].empty()) {
			if(!sim.hasNextDestination(i, m.getId())) {
				sim.isSending[i].pop();
			}
			else {
				hasMessageToSend = true;
				//int receiver = msgDestinations[m.getId()].front();
				int receiver = sim.getNextDestination(i, m.getId());
				if(sim.send(i, receiver, m)) {
					//msgDestinations[m.getId()].pop();
					sim.removeDestination(m.getId(), receiver);
					sim.isSending[i].pop();
				}
			}
		}

		if(!sim.messagesPool[i].empty()) {
			hasMessageToSend = true;
		}
	}
	return hasMessageToSend;
}

#endif
