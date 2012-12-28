#include "treeSimulator.h"
#include <queue>
#include <iostream>

using namespace std;


void TreeSimulator::run() {
	bool hasMessageToSend = true;
	int round = 0;
	while(hasMessageToSend || hasMessageToReceive()) {
		cout << "** Round " << round << endl;
		hasMessageToSend = false;
		for(int i=0; i<numProcs; i++) {
			Message msgReceived = receive(i);
			if(!msgReceived.isNull())
				//isSending[i] = msgReceived;
				isSending[i].push(msgReceived);

			//if(isSending[i].isNull()) {
			if(isSending[i].empty()) {
				Message toSend = checkMessagePool(i, round);
				if(!toSend.isNull()) {
					startSending(i, toSend);
				}
			}

			//if(!isSending[i].isNull())  {
			if(!isSending[i].empty())  {
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
