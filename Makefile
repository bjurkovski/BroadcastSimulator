CC = g++

#sim: main.cpp broadcastSimulator.o basicSimulator.o treeSimulator.o pipelineSimulator.o totalOrderLSimulator.o message.h
sim: main.cpp broadcastSimulator.hpp totalOrderBroadcastSimulator.hpp basicPolicy.hpp treePolicy.hpp pipelinePolicy.hpp message.h
	$(CC) $^ -o $@ -Wall

%.o: %.cpp %.h
	$(CC) $< -c -o $@ -Wall
