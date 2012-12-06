#ifndef PIPELINE_SIMULATOR_H
#define PIPELINE_SIMULATOR_H

#include "basicSimulator.h"

class PipelineSimulator : public BasicSimulator {
	public:
		PipelineSimulator(std::string configFile) : BasicSimulator(configFile) {
		}

		void run();
};

#endif
