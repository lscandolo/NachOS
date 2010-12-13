// stats.h 
//	Data structures for gathering statistics about Nachos performance.
//
// DO NOT CHANGE -- these stats are maintained by the machine emulation
//
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef STATS_H
#define STATS_H

#include "copyright.h"

// The following class defines the statistics that are to be kept
// about Nachos behavior -- how much time (ticks) elapsed, how
// many user instructions executed, etc.
//
// The fields in this class are public to make it easier to update.

class Statistics {
  public:
    unsigned long long totalTicks;      	// Total time running Nachos
    unsigned long long idleTicks;       	// Time spent idle (no threads to run)
    unsigned long long systemTicks;	 	// Time spent executing system code
    unsigned long long userTicks;       	// Time spent executing user code
				// (this is also equal to # of
				// user instructions executed)

    unsigned long long numDiskReads;		// number of disk read requests
    unsigned long long numDiskWrites;		// number of disk write requests
    unsigned long long numConsoleCharsRead;	// number of characters read from the keyboard
    unsigned long long numConsoleCharsWritten; // number of characters written to the display
    unsigned long long numPageFaults;		// number of virtual memory page faults
    unsigned long long numPacketsSent;		// number of packets sent over the network
    unsigned long long numPacketsRecvd;	// number of packets received over the network

    unsigned long long numMemAccess;    // number of accesses to main memory
    unsigned long long numTLBSoftMiss;    // number of times page is not in tlb
    unsigned long long numTLBHardMiss; // number of times page is not in main memory

    Statistics(); 		// initialize everything to zero

    void Print();		// print collected statistics
};

// Constants used to reflect the relative time an operation would
// take in a real system.  A "tick" is a just a unit of time -- if you 
// like, a microsecond.
//
// Since Nachos kernel code is directly executed, and the time spent
// in the kernel measured by the number of calls to enable interrupts,
// these time constants are none too exact.

#define UserTick 	1	// advance for each user-level instruction 
#define SystemTick 	10 	// advance each time interrupts are enabled
#define RotationTime 	500 	// time disk takes to rotate one sector
#define SeekTime 	500    	// time disk takes to seek past one track
#define ConsoleTime 	100	// time to read or write one character
#define NetworkTime 	100   	// time to send or receive one packet
#define TimerTicks 	100    	// (average) time between timer interrupts
#define UserSlice        5000 //Ticks in user-level before context change

#endif // STATS_H
