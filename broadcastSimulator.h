#ifndef BROADCAST_SIMULATOR_H
#define BROADCAST_SIMULATOR_H

#include <queue>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include "message.h"

class BroadcastSimulator {
	public:
		BroadcastSimulator() { }
		void initialize(std::string configFile);
		void run();
	protected:
		// Number of processes and messages in the system, and current round
		int numProcs;
		int numMessages;
		int round;

		// One queue per process representing the rounds in which a new
		// message is available (according to the configuration file)
		std::vector< std::queue<int> > messagesPool;
		// Method to check if there's a new message available for
		// this round in the pool
		bool messageInPool(int proc);
		// Puts the first message of the pool in the process' send queue
		// and fills a list of destinations waiting for this message
		virtual void sendNewMessage(int proc);

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
		virtual bool hasMessageToReceive();

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
		virtual bool send(int sender, int receiver, Message message);
		virtual Message receive(int receiver);
		// Abstract method called by the run() method:
		// contains the implementation of the broadcast protocol
		virtual bool broadcast() = 0;
};

#endif
