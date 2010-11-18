// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

int getArg(int num);
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
  int type = machine->ReadRegister(2);
  int res;

  if (which == SyscallException){
    switch(type){
    case SC_Halt:
      syscallHalt();
      break;
    case SC_Exit:
      syscallExit(getArg(1));
      break;
    case SC_Exec:
      break;
    case SC_Join:
      break;
    case SC_Create:
      syscallCreate((char*) getArg(1));
      break;
    case SC_Open:
      res = (int) syscallOpen((char*) getArg(1));
      //Set return value
      machine->WriteRegister(2,res);
      break;
    case SC_Read:
      res = (int) syscallRead((char*) getArg(1),getArg(2),getArg(3));
      //Set return value
      machine->WriteRegister(2,res);
      break;
    case SC_Write:
      syscallWrite((char*) getArg(1),getArg(2),(OpenFileId) getArg(3));
      break;
    case SC_Close:
      syscallClose((OpenFileId) getArg(1));
      break;
    case SC_Fork:
      break;
    case SC_Yield:
      syscallYield();
      break;
    default:
      printf("Unexpected syscall code %d\n", type);
      ASSERT(FALSE);
    }
      

    //Advance Program counter
    // int PrevPCRegVal = machine->ReadRegister(PrevPCReg);
    int PCRegVal = machine->ReadRegister(PCReg);
    int NextPCRegVal = machine->ReadRegister(NextPCReg);
    
    machine->WriteRegister(PrevPCReg, PCRegVal);
    machine->WriteRegister(PCReg, NextPCRegVal);
    machine->WriteRegister(NextPCReg, NextPCRegVal+4);
  }

  else {
    printf("Unexpected user mode exception %d %d\n", which, type);
    ASSERT(FALSE);
  }
}

int getArg(int num){
  return  machine->ReadRegister(3+num);
}
