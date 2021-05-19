/******************************
 * Name:  <delete this and write your first and last name and net ID>
 * CS 3339 - Spring 2019
 ******************************/
#include "Stats.h"

Stats::Stats() {
  cycles = PIPESTAGES - 1; // pipeline startup cost
  flushes = 0;
  bubbles = 0;

  memops = 0;
  branches = 0;
  taken = 0;

  for(int i = IF1; i < PIPESTAGES; i++) {
    resultReg[i] = -1;
  }
}

void Stats::clock() {
  cycles++;

  // advance all pipeline flip flops
  for(int i = WB; i > IF1; i--) {
    resultReg[i] = resultReg[i-1];
  }
  // inject a no-op into IF1
  resultReg[IF1] = -1;
}

void Stats::registerSrc(int r)
{
    for( int i = EXE1; i < WB; i++)
    {
        if( r == resultReg[i])
        {
            bubble();
        }
    }
}

void Stats::registerDest(int r)
{
    resultReg[ID] = r;
}

void Stats::flush(int count) { // count == how many ops to flush
    flushes++;
    for(int i = 0; i < count; i++)
    {
        for(int i = WB; i > IF1; i--)
        {
        resultReg[i] = resultReg[i-1];
        }
        resultReg[IF1] = -1;
    }
}

void Stats::bubble()
{
    bubbles++;
    for(int i = WB; i > EXE1; i--)
        {
        resultReg[i] = resultReg[i-1];
        }
        resultReg[EXE1] = -1;
}

