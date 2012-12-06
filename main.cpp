#include "basicSimulator.h"
#include "treeSimulator.h"
#include "pipelineSimulator.h"

#define SimType TreeSimulator
int main() {
	SimType sim("simulation.cfg");
	sim.run();
	return 0;
}
