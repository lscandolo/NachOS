#ifndef MMU_H
#define MMU_H

void PageFaultHandler(int virtAddr);
int    tlbFreeIndex();
TranslationEntry getPageEntry(int vpage);
int frameFreeIndex();
#endif
