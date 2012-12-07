#include "basicSimulator.h"
#include "treeSimulator.h"
#include "pipelineSimulator.h"
#include "totalOrderTreeSimulator.h"

//#define SimType TO_TreeSimulator
//#define SimType BasicSimulator
#define SimType TreeSimulator
//#define SimType PipelineSimulator
int main() {
	SimType sim("simulation.cfg");
	sim.run();
	return 0;
}
