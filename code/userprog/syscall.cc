#include "system.h" //No poner syscall antes de system!!!!!
#include "syscall.h"

#include <string>
#include "fdtable.h"
#include "addrspace.h"

struct Args{
  int argc;
  char** argv;
  int userSpaceArgv;
};

OpenFileId syscallOpen(char *name);

void deleteArgs(int argc, char **tokens){
  for (int i = 0; i < argc; i++)
    delete[] tokens[i];
  delete[] tokens;
}

void threadRun(int argPtr){
  currentThread->space->InitRegisters();		// set the initial register values
  currentThread->space->RestoreState();		// load page table register

  Args* userSpaceArgs = (Args*) argPtr;

  currentThread->space->copyArguments(userSpaceArgs->argc,
				                                             userSpaceArgs->argv,
				                                             userSpaceArgs->userSpaceArgv);

  machine->WriteRegister(4,userSpaceArgs->argc);
  machine->WriteRegister(5,userSpaceArgs->userSpaceArgv);

  deleteArgs(userSpaceArgs->argc,userSpaceArgs->argv);
  
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
  if (status)
    printf("Thread %s exited with status: %d\n",currentThread->getName().c_str(),status);
  currentThread->Finish();
}

/////////////////////////Exec/////////////////////////
SpaceId     syscallExec(int argc, char** argv){
  DEBUG('m', "Syscall Exec called with excutable file: %s\n",argv[0]);

  OpenFile* executable = fileSystem->Open(argv[0]);
  if (executable == NULL){
    deleteArgs(argc,argv);
    return -1;
  }

  AddrSpace* space = new AddrSpace();

  Args* userSpaceArgs = new Args;
  int userSpaceArgv;
  
  if (!space->Initialize(executable, argc, argv, &userSpaceArgv)){
    delete executable;
    deleteArgs(argc,argv);
    delete space;
    return -1;
  }

  userSpaceArgs->argc = argc;
  userSpaceArgs->argv = argv;
  userSpaceArgs->userSpaceArgv = userSpaceArgv;
  
  Thread* child = new Thread(std::string(argv[0]),true);
  child->space = space;
  child->Fork( (VoidFunctionPtr) threadRun, (int)userSpaceArgs);
  
  // delete executable; //The addrspace destructor handles this now

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
  if (descriptor.status == unused || descriptor.mode == w){
    return 0;
  }

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

