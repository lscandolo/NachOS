#include "system.h"
#include "coremap.h"


CoreMap::CoreMap(){

  usedFrames.reset();

  for (int i = 0; i < NumPhysPages; i++)
    owner[i] = -1;

}

CoreMap::~CoreMap(){
#ifdef USER_PROGRAM
  while (!threadSet.empty()){
    Thread* t = *(threadSet.begin());
    delThread(t);
    if (t != currentThread)
      delete t;
    else
      delete t->space;
  }
    
#endif
}


void CoreMap::addThread(Thread* t){
    
#ifdef USER_PROGRAM
    if (threadSet.empty())
      synchConsole=new SynchConsole(NULL,NULL);
#endif

    threadSet.insert(t);
    
  }

void CoreMap::delThread(Thread* t){
    threadSet.erase(t);

#ifdef USER_PROGRAM
    if (threadSet.empty())
      delete synchConsole;
    
#endif

  }
