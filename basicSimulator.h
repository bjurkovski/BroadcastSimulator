#ifndef BASIC_SIMULATOR_H
#define BASIC_SIMULATOR_H

#include "broadcastSimulator.h"

class BasicSimulator : public BroadcastSimulator {
	public:
		BasicSimulator() {
		}
	protected:
		bool broadcast(int round);
};

#endif
