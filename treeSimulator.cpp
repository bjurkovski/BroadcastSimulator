#include "treeSimulator.h"
#include <queue>
#include <iostream>

using namespace std;

bool TreeSimulator::broadcast() {
	bool hasMessageToSend = false;
	hasMessageToSend = false;
	for(int i=0; i<numProcs; i++) {
		Message msgReceived = receive(i);
		if(!msgReceived.isNull())
			isSending[i].push(msgReceived);

		if(isSending[i].empty()) {
			bool msgInPool = messageInPool(i);
			if(msgInPool) {
				sendNewMessage(i);
			}
		}

		if(!isSending[i].empty())  {
			Message m = isSending[i].front();
			if(msgDestinations[m.getId()].empty()) {
				isSending[i].pop();
			}
			else {
				hasMessageToSend = true;
				int receiver = msgDestinations[m.getId()].front();
				if(send(i, receiver, m)) {
					msgDestinations[m.getId()].pop();
				}
			}
		}

		if(!messagesPool[i].empty()) {
			hasMessageToSend = true;
		}
	}
	return hasMessageToSend;
}
