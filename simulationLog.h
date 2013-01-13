#ifndef SIMULATION_LOG_HPP
#define SIMULATION_LOG_HPP

#include <vector>
#include <utility>
#include <string>
#include "message.h"

class ProcRoundLog {
	public:
		ProcRoundLog() : msgSent(), msgReceived() { }
		// A list containing all pairs of messages sent
		// by this process and their destination
		std::vector< std::pair<Message, int> > msgSent;
		Message msgReceived;
};

class SimulationLog {
	public:
		SimulationLog() : roundLog() { }
		void initialize(int numProcs);
		void newRound();
		void storeSend(Message m, int receiver);
		void storeReceive(Message m, int receiver);
		void dumpMsc(std::string path);

	protected:
		int numProcs;
		std::vector< std::vector<ProcRoundLog> > roundLog;
};

#endif