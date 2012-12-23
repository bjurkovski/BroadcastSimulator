#include "broadcastSimulator.h"
#include "basicSimulator.h"
#include "treeSimulator.h"
#include "pipelineSimulator.h"
#include "totalOrderTreeSimulator.h"
#include <cstdio>
#include <cstring>

void runSimulator(char* simType, char* input) {
	printf("%s Broadcast '%s'\n", simType, input);
	if(strcmp(simType, "Basic") == 0) {
		BasicSimulator sim;
		sim.initialize(input);
		sim.run();
	}
	else if(strcmp(simType, "Tree") == 0) {
		TreeSimulator sim;
		sim.initialize(input);
		sim.run();
	}
	else if(strcmp(simType, "Pipeline") == 0) {
		PipelineSimulator sim;
		sim.initialize(input);
		sim.run();
	}
	else if(strcmp(simType, "TOTree") == 0) {
		TO_TreeSimulator sim;
		sim.initialize(input);
		sim.run();
	}
}

int main() {
	FILE* cfg = fopen("simulation.cfg", "r");
	if(cfg) {
		int numTests;
		char simType[30], input[30];
		fscanf(cfg, "%d", &numTests);
		for(int i=0; i<numTests; i++) {
			fscanf(cfg, "%s %s", simType, input);
			runSimulator(simType, input);
		}
	}
	else {
		printf("Couldn't open 'simulation.cfg' file. Please verify the file exits in your current directory.\n");
	}
	fclose(cfg);

	return 0;
}
