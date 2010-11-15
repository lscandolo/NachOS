#include "fdtable.h"

#include <algorithm>

FDTable::FDTable(){
  FileDescriptor stdin;
  FileDescriptor stdout;

  stdin.type = console;
  stdout.type = console;

  stdin.mode = r;
  stdin.mode = w;

  stdin.status = used;
  stdout.status = used;

  table.push_back(stdin);
  table.push_back(stdout);
}

FDTable::~FDTable(){

  std::vector<FileDescriptor>::iterator it;
  for(it = table.begin() ; it != table.end() ; it++)
    if (it->status == used && it->type == file)
      delete (it->file);
}

bool 
FDTable::create(std::string filename){
  return fileSystem->Create(filename.c_str(),0); 
}

OpenFileId 
FDTable::open(std::string filename){
  OpenFile* openFile = fileSystem->Open(filename.c_str());
  if (openFile == NULL)
    return -1;

  int newid = getUnusedDescriptor();
  table[newid].status = used;
  table[newid].type = file;
  table[newid].mode = rw;
  table[newid].file = openFile;
  return newid;
}

bool 
FDTable::close(OpenFileId id){
  if (id < 0 || id >= table.size())
    return false;

  if (table[id].status == unused)
    return false;

  delete table[id].file;
  return true;
}

const FileDescriptor 
FDTable::getDescriptor(OpenFileId id){

  if (id >= 0 && id < table.size())
    return table[id];
      
  FileDescriptor emptyDescriptor;
  emptyDescriptor.status = unused;
  return emptyDescriptor;
}

int
FDTable::getUnusedDescriptor(){
  int i;
  for (i = 0 ; i < table.size(); i++)
    if (table[i].status = unused) break;

  if (i < table.size())
    return i;
  
  FileDescriptor fd;
  fd.status = unused;
  table.push_back(fd);
  return table.size() - 1;
}

