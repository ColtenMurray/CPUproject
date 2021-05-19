/******************************
 * CS 3339 - Spring 2019
 * Copyright 2019, all rights reserved
 * Updated by Lee B. Hinkle based on prior work by Martin Burtscher and Molly O'Neil
 ******************************/
#ifndef __BRANCH_PRED_H
#define __BRANCH_PRED_H

#include <cstdint>
#include "Debug.h"
using namespace std;

#ifndef BPRED_SIZE
#define BPRED_SIZE 64
#endif

class BranchPred {
  private:
    int pred[BPRED_SIZE];  // stores the 2-bit saturating counter values
    uint32_t btb[BPRED_SIZE];  // stores the target address of the branch

    int predictions;
    int pred_takens;
    int mispredictions;
    int mispred_direction;
    int mispred_target;

    int transition(int counter, bool taken);

  public:
    BranchPred();

    bool predict(uint32_t pc, uint32_t &target);
    bool isMispredict(bool predTaken, uint32_t predTarget, bool taken, uint32_t target);
    void update(uint32_t pc, bool taken, uint32_t target);

    void printFinalStats();  // call at end of your CPU.cpp 
};

#endif
