#include "broadcastSimulator.hpp"
#include "totalOrderBroadcastSimulator.hpp"
#include <cstdio>
#include <cstring>

SimulationLog runSimulator(const char* simType, const char* input, bool verbose=true) {
	printf("%s Broadcast '%s'\n", simType, input);
	SimulationLog log;
	if(strcmp(simType, "Basic") == 0) {
		//BasicSimulator sim;
		BroadcastSimulator<BasicPolicy> sim(verbose);
		sim.initialize(input);
		log = sim.run();
	}
	else if(strcmp(simType, "Tree") == 0) {
		BroadcastSimulator<TreePolicy> sim(verbose);
		sim.initialize(input);
		log = sim.run();
	}
	else if(strcmp(simType, "Pipeline") == 0) {
		BroadcastSimulator<PipelinePolicy> sim(verbose);
		sim.initialize(input);
		log = sim.run();
	}
	else if(strcmp(simType, "TotalOrderTree") == 0) {
		TotalOrderBroadcastSimulator<TreePolicy> sim(verbose);
		sim.initialize(input);
		log = sim.run();
	}
	else if(strcmp(simType, "TotalOrderBasic") == 0) {
		TotalOrderBroadcastSimulator<BasicPolicy> sim(verbose);
		sim.initialize(input);
		log = sim.run();
	}
	else if(strcmp(simType, "TotalOrderPipeline") == 0) {
		TotalOrderBroadcastSimulator<PipelinePolicy> sim(verbose);
		sim.initialize(input);
		log = sim.run();
	}
	string path = string(simType) + "_" + string(input) + ".msc";
	log.dumpMsc(path);
	return log;
}

#define NUM_BENCHS 19
void benchmark() {
	string simTypes[] = {"TotalOrderBasic", "TotalOrderTree", "TotalOrderPipeline"};
	string benchs[] = {"input1.cfg", "input2.cfg", "input3.cfg", "input4.cfg", "input5.cfg",
					   "input6.cfg", "input7.cfg", "input8.cfg", "input9.cfg", "input10.cfg",
					   "input11.cfg", "input12.cfg", "input13.cfg",
					   "in_20_singleMsg.cfg", "in_20_singleSender.cfg", "in_20_nSenders.cfg",
					   "in_50_singleMsg.cfg", "in_50_singleSender.cfg", "in_50_nSenders.cfg"};
	double lat[3][NUM_BENCHS], tp[3][NUM_BENCHS], stdDevLat[3][NUM_BENCHS];

	for(int i=0; i<3; i++) {
		for(int j=0; j<NUM_BENCHS; j++) {
			SimulationLog log = runSimulator(simTypes[i].c_str(), benchs[j].c_str(), false);
			lat[i][j] = log.getAvgLatency();
			stdDevLat[i][j] = log.getStdDevLatency();
			tp[i][j] = log.getAvgThroughput();
		}
	}
	
	for(int i=0; i<3; i++) {
		printf("%s\n", simTypes[i].c_str());
		double avgLat = 0, avgTp = 0, avgStdDevLat = 0;
		for(int j=0; j<NUM_BENCHS; j++) {
			printf("%s: [lat = %lf +- %lf, tp = %lf]\n", benchs[j].c_str(), lat[i][j], stdDevLat[i][j], tp[i][j]);
			avgLat += lat[i][j];
			avgStdDevLat += stdDevLat[i][j];
			avgTp += tp[i][j];
		}
		printf("Avg Lat: %f [AvgStdDev: %lf]\n", avgLat/NUM_BENCHS, avgStdDevLat/NUM_BENCHS);
		printf("Avg Tp: %f\n", avgTp/NUM_BENCHS);
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
