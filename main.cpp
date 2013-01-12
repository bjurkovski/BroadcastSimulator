#include "broadcastSimulator.hpp"
#include "totalOrderBroadcastSimulator.hpp"
#include <cstdio>
#include <cstring>

void runSimulator(char* simType, char* input) {
	printf("%s Broadcast '%s'\n", simType, input);
	if(strcmp(simType, "Basic") == 0) {
		//BasicSimulator sim;
		BroadcastSimulator<BasicPolicy> sim;
		sim.initialize(input);
		sim.run();
	}
	else if(strcmp(simType, "Tree") == 0) {
		BroadcastSimulator<TreePolicy> sim;
		sim.initialize(input);
		sim.run();
	}
	else if(strcmp(simType, "Pipeline") == 0) {
		BroadcastSimulator<PipelinePolicy> sim;
		sim.initialize(input);
		sim.run();
	}
	else if(strcmp(simType, "TotalOrderL") == 0) {
		TotalOrderBroadcastSimulator<TreePolicy> sim;
		sim.initialize(input);
		sim.run();
	}
	else if(strcmp(simType, "TotalOrderT") == 0) {
		TotalOrderBroadcastSimulator<BasicPolicy> sim;
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
