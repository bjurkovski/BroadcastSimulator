#include "basicSimulator.h"
#include <iostream>

using namespace std;

void BasicSimulator::run() {
	bool hasMessageToSend = true;
	int round = 0;
	while(hasMessageToSend || hasMessageToReceive()) {
		cout << "** Round " << round << endl;
		hasMessageToSend = false;
		for(int i=0; i<numProcs; i++) {
			Message toSend = checkMessagePool(i, round);
			//if(!toSend.isNull() && isSending[i].isNull()) {
			if(!toSend.isNull()) {
				startSending(i, toSend);
			}
			receive(i);

			//if(!isSending[i].isNull()) {
			if(!isSending[i].empty()) {
				//Message m = isSending[i];
				Message m = isSending[i].front();
				if(!msgDestinations[m.getId()].empty()) {
					hasMessageToSend = true;
					int receiver = msgDestinations[m.getId()].front();
					if(send(i, receiver, m)) {
						msgDestinations[m.getId()].pop();
						if(msgDestinations[m.getId()].empty())
							//isSending[i].clear();
							isSending[i].pop();
					}
				}
			}

			if(!messagesPool[i].empty())
				hasMessageToSend = true;
		}
		round++;
		swapBuffers();
	}
}
