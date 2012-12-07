#include "pipelineSimulator.h"
#include <cstdlib>
#include <iostream>

using namespace std;

void PipelineSimulator::run() {
	bool hasMessageToSend = true;
	int round = 0;
	while(hasMessageToSend) {
		cout << "** Round " << round << endl;
		hasMessageToSend = false;
		for(int i=0; i<numProcs; i++) {
			Message toSend = checkMessagePool(i, round);
			Message msgReceived = receive(i);

			if(msgReceived.isNull()) {
				if(!toSend.isNull() && isSending[i].isNull()) {
					startSending(i, toSend);
				}
			}
			else {
				isSending[i] = msgReceived;
			}

			if(!isSending[i].isNull()) {
				Message m = isSending[i];
				if(msgDestinations[m.getId()].empty()) {
					isSending[i].clear();
				}
				else {
					hasMessageToSend = true;
					int receiver = msgDestinations[m.getId()].front();
					if(send(i, receiver, m)) {
						msgDestinations[m.getId()].pop();
						isSending[i].clear();
					}
				}
			}

			if(!messagesPool[i].empty()) {
				hasMessageToSend = true;
			}
		}
		round++;
		swapBuffers();
	}
}

