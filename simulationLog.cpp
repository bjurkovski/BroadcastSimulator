#include "simulationLog.h"
#include <cstdio>
#include <iostream>

using namespace std;

void SimulationLog::initialize(int numProcs) {
	this->numProcs = numProcs;
	roundLog.clear();
}

void SimulationLog::newRound() {
	roundLog.push_back(vector<ProcRoundLog>());

	int currentRound = roundLog.size()-1;
	for(int i=0; i<numProcs; i++)
		roundLog[currentRound].push_back(ProcRoundLog());
}

void SimulationLog::storeSend(Message m, int receiver) {
	int currentRound = roundLog.size()-1;
	roundLog[currentRound][m.sender].msgSent.push_back(make_pair(m, receiver));
}

void SimulationLog::storeReceive(Message m, int receiver) {
	int currentRound = roundLog.size()-1;
	roundLog[currentRound][receiver].msgReceived = m;
}

void SimulationLog::dumpMsc(string path) {
	FILE* f = fopen(path.c_str(), "w");
	fprintf(f, "msc {\n");
	fprintf(f, "\thscale = \"1\", arcgradient = 0;\n");
	fprintf(f, "\n\t");

	for(int i=0; i<numProcs; i++) {
		fprintf(f, "P%d", i);
		if(i==numProcs-1) fprintf(f, ";\n");
		else fprintf(f, ", ");
	}
	fprintf(f, "\n");

	for(int i=0; i<(int)roundLog.size(); i++) {
		int totalMsgs = 0;
		for(int j=0; j<numProcs; j++)
			totalMsgs += (int)roundLog[i][j].msgSent.size();
		fprintf(f, "\t---  [ label = \"\" ]");
		if(totalMsgs == 0) fprintf(f, ";\n");
		else fprintf(f, ",\n");

		int msgsDumped = 0;
		for(int j=0; j<numProcs; j++) {
			int numMsgs = (int)roundLog[i][j].msgSent.size();
			for(int k=0; k<numMsgs; k++) {
				msgsDumped++;
				Message m = roundLog[i][j].msgSent[k].first;
				int receiver = roundLog[i][j].msgSent[k].second;

				int latency = 0;
				Message recvm = roundLog[i+latency][receiver].msgReceived;
				while((recvm.getId() != m.getId()) || (recvm.content != m.content) || (recvm.sender != m.sender)) {
					latency++;
					if(i+latency == (int)roundLog.size()) break;
					recvm = roundLog[i+latency][receiver].msgReceived;
				}

				char arrowType = '=';
				string label = "msg";
				if(m.content == 'A') {
					arrowType = '>';
					label = "ack";
				}

				fprintf(f, "\tP%d%c>P%d [label=\"%s %d\", arcskip=%d]", m.sender, arrowType, receiver, label.c_str(), m.getId(), latency);
				if(msgsDumped==totalMsgs) fprintf(f, ";\n");
				else fprintf(f, ",\n");
			}
		}
		fprintf(f, "\n");
	}
	//fprintf(f, "\t---  [ label = \"\" ];\n");
	fprintf(f, "}\n");
	fclose(f);
}
