#include "treeSimulator.h"
#include <queue>
#include <iostream>
#include <cmath>

using namespace std;

void TreeSimulator::sendNewMessage(int proc) {
	if(messageInPool(proc)) {
		int msgId = numMessages;
		Message m = Message(msgId, proc, proc+'0');

		int level=0;
		int senderOffset=0;
		msgDestinations[msgId] = queue< pair<int, int> >();
		for(int j=0; j<numProcs-1; j++) {
			int p = (proc + j + 1) % numProcs;
			int msender = (proc+senderOffset)%numProcs;
			msgDestinations[msgId].push(make_pair(p, msender));

			senderOffset++;
			if(senderOffset == pow(2, level)) {
				senderOffset = 0;
				level++;
			}
		}   

		isSending[proc].push(m);
		messagesPool[proc].pop();
		numMessages++;
	}   
}

bool TreeSimulator::broadcast() {
	bool hasMessageToSend = false;
	hasMessageToSend = false;
	for(int i=0; i<numProcs; i++) {
		Message msgReceived = receive(i);
		if(!msgReceived.isNull())
			isSending[i].push(msgReceived);

		bool msgInPool = messageInPool(i);
		if(msgInPool) {
			sendNewMessage(i);
		}

		if(!isSending[i].empty())  {
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
				}
			}
		}

		if(!messagesPool[i].empty()) {
			hasMessageToSend = true;
		}
	}
	return hasMessageToSend;
}
