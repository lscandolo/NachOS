#ifndef COREMAP_H
#define COREMAP_H

#include <bitset>
#include <map>
#include <set>


class CoreMap{

public:
    CoreMap();
    ~CoreMap();
private:

  std::set<Thread*> threadSet;

public:
  std::bitset<NumPhysPages> usedFrames;
  std::map<int,SpaceId> owner;

  void addThread(Thread* t);
  void delThread(Thread* t);
  bool onlyThread(){return threadSet.size() == 1;}
};

#endif // COREMAP_H
