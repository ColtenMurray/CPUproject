/******************************
 * Submitted by: Colten Murray - clm335
 * CS 3339 - Spring 2019
 * Project 4 Branch Predictor
 * Copyright 2019, all rights reserved
 * Updated by Lee B. Hinkle based on prior work by Martin Burtscher and Molly O'Neil
 ******************************/
#include <iostream>
#include <iomanip>
#include <cstdint>
#include "BranchPred.h"
using namespace std;

BranchPred::BranchPred() {
  cout << "Branch Predictor Entries: " << BPRED_SIZE << endl;
  /* TODO Initialize your private variables and pred[] array here*/
  predictions = 0;
  pred_takens = 0;
  mispredictions = 0;
  mispred_direction = 0;
  mispred_target = 0;

  for (int i = 0; i < BPRED_SIZE; i++)
  {
      pred[i] = 0;
  }
}

bool BranchPred::predict(uint32_t pc, uint32_t &target) {
  /* target value updated by reference to match the btb entry
   * returns true if prediction is taken, false otherwise
   * increments predictions and if necessary pred_takens count
   */
  /* TODO implement functionality here, leave the Debug line below right before the return */
  bool predTaken = false;
  int index = (pc >> 2) % BPRED_SIZE;

  target = btb[index];
  predictions++;
  if(pred[index] >= 2)
  {
      predTaken = true;
  }
  D(cout << endl << "    BPRED: bne/beq@addr " << hex << setw(8) << pc << "pred[] value = " << pred[index] << " predTaken is " << boolalpha << (bool)predTaken; (predTaken==0)? cout << " not taken":cout << " taken target addr " << hex << setw(8) << target;)

  return predTaken; //uncomment this line once you've added the code to calculate predTaken
}

bool BranchPred::isMispredict(bool predTaken, uint32_t predTarget, bool taken, uint32_t target) {
  /* implement a function which will return:
   *  false if prediction is correct, both predTaken and predTarget match actual values
   *  true if prediction is incorrect
   *  also updates mispred_direction, mispred_target, and mispredictions counts
   */
  /* TODO implement functionality here */

  if( predTaken == taken && predTarget == target)
  {
      return false;
  }
  else
  {
      if(predTaken != taken)
      {
          mispred_direction++;
      }
      if(predTarget != target)
      {
          mispred_target++;
      }
      mispredictions++;
      return true;
  }

}

void BranchPred::update(uint32_t pc, bool taken, uint32_t target) {
  /* pred counter value should be updated and
   * if branch is taken, the target address also gets updated
   */
  /* TODO implement functionality here */

  int index = (pc >> 2) % BPRED_SIZE;
  pred[index + 1] = transition(pred[index], taken);
  if(taken == true)
  {
      btb[index] = target;
  }
}

int BranchPred::transition(int counter, bool taken) {
  /* This updates the 2-bit saturating counter values
   * You will need to use it, but shouldn't have to modify.
   */
  int transition;
  switch(counter) {
    case 0: transition = (taken ? 1 : 0);
            break;
    case 1: transition = (taken ? 2 : 0);
            break;
    case 2: transition = (taken ? 3 : 1);
            break;
    case 3: transition = (taken ? 3 : 2);
            break;
    default: cerr << "ERROR: 2-bit saturating counter FSM in illegal state" << endl;
  }
  return transition;
}

void BranchPred::printFinalStats() {
  int correct = predictions - mispredictions;
  int not_takens = predictions - pred_takens;

  cout << setprecision(1);
  cout << "Branches predicted: " << predictions << endl;
  cout << "  Pred T: " << pred_takens << " ("
       << (100.0 * pred_takens/predictions) << "%)" << endl;
  cout << "  Pred NT: " << not_takens << endl;
  cout << "Mispredictions: " << mispredictions << " ("
       << (100.0 * mispredictions/predictions) << "%)" << endl;
  cout << "  Mispredicted direction: " << mispred_direction << endl;
  cout << "  Mispredicted target: " << mispred_target << endl;
  cout << "Predictor accuracy: " << (100.0 * correct/predictions) << "%" << endl;
}
