#include "broadcastSimulator.hpp"
#include "totalOrderBroadcastSimulator.hpp"
#include <cstdio>
#include <cstring>

SimulationLog runSimulator(const char* simType, const char* input) {
	printf("%s Broadcast '%s'\n", simType, input);
	SimulationLog log;
	if(strcmp(simType, "Basic") == 0) {
		//BasicSimulator sim;
		BroadcastSimulator<BasicPolicy> sim;
		sim.initialize(input);
		log = sim.run();
	}
	else if(strcmp(simType, "Tree") == 0) {
		BroadcastSimulator<TreePolicy> sim;
		sim.initialize(input);
		log = sim.run();
	}
	else if(strcmp(simType, "Pipeline") == 0) {
		BroadcastSimulator<PipelinePolicy> sim;
		sim.initialize(input);
		log = sim.run();
	}
	else if(strcmp(simType, "TotalOrderTree") == 0) {
		TotalOrderBroadcastSimulator<TreePolicy> sim;
		sim.initialize(input);
		log = sim.run();
	}
	else if(strcmp(simType, "TotalOrderBasic") == 0) {
		TotalOrderBroadcastSimulator<BasicPolicy> sim;
		sim.initialize(input);
		log = sim.run();
	}
	else if(strcmp(simType, "TotalOrderPipeline") == 0) {
		TotalOrderBroadcastSimulator<PipelinePolicy> sim;
		sim.initialize(input);
		log = sim.run();
	}
	string path = string(simType) + "_" + string(input) + ".msc";
	log.dumpMsc(path);
	return log;
}

void benchmark() {
	string simTypes[] = {"TotalOrderBasic", "TotalOrderTree", "TotalOrderPipeline"};
	string benchs[] = {"input1.cfg", "input2.cfg", "input3.cfg", "input4.cfg", "input5.cfg"};
	double lat[3][5], tp[3][5], stdDevLat[3][5];
	int numBenchs = 5;

	for(int i=0; i<3; i++) {
		for(int j=0; j<numBenchs; j++) {
			SimulationLog log = runSimulator(simTypes[i].c_str(), benchs[j].c_str());
			lat[i][j] = log.getAvgLatency();
			stdDevLat[i][j] = log.getStdDevLatency();
			tp[i][j] = log.getAvgThroughput();
		}
	}
	
	for(int i=0; i<3; i++) {
		printf("%s\n", simTypes[i].c_str());
		double avgLat = 0, avgTp = 0, avgStdDevLat = 0;
		for(int j=0; j<numBenchs; j++) {
			printf("%s: [lat = %lf +- %lf, tp = %lf]\n", benchs[j].c_str(), lat[i][j], stdDevLat[i][j], tp[i][j]);
			avgLat += lat[i][j];
			avgStdDevLat += stdDevLat[i][j];
			avgTp += tp[i][j];
		}
		printf("Avg Lat: %f [AvgStdDev: %lf]\n", avgLat/numBenchs, avgStdDevLat/numBenchs);
		printf("Avg Tp: %f\n", avgTp/numBenchs);
	}
}

int main(int argc, char* argv[]) {
	if((argc > 1) && (strcmp(argv[1], "-b") == 0)) {
		benchmark();
	}
	else {
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
	}

	return 0;
}
