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
  lock      = new Lock(std::string("lock_SynchConsole"));
}

SynchConsole::~SynchConsole(){

  delete lock;
  delete writeDone;
  delete readAvail;
  delete console;
}

void SynchConsole::put(char c){
  lock->Acquire();
  writeDone->P();
  console->PutChar(c);
  lock->Release();
}

const char SynchConsole::get(){
  lock->Acquire();
  readAvail->P();
  char c = console->GetChar();
  lock->Release();
  return c;
}
