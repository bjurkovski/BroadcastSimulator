#include "basicSimulator.h"
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <queue>
#include <iostream>
#include <map>

using namespace std;

BasicSimulator::BasicSimulator(string configFile) {
/*
	Configuration file format:
	NUM_PROCS
	NUM_MESSAGES_PROC_0 TIME_P0_M0 TIME_P0_M1 ... TIME_P0_MX
	NUM_MESSAGES_PROC_1 TIME_P1_M0 ... TIME_P1_MY
	...
	NUM_MESSAGES_PROC_N TIME_PN_M0 ... TIME_PN_MZ
*/

	int numMessages, messageTime;
	FILE* f = fopen(configFile.c_str(), "r");
	fscanf(f, "%d", &numProcs);
	for(int i=0; i<numProcs; i++) {
		fscanf(f, "%d", &numMessages);
		messagesPool.push_back(queue<int>());
		int t = -1;
		for(int j=0; j<numMessages; j++) {
			fscanf(f, "%d", &messageTime);
			if(messageTime <= t) {
				cout << "Invalid message time '" << messageTime << "' for process " << i << endl;
				if(messageTime < 0)
					cout << "Message time must be a non-negative number." << endl;
				else
					cout << "Message times must be inserted inorder, with a maximum of 1 message per process per time unit." << endl;
				cout << "Fix configuration file..." << endl;
				exit(0);
			}
			t = messageTime;
			messagesPool[i].push(messageTime);
		}
	}
	fclose(f);

	Message m;
	for(int i=0; i<numProcs; i++) {
		isSending.push_back(m);
		for(int j=0; j<2; j++)
			procBuffer[j].push_back(m);
	}
	currentBuffer = 0;
}

void BasicSimulator::checkMessagePool(int round) {
	static int numMessages = 0;
	for(int i=0; i<numProcs; i++) {
		Message m = isSending[i];
		if(msgDestinations[m.getId()].empty()) {
				isSending[i].clear();
			}

		if((!messagesPool[i].empty()) && (messagesPool[i].front() <= round))
			if(isSending[i].isNull()) {
				int msgId = numMessages;
				isSending[i] = Message(msgId, i+'0');
				msgDestinations[msgId] = queue<int>();

				for(int j=0; j<numProcs; j++) {
					int p = (i + j) % numProcs;
					if(p != i)
						msgDestinations[msgId].push(p);
				}

				if(send(i, i, isSending[i])) {
					messagesPool[i].pop();
					numMessages++;
				}
				else {
					isSending[i].clear();
				}
			}
	}
}

void BasicSimulator::run() {
	bool hasMessageToSend = true;
	int round = 0;
	while(hasMessageToSend) {
		checkMessagePool(round);
		swapBuffers();

		cout << "** Round " << round << endl;
		hasMessageToSend = false;
		for(int i=0; i<numProcs; i++) {
			receive(i);

			if(!isSending[i].isNull()) {
				Message m = isSending[i];
				if(!msgDestinations[m.getId()].empty()) {
					hasMessageToSend = true;
					int receiver = msgDestinations[m.getId()].front();
					if(send(i, receiver, m)) {
						msgDestinations[m.getId()].pop();
						if(msgDestinations[m.getId()].empty())
							isSending[i].clear();
					}
				}
			}

			if(!messagesPool[i].empty())
				hasMessageToSend = true;
		}
		round++;
	}
}

void BasicSimulator::swapBuffers() {
	currentBuffer = (currentBuffer+1)%2;
}

bool BasicSimulator::send(int sender, int receiver, Message message) {
	if(!procBuffer[(currentBuffer+1)%2][receiver].isNull()) {
		cout << "Process " << receiver << " buffer is full! Will lose '" << message.getId() << "' from process " << sender << endl;
		return false;
	}

	procBuffer[(currentBuffer+1)%2][receiver] = message;
	if(sender != receiver)
		cout << "Process " << sender << " sent '" << message.getId() << "' to process " << receiver << endl;
	return true;
}

Message BasicSimulator::receive(int receiver) {
	Message m = procBuffer[currentBuffer][receiver];
	procBuffer[currentBuffer][receiver] = Message();
	if(!m.isNull()) {
		cout << "Process " << receiver << " received '" << m.getId() << "'" << endl;
	}
	return m;
}
