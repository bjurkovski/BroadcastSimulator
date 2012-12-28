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
/* Order test... 
	MessageQueue q;
	Message m1(1, '0'), m2(2, '1'), m3(3, '2'), m4(4, 'A'), m5(5, 'A'), m6(6, 'A');
	m1.time = 1; m1.sender = 1;
	m2.time = 2; m2.sender = 1;
	m3.time = 2; m3.sender = 0;
	m4.time = 4; m4.sender = 4;
	m5.time = 5; m5.sender = 4;
	m6.time = 5; m6.sender = 3;
	q.push(m1);
	q.push(m2);
	q.push(m3);
	q.push(m4);
	q.push(m5);
	q.push(m6);
	while(!q.empty()) {
		Message m = q.top();
		printf("id: %d, time: %d, sender: %d\n", m.getId(), m.time, m.sender);
		q.pop();
	}
	return 0;
// */

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
