#include "pipelineSimulator.h"
#include <cstdlib>
#include <iostream>

using namespace std;

bool PipelineSimulator::broadcast(int round) {
	bool hasMessageToSend = false;
	for(int i=0; i<numProcs; i++) {
		Message toSend = checkMessagePool(i, round);
		Message msgReceived = receive(i);

		if(msgReceived.isNull()) {
			//if(!toSend.isNull() && isSending[i].isNull()) {
			if(!toSend.isNull()) {
				startSending(i, toSend);
			}
		}
		else {
			//isSending[i] = msgReceived;
			isSending[i].push(msgReceived);
		}

		//if(!isSending[i].isNull()) {
		if(!isSending[i].empty()) {
			//Message m = isSending[i];
			Message m = isSending[i].front();
			if(msgDestinations[m.getId()].empty()) {
				//isSending[i].clear();
				isSending[i].pop();
			}
			else {
				hasMessageToSend = true;
				int receiver = msgDestinations[m.getId()].front();
				if(send(i, receiver, m)) {
					msgDestinations[m.getId()].pop();
					//isSending[i].clear();
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

