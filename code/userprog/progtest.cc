// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------

void test(AddrSpace* space){

  int i;

  for (i = 0; i < PageSize; i++)
    machine->mainMemory[i] = i;

  machine->tlb[0].physicalPage = 0;
  machine->tlb[0].valid = true;
  machine->tlb[0].dirty = true;
  machine->tlb[0].virtualPage = 0;

  space->savePageTableEntry(machine->tlb[0],0);

  space->swap->swapOut(0);

   for (i = 0; i < PageSize; i++)
     machine->mainMemory[i] = 0;



  space->swap->swapIn(0,0);


   printf("\n");

   for (i = 0; i < PageSize; i++)
     printf("%d ",machine->mainMemory[i]);

   printf("\n");

   exit(-1);

}


void
StartProcess(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    space = new AddrSpace();    
    if (!space->Initialize(executable)){
      std::cout << "Error initializing address space." << std::endl;
	return;
    }

    currentThread->space = space;

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    space->swap = new Swap((int)currentThread,
			   space->numPages);

    // test(space);

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}
