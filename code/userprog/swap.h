#ifndef SWAP_H
#define SWAP_H

#include "openfile.h"
#include <vector>
#include <string>

typedef int SpaceId;

class Swap{

 public:

  Swap(SpaceId id, int pageNum);
  ~Swap();
  
  bool isSwaped(int page){
    ASSERT(page >= 0 && page < swapMap.size());
    return swapMap[page];
  }

  bool swapIn(int page, int frame);
  bool swapOut(int frame);

 private:
  
  SpaceId id;
  OpenFile* swapFile;
  std::string swapFileName;
  std::vector<bool> swapMap;
};

#endif
