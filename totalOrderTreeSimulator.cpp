#include "totalOrderTreeSimulator.h"
#include <queue>
#include <iostream>

using namespace std;

bool TO_TreeSimulator::send(int sender, int receiver, Message message) {
	message.sender = sender;
	message.time = procClock[sender];
	procBuffer[(currentBuffer+1)%2][receiver].push(message);
	procClock[sender]++;

	for(int i=0; i<numProcs; i++)
		remainingAcks[i][message.getId()] = numProcs - 1;

	if(sender != receiver)
		cout << "Process " << sender << " sent '" << message.getId() << "' to process " << receiver << endl;
	return true;
}

Message TO_TreeSimulator::receive(int receiver) {
	if(procBuffer[currentBuffer][receiver].empty()) {
		return Message();
	}
	else {
		Message m = procBuffer[currentBuffer][receiver].top();
		if(m.content == 'A') {
			cout << "Process " << receiver << " received 'Ack_" << m.getId() << "'" << endl; //
			remainingAcks[receiver][m.getId()]--;
			return Message();
		}
		else if(remainingAcks[receiver][m.getId()] == 0) {
			procBuffer[currentBuffer][receiver].pop();
			procClock[receiver] = max(procClock[receiver], m.time) + 1;
			cout << "Process " << receiver << " received '" << m.getId() << "'" << endl;
			return m;
		}
		else {	
			cout << "Process " << receiver << " broadcasted an ack" << endl; //
			m.content = 'A';
			for(int i=0; i<numProcs; i++)
				send(receiver, i, m);
			return Message();
		}
	}
}
