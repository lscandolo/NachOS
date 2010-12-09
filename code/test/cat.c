#include "syscall.h"

int main(int argc, char** argv){

  char *usage_msg = "Usage: cat [filename]\n";
  int usage_msg_len = 22;

  char* file_error_msg = "Error: Unable to open file\n";
  int file_error_msg_len = 27;

  char c;
  int i;

  OpenFileId input = ConsoleInput;
  OpenFileId output = ConsoleOutput;
  OpenFileId file;

  if (argc <= 1){
    Write(usage_msg,usage_msg_len,output);
    return 0;
  }

  for (i = 1; i < argc; i++){

    file = Open(argv[1]);
  
    if (file == -1){
      Write(file_error_msg,file_error_msg_len,output);
      return 0;
    }

    while(Read(&c,1,file)) {
      Write(&c,1,output);
    }
    Close(file);
  }

  return 0;
}
