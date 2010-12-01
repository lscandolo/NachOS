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
/* main(int argc, char** argv) */
main ()
{

  char *buf = "HOLA MUNDO SOY HALT!\n";

  /* Exec("test/halt2 a b c d e f "); */

  Write(buf,21,1);

  Exec("test/halt2");

  /* while(1) */
  /*     Write("A",1,1); */

  Exit(123);
    /* Halt(); */
    /* not reached */
}

