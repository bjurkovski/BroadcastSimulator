#ifndef BROADCAST_SIMULATOR_HPP
#define BROADCAST_SIMULATOR_HPP

#include <queue>
#include <deque>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "message.h"
#include "simulationLog.h"
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
		BroadcastSimulator(bool verbose=true) { this->verbose = verbose; }
		void initialize(std::string configFile);
		SimulationLog run();
	protected:
		// Number of processes and messages in the system, and current round
		int numProcs;
		int numMessages;
		int round;
		SimulationLog log;
		bool verbose;

		// Method meant to be called at the beginning of each round,
		// to set everything up and update the processes state
		virtual void initializeRound(); 
		// One queue per process representing the rounds in which a new
		// message is available (according to the configuration file)
		std::vector< std::queue<int> > messagesPool;
		// Method to check if there's a new message available for
		// this round in the pool
		bool messageInPool(int proc);
		// Puts the first message of the pool in the process' send queue
		// and fills a list of destinations waiting for this message
		void sendNewMessage(int proc); 

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
		//std::vector< std::deque<Message> > isSending;
		std::vector<MessageQueue> isSending;
		// Logical clock per process
		std::vector<int> procClock;
		// Check if there's at least one process with a message in the buffer
		virtual bool hasMessageToReceive(); 

		// For each message sent, the round in which it was first sent (stored to measure latency)
		std::vector<int> firstTimeSent;
		// For each message sent, the time it took to be received
		std::vector<int> msgLatencies;
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
		virtual bool send(int sender, int receiver, Message message); 
		virtual Message receive(int receiver); 
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
	msgLatencies.clear();
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
		//isSending.push_back(std::deque<Message>());
		isSending.push_back(MessageQueue());
		procClock.push_back(0);
		for(int j=0; j<2; j++)
			procBuffer[j].push_back(MessageQueue());
	}
	currentBuffer = 0;
	numMessages = 0;
}

template <class BroadcastPolicy>
SimulationLog BroadcastSimulator<BroadcastPolicy>::run() {
	bool running = false, hasMessageToSend = true;
	log.initialize(numProcs);
	numMessages = 0;
	round = 0;
	while(running || hasMessageToSend || hasMessageToReceive()) {
		if(verbose)
			std::cout << "** Round " << round << std::endl;
		log.newRound();
		initializeRound();
		
		running = BroadcastPolicy::broadcast(*this);
		hasMessageToSend = false;
		for(int i=0; i<numProcs; i++)
			if(!messagesPool[i].empty())
				hasMessageToSend = true;

		round++;
		swapBuffers();
	}

	// Compute measures:
	int sumLatencies = 0;
	for(int i=0; i<(int)msgLatencies.size(); i++) {
		sumLatencies += msgLatencies[i];
	}
	double avgLat = (double)sumLatencies/msgLatencies.size();
	
	double sumSqrDiff = 0;
	for(int i=0; i<(int)msgLatencies.size(); i++) {
		sumSqrDiff += pow(msgLatencies[i] - avgLat, 2);
	}
	
	log.setAvgThroughput((double)numMessages/round);
	log.setAvgLatency(avgLat);
	log.setStdDevLatency(sqrt((double)sumSqrDiff/msgLatencies.size()));

	if(verbose) {
		cout << "Avg throughput: " << log.getAvgThroughput() << endl;
		cout << "Avg latency: " << log.getAvgLatency() << " [StdDev: " << log.getStdDevLatency() << "]" << endl;
	}
	return log;
}

template <class BroadcastPolicy>
void BroadcastSimulator<BroadcastPolicy>::initializeRound() {
	for(int proc=0; proc<numProcs; proc++) {
		while(!isSending[proc].empty()) { 
			//Message m = isSending[proc].front();
			Message m = isSending[proc].top();
			if(hasNextDestination(proc, m.getId()))
				break;
			//else isSending[proc].pop_front();
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
		Message m = Message(msgId, proc, (proc%10)+'0');

		/*
		msgDestinations[msgId] = queue< pair<int, int> >();
		for(int j=0; j<numProcs-1; j++) {
			int p = (proc + j + 1) % numProcs;
			msgDestinations[msgId].push(make_pair(p, -1));
		}
		*/
		msgDestinations[msgId] = BroadcastPolicy::generateMsgDestinations(*this, proc);

		//isSending[proc].push_back(m);
		isSending[proc].push(m);
		firstTimeSent.push_back(0);
		msgLatencies.push_back(0);
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
	log.storeSend(message, receiver);

	if((int)msgDestinations[message.getId()].size() == numProcs - 1)
		firstTimeSent[message.getId()] = round;
	procsToReceive[message.getId()]++;

	for(int i=0; i<numProcs; i++)
		procClock[i]++;

	if(verbose && (sender != receiver))
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
		log.storeReceive(m, receiver);
		log.storeDeliver(m, receiver);
		if(verbose)
			cout << "Process " << receiver << " received '" << m.getId() << "'" << endl;
		procsToReceive[m.getId()]--;
		if((procsToReceive[m.getId()]==0) && (msgDestinations[m.getId()].size()==0)) {
			msgLatencies[m.getId()] = round - firstTimeSent[m.getId()];
			if(verbose)
				cout << ">> '" << m.getId() << "' was broadcasted! Latency was " << msgLatencies[m.getId()] << "." << endl;
		}
		return m;
	}
}

#endif
