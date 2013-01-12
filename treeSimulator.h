#ifndef TREE_SIMULATOR_H
#define TREE_SIMULATOR_H

#include "broadcastSimulator.h"

class TreeSimulator : public BroadcastSimulator {
	public:
		TreeSimulator() : BroadcastSimulator() {
		}
	protected:
		void sendNewMessage(int proc);
		bool broadcast();
};

#endif
