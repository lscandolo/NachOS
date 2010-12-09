#include "synchconsole.h"

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

void readCallback(int arg) {((SynchConsole*)arg)->readAvailV();}
void writeCallback(int arg){((SynchConsole*)arg)->writeDoneV();}

SynchConsole::SynchConsole(char* in, char* out){

  console   = new Console(in,out,
			  readCallback,
			  writeCallback,
			  (int)this);

  readAvail = new Semaphore(std::string("read_semaphore_SynchConsole"),0);
  writeDone = new Semaphore(std::string("write_semaphore_SynchConsole"),0);
  readLock      = new Lock(std::string("readLock_SynchConsole"));
  writeLock      = new Lock(std::string("writeLock_SynchConsole"));
}

SynchConsole::~SynchConsole(){

  delete readLock;
  delete writeLock;
  delete writeDone;
  delete readAvail;
  delete console;
}

void SynchConsole::put(char c){
  writeLock->Acquire();
  console->PutChar(c);
  writeDone->P();
  writeLock->Release();
}

const char SynchConsole::get(){
  readLock->Acquire();
  readAvail->P();
  char c = console->GetChar();
  readLock->Release();
  return c;
}
