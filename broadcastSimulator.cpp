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
	firstTimeSent.clear();
	procsToReceive.clear();

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

	//Message m;
	for(int i=0; i<numProcs; i++) {
		isSending.push_back(queue<Message>());
		procClock.push_back(0);
		for(int j=0; j<2; j++)
			procBuffer[j].push_back(MessageQueue());
	}
	currentBuffer = 0;
	numMessages = 0;
}

bool BroadcastSimulator::messageInPool(int proc) {
	if(!isSending[proc].empty()) { 
		Message m = isSending[proc].front();
		//if(msgDestinations[m.getId()].empty()) {
		if(!hasNextDestination(proc, m.getId())) {
			isSending[proc].pop();
		}
	} 

	if((!messagesPool[proc].empty()) && (messagesPool[proc].front() <= round)) {
		return true;
	}
	return false;
}

void BroadcastSimulator::sendNewMessage(int proc) {
	if(messageInPool(proc)) {
		int msgId = numMessages;
		Message m = Message(msgId, proc, proc+'0');

		msgDestinations[msgId] = queue< pair<int, int> >();
		for(int j=0; j<numProcs-1; j++) {
			int p = (proc + j + 1) % numProcs;
			msgDestinations[msgId].push(make_pair(p, -1));
		}

		isSending[proc].push(m);
		messagesPool[proc].pop();
		numMessages++;
	}
}

void BroadcastSimulator::swapBuffers() {
	int nextBuffer = (currentBuffer+1)%2;
	for(int i=0; i<numProcs; i++) {
		while(!procBuffer[currentBuffer][i].empty()) {
			Message m = procBuffer[currentBuffer][i].top();
			procBuffer[currentBuffer][i].pop();
			procBuffer[nextBuffer][i].push(m);
		}
	}
	currentBuffer = nextBuffer;
}

bool BroadcastSimulator::hasMessageToReceive() {
	for(int i=0; i<numProcs; i++) {
		if(!(procBuffer[0][i].empty() && procBuffer[1][i].empty()))
			return true;
	}
	return false;
}

void BroadcastSimulator::run() {
	bool running = true;
	round = 0;
	while(running || hasMessageToReceive()) {
		cout << "** Round " << round << endl;
		running = this->broadcast();
		round++;
		swapBuffers();
	}
	cout << "Avg throughput: " << (double)numMessages/round << endl;
}

bool BroadcastSimulator::hasNextDestination(int proc, int messageId) {
	bool hasDest = false;
	queue< pair<int, int> > tmp;
	while(!msgDestinations[messageId].empty()) {
		pair<int, int> p = msgDestinations[messageId].front();
		if((p.second == proc) || (p.second == -1))
			hasDest = true;
		tmp.push(p);
		msgDestinations[messageId].pop();
	}
	msgDestinations[messageId] = tmp;
	return hasDest;
}

int BroadcastSimulator::getNextDestination(int proc, int messageId) {
		if(msgDestinations[messageId].empty())
			return -1;
		else {
			int nextDest = -1;
			queue< pair<int, int> > tmp;
			while(!msgDestinations[messageId].empty()) {
				pair<int, int> p = msgDestinations[messageId].front();
				if((nextDest==-1) && ((p.second == proc) || (p.second == -1)))
					nextDest = p.first;
				tmp.push(p);
				msgDestinations[messageId].pop();
			}
			msgDestinations[messageId] = tmp;
			return nextDest;
		}
}

void BroadcastSimulator::removeDestination(int messageId, int dest) {
	queue< pair<int, int> > tmp;
	while(!msgDestinations[messageId].empty()) {
		pair<int, int> p = msgDestinations[messageId].front();
		if(p.first != dest)
			tmp.push(p);
		msgDestinations[messageId].pop();
	}
	msgDestinations[messageId] = tmp;
}

bool BroadcastSimulator::send(int sender, int receiver, Message message) {
	message.sender = sender;
	message.time = procClock[sender];
	procBuffer[(currentBuffer+1)%2][receiver].push(message);

	if((int)msgDestinations[message.getId()].size() == numProcs - 1)
		firstTimeSent[message.getId()] = round;
	procsToReceive[message.getId()]++;

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
		procsToReceive[m.getId()]--;
		if((procsToReceive[m.getId()]==0) && (msgDestinations[m.getId()].size()==0)) {
			cout << ">> '" << m.getId() << "' was broadcasted! Latency was " << round-firstTimeSent[m.getId()] << endl;
		}
		return m;
	}
}
