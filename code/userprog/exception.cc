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
#include "system.h" //Nunca poner system debajo de syscall!!
#include "syscall.h"
#include "addrspace.h"
#include <string>
#include <sstream>
#include <vector>

int getArg(int num);
bool readString(int virtAddr,std::string& str);
bool readBuffer(int virtAddr, int size, char* buf);
bool writeBuffer(char* buf, int size, int virtAddr);
int tokenize(std::string str, char **&tokens);
void deleteTokens(int argc, char **tokens);
void SyscallHandler(int type);

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

  switch(which){

  case(SyscallException):      // A program executed a system call.
    SyscallHandler(machine->ReadRegister(2));
    break;
  case(NoException): // Everything ok!
    break;
  case(PageFaultException):    // No valid translation found
    printf("Unexpected Exception type: %d",which);
    ASSERT(FALSE);
    break;
  case(ReadOnlyException):     // Write attempted to page marked "read-only"
    printf("Unexpected Exception type: %d",which);
    ASSERT(FALSE);
    break;
  case(BusErrorException):     // Translation resulted in an invalid physical address
    printf("Unexpected Exception type: %d",which);
    ASSERT(FALSE);
    break;
  case(AddressErrorException): // Unaligned reference or one that
                					    // was beyond the end of the
		                			    // address space
    printf("Unexpected Exception type: %d",which);
    ASSERT(FALSE);
    break;
  case(OverflowException):     // Integer overflow in add or sub.
    printf("Unexpected Exception type: %d",which);
    ASSERT(FALSE);
    break;
  case(IllegalInstrException): // Unimplemented or reserved instr.
    printf("Unexpected Exception type: %d",which);
    ASSERT(FALSE);
    break;
  default:
    printf("Unexpected Exception type: %d",which);
    ASSERT(FALSE);
  }

}

void SyscallHandler(int type){

  int res;
  std::string arg;
  char* buf = NULL;
  char **argv;
  int argc;

  switch(type){
  case SC_Halt:
    syscallHalt();
    break;

  case SC_Exit:
    machine->WriteRegister(2,getArg(1));
    syscallExit(getArg(1));
    break;

  case SC_Exec:
    res = -1;
    if (readString(getArg(1), arg)){
      argc = tokenize(arg, argv);
      res = syscallExec(argc, argv);
      // deleteTokens(argc, argv); //We free this from the new thread
    }
    machine->WriteRegister(2,res);
    break;

  case SC_Join:
    res = syscallJoin((SpaceId)getArg(1));
    machine->WriteRegister(2,res);
    break;

  case SC_Create:
    if (readString(getArg(1), arg))
      syscallCreate(arg.c_str());
    break;

  case SC_Open:
    if (readString(getArg(1), arg))
      res = syscallOpen(arg.c_str());
    else 
      res = -1;
    machine->WriteRegister(2,res);
    break;

  case SC_Read:
    if (getArg(2) <= 0){
      machine->WriteRegister(2,-1);
      break;
    }
    buf = new char[getArg(2)];
    res = (int) syscallRead(buf , getArg(2),(OpenFileId) getArg(3));
    if (!writeBuffer(buf,res,getArg(1)))
      res = -1;
    //Set return value
    delete buf;
    machine->WriteRegister(2,res);
    break;

  case SC_Write:
    if (getArg(2) <= 0)
      break;
    buf = new char[getArg(2)];
    if (readBuffer(getArg(1), getArg(2), buf)){
      syscallWrite(buf,getArg(2),(OpenFileId) getArg(3));
    }
    delete buf;
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





int getArg(int num){
  return  machine->ReadRegister(3+num);
}

bool readString(int virtAddr,std::string& str){
  str = std::string();

  int c = 1;
  bool res = true;

  while (res  && c != '\0'){
    res = machine->ReadMem(virtAddr,1,&c);
    virtAddr++;
    str += (char)c;
  }

  return res;
}

bool readBuffer(int virtAddr, int size, char* buf){
  int rc;
  for (int i = 0; i < size ; i++,virtAddr++){
    if (!machine->ReadMem(virtAddr, 1, &rc))
      return false;
    buf[i] = rc;
  }
  return true;
}


bool writeBuffer(char* buf, int size, int virtAddr){
  for (int i = 0; i < size ; i++,virtAddr++){
    if (!machine->WriteMem(virtAddr, 1, buf[i]))
      return false;
  }
  return true;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
      if (!item.empty() && item[0] != '\0')
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}


// std::vector<std::string> tokenize(std::string str){
//   return split(str, ' ');
// }

int tokenize(std::string str, char **& tokens){
  std::vector<std::string> tks = split(str, ' ');
  tokens = new char*[tks.size()];
  for (int i = 0; i < tks.size(); i++){
    tokens[i] = new char[tks[i].size()+1];
    strcpy(tokens[i], tks[i].c_str());
  }
  return tks.size();
}

