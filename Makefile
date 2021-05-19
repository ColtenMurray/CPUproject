#CS3339 Project 4 Branch Prediction Makefile
CFLAGS=-O3 -std=c++11

ifdef BPRED_SIZE
	CFLAGS+=-DBPRED_SIZE=$(BPRED_SIZE)
endif

simulator: ALU.o CPU.o Memory.o Stats.o Simulator.o BranchPred.o
	g++ $(CFLAGS) ALU.o CPU.o Memory.o Stats.o Simulator.o BranchPred.o -o simulator

ALU.o: Debug.h ALU.h ALU.cpp
	g++ $(CFLAGS) -c ALU.cpp
	
BranchPred.o: Debug.h BranchPred.h BranchPred.cpp
	g++ $(CFLAGS) -c BranchPred.cpp

CPU.o: Debug.h ALU.h Memory.h Stats.h CPU.h  BranchPred.h CPU.cpp
	g++ $(CFLAGS) -c CPU.cpp

Memory.o: Debug.h Memory.h Memory.cpp
	g++ $(CFLAGS) -c Memory.cpp

Stats.o: Debug.h Stats.h Stats.cpp
	g++ $(CFLAGS) -c Stats.cpp

Simulator.o: Debug.h CPU.h Memory.h Stats.h BranchPred.h Simulator.cpp
	g++ $(CFLAGS) -c Simulator.cpp

.PHONY: clean
clean:
	rm -f ALU.o BranchPred.o CPU.o Memory.o Stats.o Simulator.o simulator
