#ifndef PIPELINE_SIMULATOR_H
#define PIPELINE_SIMULATOR_H

#include "broadcastSimulator.h"

class PipelineSimulator : public BroadcastSimulator {
	public:
		PipelineSimulator() : BroadcastSimulator() {
		}
	protected:
		bool broadcast(int round);
};

#endif
