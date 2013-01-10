#include "basicSimulator.h"
#include <iostream>

using namespace std;

bool BasicSimulator::broadcast() {
	bool hasMessageToSend = false;
	for(int i=0; i<numProcs; i++) {
		bool msgInPool = messageInPool(i);
		if(msgInPool) {
			sendNewMessage(i);
		}
		receive(i);

		if(!isSending[i].empty()) {
			Message m = isSending[i].front();
			if(!msgDestinations[m.getId()].empty()) {
				hasMessageToSend = true;
				int receiver = msgDestinations[m.getId()].front();
				if(send(i, receiver, m)) {
					msgDestinations[m.getId()].pop();
					if(msgDestinations[m.getId()].empty())
						isSending[i].pop();
				}
			}
		}

		if(!messagesPool[i].empty())
			hasMessageToSend = true;
	}
	return hasMessageToSend;
}
