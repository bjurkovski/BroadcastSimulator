#ifndef BROADCAST_SIMULATOR_H
#define BROADCAST_SIMULATOR_H

#include <queue>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "message.h"
#include "basicPolicy.hpp"
#include "treePolicy.hpp"
#include "pipelinePolicy.hpp"

template <class BroadcastPolicy>
class BroadcastSimulator : public BroadcastPolicy {
	// Accepted broadcast protocols
	friend class BasicPolicy;
	friend class TreePolicy;
	friend class PipelinePolicy;

	// Method called by the sendNewMessage() method:
	// generates the mapping of receiver to sender for each message
	using BroadcastPolicy::generateMsgDestinations;
	// Method called by the run() method:
	// contains the implementation of the broadcast protocol
	using BroadcastPolicy::broadcast;

	public:
		BroadcastSimulator() { }
		void initialize(std::string configFile);
		void run();
	protected:
		// Number of processes and messages in the system, and current round
		int numProcs;
		int numMessages;
		int round;

		// Method meant to be called at the beginning of each round,
		// to set everything up and update the processes state
		virtual void initializeRound(); //v
		// One queue per process representing the rounds in which a new
		// message is available (according to the configuration file)
		std::vector< std::queue<int> > messagesPool;
		// Method to check if there's a new message available for
		// this round in the pool
		bool messageInPool(int proc);
		// Puts the first message of the pool in the process' send queue
		// and fills a list of destinations waiting for this message
		void sendNewMessage(int proc); //v

		// Each process has two buffers to receive a message: one to
		// represent the current buffer and one of future messages
		// (that will be available only at the end of the round)
		int currentBuffer;
		std::vector<MessageQueue> procBuffer[2];
		// This method needs to be called at the end of every round,
		// so messages sent on the current round will be available for
		// the corresponding processes in the next one
		void swapBuffers();

		// Each process has a queue of messages to be sent
		std::vector< std::queue<Message> > isSending;
		// Logical clock per process
		std::vector<int> procClock;
		// Check if there's at least one process with a message in the buffer
		virtual bool hasMessageToReceive(); //v

		// First time (round) a message M was sent, stored to measure latency
		std::map<int, int> firstTimeSent;
		// Number of processes that still need to receive a message M:
		// used to know when I message was sucessfully broadcasted
		std::map<int, int> procsToReceive;
		// Queue of processes to which a message M still needs to be sent
		// - First element of the pair in the queue is the process which 
		// still needs to receive the message
		// - Second element of the pair is the process which is supposed
		// to send the message (or -1 if it doesn't matter who sends)
		std::map<int, std::queue< std::pair<int, int> > > msgDestinations;
		bool hasNextDestination(int proc, int messageId);
		int getNextDestination(int proc, int messageId);
		void removeDestination(int messageId, int dest);

		// Network abstractions
		virtual bool send(int sender, int receiver, Message message); //v
		virtual Message receive(int receiver); //v
};

using namespace std;

template <class BroadcastPolicy>
void BroadcastSimulator<BroadcastPolicy>::initialize(string configFile) {
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
	FILE* f = fopen(("input/" + configFile) .c_str(), "r");
	fscanf(f, "%d", &numProcs);
	for(int i=0; i<numProcs; i++) {
		fscanf(f, "%d", &numMessages);
		messagesPool.push_back(std::queue<int>());
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
		isSending.push_back(std::queue<Message>());
		procClock.push_back(0);
		for(int j=0; j<2; j++)
			procBuffer[j].push_back(MessageQueue());
	}
	currentBuffer = 0;
	numMessages = 0;
}

template <class BroadcastPolicy>
void BroadcastSimulator<BroadcastPolicy>::run() {
	bool running = true;
	numMessages = 0;
	round = 0;
	while(running || hasMessageToReceive()) {
		std::cout << "** Round " << round << std::endl;
		initializeRound();
		running = BroadcastPolicy::broadcast(*this);
		round++;
		swapBuffers();
	}
	std::cout << "Avg throughput: " << (double)numMessages/round << std::endl;
}

template <class BroadcastPolicy>
void BroadcastSimulator<BroadcastPolicy>::initializeRound() {
	for(int proc=0; proc<numProcs; proc++) {
		while(!isSending[proc].empty()) { 
			Message m = isSending[proc].front();
			if(hasNextDestination(proc, m.getId()))
				break;
			else isSending[proc].pop();
		} 
	}
}

template <class BroadcastPolicy>
bool BroadcastSimulator<BroadcastPolicy>::messageInPool(int proc) {
	if((!messagesPool[proc].empty()) && (messagesPool[proc].front() <= round)) {
		return true;
	}
	return false;
}

template <class BroadcastPolicy>
void BroadcastSimulator<BroadcastPolicy>::sendNewMessage(int proc) {
	if(messageInPool(proc)) {
		int msgId = numMessages;
		Message m = Message(msgId, proc, proc+'0');

		/*
		msgDestinations[msgId] = queue< pair<int, int> >();
		for(int j=0; j<numProcs-1; j++) {
			int p = (proc + j + 1) % numProcs;
			msgDestinations[msgId].push(make_pair(p, -1));
		}
		*/
		msgDestinations[msgId] = BroadcastPolicy::generateMsgDestinations(*this, proc);

		isSending[proc].push(m);
		messagesPool[proc].pop();
		numMessages++;
	}
}

template <class BroadcastPolicy>
void BroadcastSimulator<BroadcastPolicy>::swapBuffers() {
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

template <class BroadcastPolicy>
bool BroadcastSimulator<BroadcastPolicy>::hasMessageToReceive() {
	for(int i=0; i<numProcs; i++) {
		if(!(procBuffer[0][i].empty() && procBuffer[1][i].empty()))
			return true;
	}
	return false;
}

template <class BroadcastPolicy>
bool BroadcastSimulator<BroadcastPolicy>::hasNextDestination(int proc, int messageId) {
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

template <class BroadcastPolicy>
int BroadcastSimulator<BroadcastPolicy>::getNextDestination(int proc, int messageId) {
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

template <class BroadcastPolicy>
void BroadcastSimulator<BroadcastPolicy>::removeDestination(int messageId, int dest) {
	queue< pair<int, int> > tmp;
	while(!msgDestinations[messageId].empty()) {
		pair<int, int> p = msgDestinations[messageId].front();
		if(p.first != dest)
			tmp.push(p);
		msgDestinations[messageId].pop();
	}
	msgDestinations[messageId] = tmp;
}

template <class BroadcastPolicy>
bool BroadcastSimulator<BroadcastPolicy>::send(int sender, int receiver, Message message) {
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

template <class BroadcastPolicy>
Message BroadcastSimulator<BroadcastPolicy>::receive(int receiver) {
	if(procBuffer[currentBuffer][receiver].empty()) {
		return Message();
	}
	else {
		Message m = procBuffer[currentBuffer][receiver].top();
		procBuffer[currentBuffer][receiver].pop();
		cout << "Process " << receiver << " received '" << m.getId() << "'" << endl;
		procsToReceive[m.getId()]--;
		if((procsToReceive[m.getId()]==0) && (msgDestinations[m.getId()].size()==0)) {
			cout << ">> '" << m.getId() << "' was broadcasted! Latency was " << round - firstTimeSent[m.getId()] << "." << endl;
		}
		return m;
	}
}

#endif