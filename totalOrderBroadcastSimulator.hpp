#ifndef TOTAL_ORDER_BROADCAST_SIMULATOR_HPP
#define TOTAL_ORDER_BROADCAST_SIMULATOR_HPP

#include "broadcastSimulator.hpp"

template <class BroadcastPolicy>
class TotalOrderBroadcastSimulator : public BroadcastSimulator<BroadcastPolicy> {
	public:
		TotalOrderBroadcastSimulator(bool verbose=true) : BroadcastSimulator<BroadcastPolicy>(verbose) {
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

		void sendNewMessage(int proc);

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
void TotalOrderBroadcastSimulator<BroadcastPolicy>::sendNewMessage(int proc) {
	//if(this->waitingForAcks[proc].empty() && this->isSending[proc].empty())
		BroadcastSimulator<BroadcastPolicy>::sendNewMessage(proc);
}

template <class BroadcastPolicy>
bool TotalOrderBroadcastSimulator<BroadcastPolicy>::send(int sender, int receiver, Message message) {
	if(sentMsg[sender]) {
		return false;
	}
	else {
		message.sender = sender;

		if(message.content == 'A') {
			for(int i=0; i<this->numProcs; i++) {
				if(i != sender) {
					this->procBuffer[(this->currentBuffer+1)%2][i].push(message);
					this->log.storeSend(message, i);
				}
			}
			this->procClock[sender]++;

			if(this->verbose) {
				cout << "Process " << sender << " broadcasted 'ack " << message.getId();
				cout << "' [time " << message.time << "]" << endl; 
			}
		}
		else {
			message.time = this->procClock[sender];
			this->procBuffer[(this->currentBuffer+1)%2][receiver].push(message);
			this->log.storeSend(message, receiver);
			this->procClock[sender]++;

			if((int)this->msgDestinations[message.getId()].size() == this->numProcs - 1) {
				this->firstTimeSent[message.getId()] = this->round;
				// The sender includes the message in its waiting list
				this->updateWaitingForAcksList(sender, message);
				// The first time we increment this to signalize the sender
				// also has to deliver this message
				this->procsToReceive[message.getId()]++; 
			}
			this->procsToReceive[message.getId()]++; 

			for(int i=0; i<this->numProcs; i++) {
				if(remainingAcks[i][message.getId()] == 0) {
					// Proc receives its own ack 'for free', that's why the -1
					remainingAcks[i][message.getId()] = this->numProcs - 1;
				}
			}

			if(this->verbose && (sender != receiver))
				cout << "Process " << sender << " sent '" << message.getId() << "' to process " << receiver << " [time " << message.time << "]" << endl;
		}

		sentMsg[sender] = true;
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
		this->log.storeReceive(m, receiver);
		this->procClock[receiver] = this->procClock[receiver] < m.time ? m.time + 1 : this->procClock[receiver];

		if(m.content == 'A') {
			if(this->verbose)
				cout << "Process " << receiver << " received 'ack " << m.getId() << "' from " << m.sender << " [time " << m.time << "]" << endl; 
			this->procBuffer[this->currentBuffer][receiver].pop();
			remainingAcks[receiver][m.getId()]--;

			updateWaitingForAcksList(receiver, m);
			Message msgToReturn = getReadyMessage(receiver);
			this->log.storeDeliver(msgToReturn, receiver);
			return msgToReturn;
		}
		else {
			this->procBuffer[this->currentBuffer][receiver].pop(); 
			remainingAcks[receiver][m.getId()]--; 
			
			// Process that receives a message (even if it hasn't been delivered yet)
			// gains the right to help broadcasting it to other processes (this makes
			// it possible to implement the Tree and Pipeline protocols)
			/*
			Message recSendingMsg;
			if(!this->isSending[receiver].empty())
				recSendingMsg = this->isSending[receiver].front();
			if(!this->isSending[receiver].empty() && ((m.time < recSendingMsg.time)
				|| ((m.time == recSendingMsg.time)
				&& (m.getCreator()<recSendingMsg.getCreator())))) // if the message must be
				this->isSending[receiver].push_front(m);  // received before, help the
			else                                         // process finishing the
				this->isSending[receiver].push_back(m);  // broadcast with higher priority
			*/
			this->isSending[receiver].push(m);
		 
			//* Multicasts the ack right now
			Message mAck = m;
			mAck.content = 'A';
			send(receiver, -1, mAck);
			// */
			/* Multicasts the ack later...
			Message mAck = m;
			mAck.content = 'A';
			this->isSending[receiver].push(mAck);
			// */

			updateWaitingForAcksList(receiver, m); 
			Message msgToReturn = getReadyMessage(receiver); 
			this->log.storeDeliver(msgToReturn, receiver);
			return msgToReturn;
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
			if(msg.content != 'A') {
				//cout << "Updating content for " << msg.getId() << " with " << msg.content << endl;
				m.content = msg.content;
			}
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
		if(this->verbose)
			cout << "Process " << receiver << " has a message(" << mAcks.getId() << ") waiting for " << remainingAcks[receiver][mAcks.getId()] << " acks" << endl;
		if(remainingAcks[receiver][mAcks.getId()] == 0) {
			if(this->verbose)
				cout << "Process " << receiver << " received '" << mAcks.getId() << "'(" << mAcks.content << ")" << endl;
			waitingForAcks[receiver].pop();
			this->procClock[receiver] = this->procClock[receiver] < mAcks.time ? mAcks.time + 1 : this->procClock[receiver];

			this->procsToReceive[mAcks.getId()]--;
			if((this->procsToReceive[mAcks.getId()]==0) && (this->msgDestinations[mAcks.getId()].size()==0)) {
				this->msgLatencies[mAcks.getId()] = this->round - this->firstTimeSent[mAcks.getId()];
				if(this->verbose)
					cout << ">> '" << mAcks.getId() << "' was broadcasted! Latency was " << this->msgLatencies[mAcks.getId()] << "." << endl;
			}

			return mAcks;
		}
	}
	return Message();
}

#endif
