#include "syscall.h"

int main(int argc, char** argv){

  char *usage_msg = "Usage: cp [source] [dest]\n";
  int usage_msg_len = 26;

  char* file_error_msg = "Error: Unable to open source file\n";
  int file_error_msg_len = 34;

  char c;

  OpenFileId input = ConsoleInput;
  OpenFileId output = ConsoleOutput;
  OpenFileId source;
  OpenFileId dest;

  if (argc != 3){
    Write(usage_msg,usage_msg_len,output);
    return 0;
  }

  source = Open(argv[1]);

  if (source == -1){
    Write(file_error_msg,file_error_msg_len,output);
    return 0;
  }

  Create(argv[2]);
  dest = Open(argv[2]);

  while(Read(&c,1,source)) {
    Write(&c,1,dest);
  }

  Close(source);
  Close(dest);

  return 0; 
}
