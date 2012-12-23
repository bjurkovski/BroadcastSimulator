CC = g++

sim: main.cpp broadcastSimulator.o basicSimulator.o treeSimulator.o pipelineSimulator.o totalOrderTreeSimulator.o
	$(CC) $^ -o $@ -Wall

%.o: %.cpp %.h
	$(CC) $< -c -o $@ -Wall
