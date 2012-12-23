#include "broadcastSimulator.h"
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <queue>
#include <iostream>
#include <map>

using namespace std;

void BroadcastSimulator::initialize(string configFile) {
/*
	Configuration file format:
	NUM_PROCS
	NUM_MESSAGES_PROC_0 TIME_P0_M0 TIME_P0_M1 ... TIME_P0_MX
	NUM_MESSAGES_PROC_1 TIME_P1_M0 ... TIME_P1_MY
	...
	NUM_MESSAGES_PROC_N TIME_PN_M0 ... TIME_PN_MZ
*/
	messagesPool.clear();
	procClock.clear();
	procBuffer[0].clear();
	procBuffer[1].clear();
	isSending.clear();
	msgDestinations.clear();

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
		procClock.push_back(0);
		for(int j=0; j<2; j++)
			procBuffer[j].push_back(MessageQueue());
	}
	currentBuffer = 0;
	this->numMessages = 0;
}

Message BroadcastSimulator::checkMessagePool(int proc, int round) {
	Message m = isSending[proc];
	if(msgDestinations[m.getId()].empty()) {
		isSending[proc].clear();
	}

	if((!messagesPool[proc].empty()) && (messagesPool[proc].front() <= round))
		if(isSending[proc].isNull()) {
		int msgId = numMessages;
			//isSending[i] = Message(msgId, i+'0');
			Message m = Message(msgId, proc+'0');
			msgDestinations[msgId] = queue<int>();

			for(int j=0; j<numProcs-1; j++) {
				int p = (proc + j + 1) % numProcs;
				msgDestinations[msgId].push(p);
			}

			return m;
			/*
			if(send(i, i, isSending[i])) {
				messagesPool[i].pop();
				numMessages++;
			}
			else {
				isSending[i].clear();
			}
			*/
		}
	return Message();
}

void BroadcastSimulator::startSending(int proc, Message m) {
	isSending[proc] = m;
	messagesPool[proc].pop();
	numMessages++;
}

void BroadcastSimulator::swapBuffers() {
	int nextBuffer = (currentBuffer+1)%2;
	for(int i=0; i<numProcs; i++) {
		if(!procBuffer[currentBuffer][i].empty()) {
			Message m = procBuffer[currentBuffer][i].top();
			procBuffer[currentBuffer][i].pop();
			procBuffer[nextBuffer][i].push(m);
		}
	}
	currentBuffer = nextBuffer;
}

bool BroadcastSimulator::send(int sender, int receiver, Message message) {
/*
	if(!procBuffer[(currentBuffer+1)%2][receiver].isNull()) {
		cout << "Process " << receiver << " buffer is full! Will lose '" << message.getId() << "' from process " << sender << endl;
		return false;
	}
*/

	message.sender = sender;
	message.time = procClock[sender];
	procBuffer[(currentBuffer+1)%2][receiver].push(message);
	for(int i=0; i<numProcs; i++)
		procClock[i]++;
	if(sender != receiver)
		cout << "Process " << sender << " sent '" << message.getId() << "' to process " << receiver << endl;
	return true;
}

Message BroadcastSimulator::receive(int receiver) {
	if(procBuffer[currentBuffer][receiver].empty()) {
		return Message();
	}
	else {
		Message m = procBuffer[currentBuffer][receiver].top();
		procBuffer[currentBuffer][receiver].pop();
		cout << "Process " << receiver << " received '" << m.getId() << "'" << endl;
		return m;
	}
}
