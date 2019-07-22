//
//  CAllocator.h
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#ifndef cengine_CAllocator_h
#define cengine_CAllocator_h

struct CAllocatorNode {
    size_t size;
    void *pointer;
    char *name;
    bool deallocated;
    bool duplicate_address;
};

struct CAllocatorArray {
    int x;
    int y;
    int z;
    void **array1d;
    void ***array2d;
    void ****array3d;
};

void cAllocatorSetTrackAllocations(bool track);
void cAllocatorSetDebuglog(bool debuglog);
void cAllocatorPrintAllocationCount(void);
void cAllocatorPrintAllocations(void);
void cAllocatorCleanup(void);
void *cAllocatorAlloc(size_t size, char *name);
void *cAllocatorFree(void *ptr);
void cAllocatorExpandTrackSize(void);
void cAllocatorCheckDuplicates(void *pointer);

// TODO testing, and adding factory methods for int arrays etc.
struct CAllocatorArray *cAllocatorArrayNew(void);
struct CAllocatorArray *cAllocator1dPointerArray(int x, size_t size);
struct CAllocatorArray *cAllocator2dPointerArray(int x, int y, size_t size);
struct CAllocatorArray *cAllocator3dPointerArray(int x, int y, int z, size_t size);
void *cAllocatorFreeArray(struct CAllocatorArray *array);

#endif