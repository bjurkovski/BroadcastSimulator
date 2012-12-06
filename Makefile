CC = g++

sim: main.cpp basicSimulator.o treeSimulator.o pipelineSimulator.o
	$(CC) $^ -o $@ -Wall

%.o: %.cpp %.h
	$(CC) $< -c -o $@ -Wall
