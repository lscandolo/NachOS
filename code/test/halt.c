/* halt.c
 	Simple program to test whether running a user program works.
 	
 	Just do a "syscall" that shuts down the OS.

  	NOTE: for some reason, user programs with global data structures 
 	sometimes haven't worked in the Nachos environment.  So be careful
 	out there!  One option is to allocate data structures as 
 	automatics within a procedure, but if you do this, you have to
 	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

int
main(int argc, char** argv)
/* main () */
{

  int i;
  int prog;
  char *buf = "HOLA MUNDO SOY HALT!\n";

  for(i = 0; i < argc; i++){
    Write(argv[i],1,1);
  }

  Write(buf,22,1);

  prog = Exec("test/halt2 a b c d e f ");
  /* Exec("test/halt2"); */

  /* while(1) */
  /*     Write("A",1,1); */

  Join(prog);
  Exit(125);
    /* Halt(); */
    /* not reached */
}

