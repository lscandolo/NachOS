#ifndef COREMAP_H
#define COREMAP_H

#include <bitset>
#include <map>
#include <set>
#include <list>

class CoreMap{

public:
    CoreMap();
    ~CoreMap();
private:

  std::set<Thread*> threadSet;
  std::list<int> lru;

public:
  std::bitset<NumPhysPages> usedFrames;
  std::map<int,SpaceId> owner;

  void addThread(Thread* t);
  void delThread(Thread* t);
  bool onlyThread(){return threadSet.size() == 1;}

  void setUsed(int frame)  {lru.remove(frame); lru.push_back(frame);}
  void setUnused(int frame){lru.remove(frame);}
  int  getLRU(){return lru.front();}

};

#endif // COREMAP_H
