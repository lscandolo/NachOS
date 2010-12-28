#include "syscall.h"

int
main(int argc , char** argv )
{

  int i = 0;

  /* Write("B",1,1); */
  char *buf = "HOLA MUNDO SOY HALT2!!\n";

  Write(buf,24,1);

  for(i = 0; i < argc; i++){
    Write(argv[i],1,1);
  }
       
  Exit(123);
}

