#ifndef TOTAL_ORDER_BROADCAST_SIMULATOR_H
#define TOTAL_ORDER_BROADCAST_SIMULATOR_H

#include "broadcastSimulator.hpp"

template <class BroadcastPolicy>
class TotalOrderBroadcastSimulator : public BroadcastSimulator<BroadcastPolicy> {
	public:
		TotalOrderBroadcastSimulator() : BroadcastSimulator<BroadcastPolicy>() {
		}

		void initialize(std::string configFile) {
			BroadcastSimulator<BroadcastPolicy>::initialize(configFile);
			remainingAcks.clear();
			waitingForAcks.clear();
			sentMsg.clear();
			for(int i=0; i<this->numProcs; i++) {
				remainingAcks.push_back(std::map<int, int>());
				waitingForAcks.push_back(MessageQueue());
				sentMsg.push_back(false);
			}
		}
	protected:
		// For each process, a table which contains the number of
		// acks this process is still waiting to deliver a message M
		std::vector< std::map<int, int> > remainingAcks;
		// Each process contains a buffer of messages that are just
		// waiting for some acks to be able to be delivered
		std::vector<MessageQueue> waitingForAcks;
		void updateWaitingForAcksList(int receiver, Message m);
		Message getReadyMessage(int receiver);

		// Each process has a flag to indicate if it has already sent
		// a message (acknowledgement) in this turn, because this is 
		// automatically done in the 'receive()' method. This is used
		// to avoid that a process sends two messages in the same round
		std::vector<bool> sentMsg;

		// Overload of the original method: also sets sentMsg[proc]
		// to false in the beginning of each round
		void initializeRound();

		// Overload of the original method: also checks if there's
		// a message in the 'waitingForAcks' buffer
		bool hasMessageToReceive();

		// Overload of the original methods: 
		// - send() is able to send acks (message whose content = 'A')
		// in multicast
		// - receive() automatically multicasts an ack when a new 
		// message is received
		bool send(int sender, int receiver, Message message);
		Message receive(int receiver);
};

using namespace std;

template <class BroadcastPolicy>
bool TotalOrderBroadcastSimulator<BroadcastPolicy>::hasMessageToReceive() {
	if(BroadcastSimulator<BroadcastPolicy>::hasMessageToReceive())
		return true;

	for(int i=0; i<this->numProcs; i++)
		if(!waitingForAcks[i].empty())
			return true;

	return false;
}

template <class BroadcastPolicy>
void TotalOrderBroadcastSimulator<BroadcastPolicy>::initializeRound() {
	BroadcastSimulator<BroadcastPolicy>::initializeRound();

	for(int proc=0; proc<this->numProcs; proc++)
		sentMsg[proc] = false;
}

template <class BroadcastPolicy>
bool TotalOrderBroadcastSimulator<BroadcastPolicy>::send(int sender, int receiver, Message message) {
	if(sentMsg[sender]) {
		return false;
	}
	else {
		message.sender = sender;

		if(message.content == 'A') {
			this->procBuffer[(this->currentBuffer+1)%2][receiver].push(message);
		}
		else {
			message.time = this->procClock[sender];
			this->procBuffer[(this->currentBuffer+1)%2][receiver].push(message);
			this->procClock[sender]++;

			if((int)this->msgDestinations[message.getId()].size() == this->numProcs - 1) {
				this->firstTimeSent[message.getId()] = this->round;
				// The first time we increment this to signalize the sender
				// also has to deliver this message
				this->procsToReceive[message.getId()]++; 
			}
			this->procsToReceive[message.getId()]++; 

			for(int i=0; i<this->numProcs; i++) {
				if(remainingAcks[i][message.getId()] == 0) {
					remainingAcks[i][message.getId()] = this->numProcs - 1; // Proc receives its own ack 'for free'
				}
			}

			if(sender != receiver)
				cout << "Process " << sender << " sent '" << message.getId() << "' to process " << receiver << " [time " << message.time << "]" << endl;
		}

		return true;
	}
}

template <class BroadcastPolicy>
Message TotalOrderBroadcastSimulator<BroadcastPolicy>::receive(int receiver) {
	Message m = getReadyMessage(receiver);
	if(!m.isNull()) return m;

	if(this->procBuffer[this->currentBuffer][receiver].empty()) {
		return Message();
	}
	else {
		sentMsg[receiver] = false;
		Message m = this->procBuffer[this->currentBuffer][receiver].top();
		this->procClock[receiver] = this->procClock[receiver] < m.time ? m.time + 1 : this->procClock[receiver];

		if(m.content == 'A') {
			cout << "Process " << receiver << " received 'ack " << m.getId() << "' from " << m.sender << " [time " << m.time << "]" << endl; //
			this->procBuffer[this->currentBuffer][receiver].pop();
			remainingAcks[receiver][m.getId()]--;

			updateWaitingForAcksList(receiver, m);
			return getReadyMessage(receiver); 
		}
		else {
			this->procBuffer[this->currentBuffer][receiver].pop(); 
			remainingAcks[receiver][m.getId()]--; 

			this->isSending[receiver].push(m); // NEW NEW NEW, testing! - to implement tree
			cout << "Process " << receiver << " broadcasted 'ack " << m.getId() << "' [time " << m.time << "]" << endl; 
			m.content = 'A';
			for(int i=0; i<this->numProcs; i++) {
				if(i != receiver) send(receiver, i, m);
			}
			this->procClock[receiver]++;
			sentMsg[receiver] = true;

			updateWaitingForAcksList(receiver, m); 
			return getReadyMessage(receiver); 
		}
	}
}

template <class BroadcastPolicy>
void TotalOrderBroadcastSimulator<BroadcastPolicy>::updateWaitingForAcksList(int receiver, Message m) {
	MessageQueue bkp;
	while(!waitingForAcks[receiver].empty()) {
		Message msg = waitingForAcks[receiver].top();
		waitingForAcks[receiver].pop();
		if(m.getId() == msg.getId()) {
			m.time = min(m.time, msg.time);
		}
		else bkp.push(msg);
	}
	waitingForAcks[receiver] = bkp;
	waitingForAcks[receiver].push(m); 
}

template <class BroadcastPolicy>
Message TotalOrderBroadcastSimulator<BroadcastPolicy>::getReadyMessage(int receiver) {
	if(!waitingForAcks[receiver].empty()) {
		Message mAcks = waitingForAcks[receiver].top();
		cout << "Process " << receiver << " has a message(" << mAcks.getId() << ") waiting for " << remainingAcks[receiver][mAcks.getId()] << " acks" << endl;
		if(remainingAcks[receiver][mAcks.getId()] == 0) {
			cout << "Process " << receiver << " received '" << mAcks.getId() << "'" << endl;
			waitingForAcks[receiver].pop();
			this->procClock[receiver] = this->procClock[receiver] < mAcks.time ? mAcks.time + 1 : this->procClock[receiver];

			this->procsToReceive[mAcks.getId()]--;
			if((this->procsToReceive[mAcks.getId()]==0) && (this->msgDestinations[mAcks.getId()].size()==0)) {
				cout << ">> '" << mAcks.getId() << "' was broadcasted! Latency was " << this->round - this->firstTimeSent[mAcks.getId()] << endl;
			}

			return mAcks;
		}
	}
	return Message();
}

#endif
