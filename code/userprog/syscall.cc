#include "system.h" //No poner syscall antes de system!!!!!
#include "syscall.h"

#include <string>
#include "fdtable.h"
#include "addrspace.h"

OpenFileId syscallOpen(char *name);

void threadRun(int arg){
  currentThread->space->InitRegisters();		// set the initial register values
  currentThread->space->RestoreState();		// load page table register
  machine->Run();
}

/////////////////////////Halt/////////////////////////
void        syscallHalt(){
	DEBUG('m', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
}

/////////////////////////Exit/////////////////////////
void        syscallExit(int status){
  DEBUG('m', "Syscall Exit called with status: %d\n",status);
  currentThread->Finish();
}

/////////////////////////Exec/////////////////////////
SpaceId     syscallExec(int argc, char** argv){

  DEBUG('m', "Syscall Exec called with excutable file: %s\n",argv[0]);

  OpenFile* executable = fileSystem->Open(argv[0]);
  if (executable == NULL)
    return -1;

  AddrSpace* space = new AddrSpace();

  if (!space->Initialize(executable)){
    delete executable;
    delete space;
    return -1;
  }
  
  Thread* child = new Thread(std::string(argv[0]),true);
  child->space = space;
  child->Fork( (VoidFunctionPtr) threadRun, 0);
  
  delete executable;
  return (SpaceId) child;
}

/////////////////////////Join////////////////////////////
int         syscallJoin(SpaceId id){
  if (currentThread->isJoinableChild((Thread*) id))
    return ((Thread*)id)->Join();
  else
    return -1;
}

/////////////////////////Create/////////////////////////
void        syscallCreate(const char *name){
  currentThread->fdtable->create(std::string(name));
}

/////////////////////////Open/////////////////////////
OpenFileId syscallOpen(const char *name){
  return currentThread->fdtable->open(std::string(name));
}

/////////////////////////Write/////////////////////////
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

/////////////////////////Read/////////////////////////
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

/////////////////////////Close/////////////////////////
void        syscallClose(OpenFileId id){
  currentThread->fdtable->close(id);
}

/////////////////////////Fork/////////////////////////
void        syscallFork(void (*func)()){
}

/////////////////////////Yield/////////////////////////
void        syscallYield(){
  currentThread->Yield();
}
