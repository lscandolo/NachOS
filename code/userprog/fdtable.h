#ifndef FDTABLE_H
#define FDTABLE_H

#include "../filesys/filesys.h"
#include "system.h"

#include <vector>
#include <string>

extern FileSystem  *fileSystem;

typedef int OpenFileId;

enum FDType{file,console};
enum FDMode{r,w,rw};
enum FDStatus{used,unused};

//File descriptor
struct FileDescriptor{
  FDStatus status;
  FDType type;
  FDMode mode;
  OpenFile* file;
};


// File descriptor table
class FDTable{

 public:

  FDTable();
  ~FDTable();

  bool        create(std::string filename);
  OpenFileId open(std::string filename);
  bool        close(OpenFileId id);
  const FileDescriptor getDescriptor(OpenFileId id);

 private:

  int getUnusedDescriptor();
  std::vector<FileDescriptor> table;
  
};

#endif // FDTABLE_H

