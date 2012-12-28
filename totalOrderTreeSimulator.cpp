#include "totalOrderTreeSimulator.h"
#include <queue>
#include <iostream>

using namespace std;

bool TO_TreeSimulator::hasMessageToReceive() {
	if(TreeSimulator::hasMessageToReceive())
		return true;

	for(int i=0; i<numProcs; i++)
		if(!waitingForAcks[i].empty())
			return true;

	return false;
}

bool TO_TreeSimulator::send(int sender, int receiver, Message message) {
	if(sentMsg[sender]) {
		return false;
	}
	else {
		message.sender = sender;

		if(message.content == 'A') {
			procBuffer[(currentBuffer+1)%2][receiver].push(message);
		}
		else {
			message.time = procClock[sender];
			procBuffer[(currentBuffer+1)%2][receiver].push(message);
			procClock[sender]++;
			for(int i=0; i<numProcs; i++) {
				//if(i == sender)
				if(remainingAcks[i][message.getId()] == 0) {
					remainingAcks[i][message.getId()] = numProcs - 1; // Proc receives its own ack 'for free'
					//cout << "remainingAcks[" << i << "][" << message.getId() << "] == " << remainingAcks[i][message.getId()] << endl;
				}
				//else
				//	remainingAcks[i][message.getId()] = numProcs - 2; // Proc also receives the message sender's ack 'for free'
			}

			if(sender != receiver)
				cout << "Process " << sender << " sent '" << message.getId() << "' to process " << receiver << " [time " << message.time << "]" << endl;
		}

		return true;
	}
}

Message TO_TreeSimulator::receive(int receiver) {
	if(!waitingForAcks[receiver].empty()) {
		Message mAcks = waitingForAcks[receiver].top();
		cout << "Process " << receiver << " has a message(" << mAcks.getId() << ") waiting for " << remainingAcks[receiver][mAcks.getId()] << " acks" << endl;
		if(remainingAcks[receiver][mAcks.getId()] == 0) {
			cout << "Process " << receiver << " received '" << mAcks.getId() << "'" << endl;
			waitingForAcks[receiver].pop();
			procClock[receiver] = procClock[receiver] < mAcks.time ? mAcks.time + 1 : procClock[receiver];
			return mAcks;
		}
	}

	if(procBuffer[currentBuffer][receiver].empty()) {
		//cout << "Process " << receiver << " has an empty buffer..." << endl;
		//cout << "Process " << receiver << " empty(buf[next]) = " << procBuffer[(currentBuffer+1)%2][receiver].empty() << endl;
		return Message();
	}
	else {
		sentMsg[receiver] = false;
		Message m = procBuffer[currentBuffer][receiver].top();
		procClock[receiver] = procClock[receiver] < m.time ? m.time + 1 : procClock[receiver];

		if(m.content == 'A') {
			cout << "Process " << receiver << " received 'ack " << m.getId() << "' from " << m.sender << " [time " << m.time << "]" << endl; //
			procBuffer[currentBuffer][receiver].pop();
			remainingAcks[receiver][m.getId()]--;

			MessageQueue bkp;
			bool knowsMessage = false;
			while(!waitingForAcks[receiver].empty()) {
				Message msg = waitingForAcks[receiver].top();
				waitingForAcks[receiver].pop();
				if(m.getId() == msg.getId()) {
					knowsMessage = true;
					msg.time = min(m.time, msg.time);
				}
				bkp.push(msg);
			}
			if(!knowsMessage) bkp.push(m);
			waitingForAcks[receiver] = bkp;
			//cout << "remainingAcks[" << receiver << "][" << m.getId() << "] == " << remainingAcks[receiver][m.getId()] << endl;
			return Message();
		}
		else {
			procBuffer[currentBuffer][receiver].pop(); //
			remainingAcks[receiver][m.getId()]--; //
			//m.time = procClock[receiver]; //

			cout << "Process " << receiver << " broadcasted 'ack " << m.getId() << "' [time " << m.time << "]" << endl; //
			m.content = 'A';
			for(int i=0; i<numProcs; i++) {
				if(i != receiver) send(receiver, i, m);
			}
			procClock[receiver]++; //
			sentMsg[receiver] = true;

			if((remainingAcks[receiver][m.getId()] == 0)
				&& (waitingForAcks[receiver].empty() || (m < waitingForAcks[receiver].top()))
			  ){
				cout << "Process " << receiver << " received '" << m.getId() << "'" << endl;
				MessageQueue bkp;
				while(!waitingForAcks[receiver].empty()) {
					Message msg = waitingForAcks[receiver].top();
					waitingForAcks[receiver].pop();
					if(m.getId() != msg.getId()) {
						bkp.push(msg);
					}
				}
				waitingForAcks[receiver] = bkp;
				return m;
			}
			else {
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
				waitingForAcks[receiver].push(m); //
				return Message();
			}
		}
	}
}
