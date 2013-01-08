#ifndef TREE_SIMULATOR_H
#define TREE_SIMULATOR_H

#include "broadcastSimulator.h"

class TreeSimulator : public BroadcastSimulator {
	public:
		TreeSimulator() : BroadcastSimulator() {
		}
	protected:
		bool broadcast(int round);
};

#endif
