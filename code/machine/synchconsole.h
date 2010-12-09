#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "console.h"
#include "thread.h"
#include "synch.h"
#include <iostream>
#include <string>

class SynchConsole {

 public:

  SynchConsole(char* in, char* out);
  ~SynchConsole();

  const char get();
  void put(char c);

  ///////////////////////
  // writeStr: Aux function to output a string
  ///////////////////////

  void writeStr(std::string s){
    for (unsigned int i = 0; i < s.size() ; i++)
      put(s[i]);
  }

  void readStr(char* buffer, int size){
    for (int i = 0; i < size ; i++)
      buffer[i] = get();
  }

  
  void readAvailV(){readAvail->V();}
  void writeDoneV(){writeDone->V();}

 private:

  Semaphore* readAvail;
  Semaphore* writeDone;
  Console* console;
  Lock*       readLock;
  Lock*       writeLock;

};

#endif  //SYNCHCONSOLE_H 
