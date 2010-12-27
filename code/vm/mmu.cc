#include "system.h" //Nunca poner system debajo de syscall!!
#include "mmu.h"

void PageFaultHandler(int virtAddr){

  int vpage = virtAddr / PageSize;

  stats->numTLBSoftMiss++;
  stats->numMemAccess--; //Because the translate will be tried again
  DEBUG('a',"vpage fault: %d\n",vpage);

  //Get the page entry info for the page (get it back from swap if paged out)
  TranslationEntry entry = getPageEntry(vpage);

  if (!entry.valid){
      bool res;
      int frame = frameFreeIndex();

      // If page is invalid not swaped then it's still
      // not loaded from the executable, so load it
      if (!currentThread->space->swap->isSwaped(vpage)){
	res = currentThread->space->loadPageOnDemand(vpage,frame);
	entry.dirty = true;
	ASSERT(res); //Change this to killing the thread afterwards!
      }
      // Else it's in swap, so swap it in
      else{
	stats->numTLBHardMiss++;
	res = currentThread->space->swap->swapIn(vpage,frame);
	ASSERT(res); //Change this to killing the thread afterwards!
      }
      ASSERT(res); //Change this to killing the thread afterwards!
      ASSERT(entry.virtualPage == vpage); // Just in case..

      entry.valid = true;
      entry.physicalPage = frame;
      currentThread->space->savePageTableEntry(entry,vpage);
    }

  //Find a free index in the TLB (evict if necessary)
  int tlb_index = tlbFreeIndex();

  machine->tlb[tlb_index] = entry;
}

int tlbFreeIndex(){
  
  for(int i = 0; i < TLBSize; i++){
    if (!machine->tlb[i].valid)
      return i;
  }
  
  int tlbEntry = rand()%TLBSize;

  currentThread->space->savePageTableEntry(machine->tlb[tlbEntry]
					   ,machine->tlb[tlbEntry].virtualPage);
  return tlbEntry;
}

TranslationEntry getPageEntry(int vpage){
  return currentThread->space->pageTableEntry(vpage);
}

int frameFreeIndex(){
  int j = 0;

  // If one of the frames is not used return that
  for (; j < coremap->usedFrames.size(); j++)
    if (!coremap->usedFrames[j])
      return j;

  //Pick a random one
  // j = rand() % coremap->usedFrames.size();
  //Super LRU megamode
  j = coremap->getLRU();
  coremap->setUsed(j);

  //If that page is in the tlb, mark it as invalid
  for (int i = 0; i < TLBSize; i++)
    if (machine->tlb[i].physicalPage == j &&
	machine->tlb[i].valid){
      currentThread->space->savePageTableEntry(machine->tlb[i]
					       ,machine->tlb[i].virtualPage);
      machine->tlb[i].valid = false;
      break;
    }

  //Then swap it out
  Thread* frameOwner = (Thread*)coremap->owner[j];
  frameOwner->space->swap->swapOut(j);

  return j;
}

