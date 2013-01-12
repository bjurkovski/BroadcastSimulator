#include "pipelineSimulator.h"
#include <cstdlib>
#include <iostream>

using namespace std;

bool PipelineSimulator::broadcast() {
	bool hasMessageToSend = false;
	for(int i=0; i<numProcs; i++) {
		bool msgInPool = messageInPool(i);
		Message msgReceived = receive(i);

		if(msgReceived.isNull() && msgInPool) {
			sendNewMessage(i);
		}
		else if(!msgReceived.isNull()) {
			isSending[i].push(msgReceived);
		}

		if(!isSending[i].empty()) {
			Message m = isSending[i].front();
			 //if(msgDestinations[m.getId()].empty()) {
			if(!hasNextDestination(i, m.getId())) {
				isSending[i].pop();
			}
			else {
				hasMessageToSend = true;
				//int receiver = msgDestinations[m.getId()].front();
				int receiver = getNextDestination(i, m.getId());
				if(send(i, receiver, m)) {
					//msgDestinations[m.getId()].pop();
					removeDestination(m.getId(), receiver);
					isSending[i].pop();
				}
			}
		}

		if(!messagesPool[i].empty()) {
			hasMessageToSend = true;
		}
	}
	return hasMessageToSend;
}

