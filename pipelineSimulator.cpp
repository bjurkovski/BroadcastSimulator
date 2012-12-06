#include "pipelineSimulator.h"
#include <cstdlib>
#include <iostream>

using namespace std;

void PipelineSimulator::run() {
	bool hasMessageToSend = true;
	int round = 0;
	while(hasMessageToSend) {
		checkMessagePool(round);
		swapBuffers();

		cout << "** Round " << round << endl;
		hasMessageToSend = false;
		for(int i=0; i<numProcs; i++) {
			Message msgReceived = receive(i);
			if(!msgReceived.isNull()) {
				isSending[i] = msgReceived;

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
	}
}

