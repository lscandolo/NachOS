#include "syscall.h"
#include "fdtable.h"
#include "system.h"
#include <string>

void        syscallHalt(){
	DEBUG('m', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
}

void        syscallExit(int status){
  DEBUG('m', "Syscall Exit called with status: %d\n",status);
  currentThread->Finish();
}

SpaceId     syscallExec(char *name){}

int         syscallJoin(SpaceId id){}

void        syscallCreate(char *name){
  currentThread->fdtable->create(std::string(name));
}

OpenFileId syscallOpen(char *name){
  return currentThread->fdtable->open(std::string(name));
}

void        syscallWrite(char *buffer, int size, OpenFileId id){
  FileDescriptor descriptor = currentThread->fdtable->getDescriptor(id);
  if (descriptor.status == unused || descriptor.mode == r)
    return;

  if (descriptor.type == file){
    descriptor.file->Write(buffer,size);
  }

  if (descriptor.type == console){
    synchConsole->writeStr(std::string(buffer,size));
  }
}

int         syscallRead(char *buffer, int size, OpenFileId id){
  FileDescriptor descriptor = currentThread->fdtable->getDescriptor(id);
  if (descriptor.status == unused || descriptor.mode == w)
    return 0;

  if (descriptor.type == file){
    return descriptor.file->Read(buffer,size);
  }

  if (descriptor.type == console){
    synchConsole->readStr(buffer,size);
    return size;
  }
}

void        syscallClose(OpenFileId id){
  currentThread->fdtable->close(id);
}

void        syscallFork(void (*func)()){}

void        syscallYield(){}
