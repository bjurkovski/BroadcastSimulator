CC = g++

sim: main.cpp broadcastSimulator.hpp totalOrderBroadcastSimulator.hpp basicPolicy.hpp treePolicy.hpp pipelinePolicy.hpp simulationLog.o message.h
	$(CC) $^ -o $@ -Wall

%.o: %.cpp %.h
	$(CC) $< -c -o $@ -Wall
