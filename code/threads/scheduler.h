// scheduler.h 
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"

#include <algorithm>
#include <set>

typedef Thread* ThreadP;

class ThreadCompare{
 public:
  bool operator() (const ThreadP& lhs, const ThreadP& rhs) const{
    return (lhs->getPriority()) < (rhs->getPriority());
  }
};

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.

class Scheduler {
  public:
    Scheduler();			// Initialize list of ready threads 
    ~Scheduler();			// De-allocate ready list

    void ReadyToRun(Thread* thread);	// Thread can be dispatched.
    Thread* FindNextToRun();		// Dequeue first thread on the ready 
					// list, if any, and return thread.
    void Run(Thread* nextThread);	// Cause nextThread to start running
    void Print();			// Print contents of ready list
    
  private:

    typedef
      std::multiset<ThreadP, ThreadCompare>
      readyList_t;

    readyList_t readyList; // queue of threads that are ready to run,
                            // but not running
};

#endif // SCHEDULER_H
