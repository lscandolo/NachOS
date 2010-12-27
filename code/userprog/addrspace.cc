// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"

#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))

bool writeArgBuffer(char* buf, int size, int virtAddr);
bool WriteMem(int addr, int size, int value);

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::Initialize
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

bool AddrSpace::Initialize(OpenFile *executableFile, int argc, char** argv,  int* userSpaceArgv)
{

    unsigned int i, size;

    executable = executableFile;
    int argSize = argc * sizeof(char*); //Asumimos q es lo mismo en el comp cruzado
    for (int j = 0; j < argc; j++){
      argSize += strlen(argv[j])+1;
    }

    DEBUG('a', "Address space argument size: %d\n", argSize);

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
      SwapHeader(&noffH);

    if (noffH.noffMagic != NOFFMAGIC)
      return false;

    // how big is the address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize + argSize;
    // we need to increase the size
    // to leave room for the stack and the arguments

    size += 4; //This is to make sure we can write the args starting at an 4-aligned address

    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    DEBUG('a', "Initializing address space, numPages: %d\t size: 0x%x\n"
	  ,numPages,size);


// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++){

      pageTable[i].physicalPage = -1;	// No physical frame matches this
      pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
      pageTable[i].valid = false;
      pageTable[i].use = false;
      pageTable[i].dirty = false;
      pageTable[i].readOnly = false;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    }

    int initialArgAddress = divRoundUp(size - UserStackSize - argSize, 4)*4; //To make sure it's aligned
    if (userSpaceArgv != NULL)
      *userSpaceArgv = initialArgAddress;

    return true;
}

bool AddrSpace::loadPageOnDemand(int page, int frame){
  ASSERT(page >= 0 && page < numPages);
  ASSERT (frame != -1);

  bzero(machine->mainMemory + PageSize * frame, PageSize);
  
  unsigned int pageSP; // page starting point
  unsigned int segmentSP; // segment starting point

  unsigned int copySize; // Amount of bytes to copy

  int charsRead;

  // printf("Loading page: %d, frame: %d\n", page,frame);
  /////////////////////////////// CODE SEGMENT //////////////////////////////////////
  if (noffH.code.virtualAddr < (page + 1) * PageSize &&
      noffH.code.virtualAddr + noffH.code.size > page * PageSize){

    // printf("Loading code segment\n");

    if (noffH.code.virtualAddr <= page * PageSize){
      pageSP = 0;
      segmentSP = page * PageSize - noffH.code.virtualAddr;
    }
    else {
      pageSP = noffH.code.virtualAddr - page * PageSize;
      segmentSP = 0;
    }

    // printf("code.virtualAddr: %d \t code.size = %d\n",noffH.code.virtualAddr, noffH.code.size);
    // printf("pageSP: %d \t segmentSP = %d\n",pageSP, segmentSP);
    

    copySize = PageSize - pageSP;
    if (noffH.code.virtualAddr + noffH.code.size < (page+1) * PageSize)
      copySize -= (page+1) * PageSize - (noffH.code.virtualAddr + noffH.code.size);

    charsRead = executable->ReadAt(machine->mainMemory + frame*PageSize+pageSP,
				   copySize,
				   noffH.code.inFileAddr + segmentSP);

    // printf("copySize: %d \t charsRead = %d\n", copySize, charsRead);

    if (charsRead != copySize)
      return false;
  }

  ////////////////////////////////// DATA SEGMENT ///////////////////////////////////////
  if (noffH.initData.virtualAddr < (page + 1) * PageSize &&
      noffH.initData.virtualAddr + noffH.initData.size > page * PageSize){

    // printf("Loading data segment\n");
    if (noffH.initData.virtualAddr <= page * PageSize){
      pageSP = 0;
      segmentSP = page * PageSize - noffH.initData.virtualAddr;
    }
    else {
      pageSP = noffH.initData.virtualAddr - page * PageSize;
      segmentSP = 0;
    }

    // printf("initData.virtualAddr: %d \t initData.size = %d\n",noffH.initData.virtualAddr, noffH.initData.size);
    // printf("pageSP: %d \t segmentSP = %d\n",pageSP, segmentSP);

    copySize = PageSize - pageSP;
    if (noffH.initData.virtualAddr + noffH.initData.size < (page+1) * PageSize)
      copySize -= (page+1) * PageSize - (noffH.initData.virtualAddr + noffH.initData.size);

    charsRead = executable->ReadAt(machine->mainMemory + frame*PageSize+pageSP,
				   copySize,
				   noffH.initData.inFileAddr + segmentSP);

    // printf("copySize: %d \t charsRead = %d\n", copySize, charsRead);

    if (charsRead != copySize)
      return false;
  }
  /////////////////////////////////////////////////////////////////////////////////////////////
  
  coremap->usedFrames[frame] = true;
  coremap->owner[frame] = (SpaceId)currentThread;

  currentThread->space->setValidity(page,true);
  currentThread->space->setDirty(page,false);

  // printf("Loading frame: %d, from page: %d\n",frame,page);

  return true;
}

    //Copy args into user space
void AddrSpace::copyArguments(int argc, char** argv, int initialArgAddress)
{

    int argSize = argc * sizeof(char*); //Asumimos q es lo mismo en el comp cruzado
    for (int j = 0; j < argc; j++){
      argSize += strlen(argv[j])+1;
    }

    char* argBuffer = new char[argSize];
    
    int pointerOffset = 0;
    int dataOffset = pointerOffset + argc * sizeof(char*);

    for (int j = 0 ; j < argc ; j++){
      
      *((int*)(argBuffer + pointerOffset)) = initialArgAddress + dataOffset;
      strcpy(argBuffer + dataOffset, argv[j]);
      
      pointerOffset += sizeof(char*);
      dataOffset += strlen(argv[j]) + 1;
    }

    // for (int i = 0; i < argc; i++)
    //   printf("\n%d",((int*)argBuffer)[i]);

    // for (int i = argc*4; i < argSize; i++)
    //   printf("\n%c",argBuffer[i]);

    // exit(0);

    writeArgBuffer(argBuffer, argSize, initialArgAddress);

    delete argBuffer;
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace() {

  if (pageTable != NULL){
    // Mark the pages as free now
    for (int i = 0; i < numPages; i++){
      if (pageTable[i].valid){
	coremap->usedFrames.set(pageTable[i].physicalPage,false);
	coremap->setUnused(pageTable[i].physicalPage);
      }
      
    }
 
    delete pageTable;
    delete executable;
    delete swap;
  }
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);


    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{

}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
#ifndef USE_TLB
  machine->pageTable = pageTable;
  machine->pageTableSize = numPages;
#else
  for(int i = 0; i < TLBSize; i++)
    machine->tlb[i].valid = false;
#endif
}

TranslationEntry AddrSpace::pageTableEntry(int vpage){
  // printf("Calling page table entry with vpage: %d, from: %d\n",vpage, numPages);
  ASSERT(vpage >= 0 && vpage < numPages);
  return pageTable[vpage];
}

bool writeArgBuffer(char* buf, int size, int virtAddr){
  for (int i = 0; i < size ; i++,virtAddr++){
    if (!WriteMem(virtAddr, 1, buf[i]))
      return false;
  }
  return true;
}


// bool
// WriteMem(int addr, int size, int value){
//     ExceptionType exception;
//     int physicalAddress;
     
//     DEBUG('a', "Writing VA 0x%x, size %d, value 0x%x\n", addr, size, value);

//     do {
//       exception = machine->Translate(addr, &physicalAddress, size, FALSE);
//       if (exception != NoException) 
// 	ExceptionHandler(exception);
//     } while (exception != NoException);

//     switch (size) {
//       case 1:
// 	machine->mainMemory[physicalAddress] = (unsigned char) (value & 0xff);
// 	break;

//       case 2:
// 	*(unsigned short *) &machine->mainMemory[physicalAddress]
// 		= ShortToMachine((unsigned short) (value & 0xffff));
// 	break;
      
//       case 4:
// 	*(unsigned int *) &machine->mainMemory[physicalAddress]
// 		= WordToMachine((unsigned int) value);
// 	break;
	
//       default: ASSERT(FALSE);
//     }
    
//     return TRUE;
  
// }
