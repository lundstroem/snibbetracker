/*
 
MIT License

Copyright (c) 2019 Harry Lundström

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "CAllocator.h"

static bool track_allocations = false;
static bool debug_log = false;
static int null_allocs = 0;
static int alloc_count = 0;
static int alloc_track_size = 1000;
static int alloc_track_cursor = 0;
static struct CAllocatorNode **alloc_ids = NULL;
static struct CAllocatorNode *cAllocatorNodeNew(size_t size, void *pointer, char *name);
static bool cAllocatorRemoveTrackedAllocation(void *pointer);

void cAllocatorSetTrackAllocations(bool track) {
    
    track_allocations = track;
}

void cAllocatorSetDebuglog(bool debuglog) {
    
    debug_log = debuglog;
}

void cAllocatorPrintAllocationCount(void) {
    
    if(debug_log) {
        printf("CAllocator alloc count:%d\n", alloc_count);
    }
}

void cAllocatorPrintAllocations(void) {
    
    if (track_allocations) {
        for (int i = 0; i < alloc_track_size; i++) {
            if(alloc_ids[i] != NULL) {
                if (debug_log) {
                    if(!alloc_ids[i]->deallocated && !alloc_ids[i]->duplicate_address) {
                        printf("active alloc name: %s size:%zu pointer:%p\n", alloc_ids[i]->name, alloc_ids[i]->size, alloc_ids[i]->pointer);
                    }
                }
            }
        }
    } else {
        if(debug_log) {
            printf("cAllocator - allocation tracking is disabled.\n");
        }
    }
}

void cAllocatorCleanup(void) {
    
    if (track_allocations) {
        if(alloc_ids != NULL) {
            for (int i = 0; i < alloc_track_size; i++) {
                if(alloc_ids[i] != NULL) {
                    free(alloc_ids[i]);
                    alloc_ids[i] = NULL;
                }
            }
            free(alloc_ids);
            alloc_ids = NULL;
        }
    }
}

static struct CAllocatorNode *cAllocatorNodeNew(size_t size, void *pointer, char *name) {

    struct CAllocatorNode *n = NULL;
    if(alloc_track_cursor < alloc_track_size) {
        if(alloc_ids == NULL) {
            alloc_ids = malloc(sizeof(struct CAllocatorNode*)*alloc_track_size);
            for (int i = 0; i < alloc_track_size; i++) {
                alloc_ids[i] = NULL;
            }
        }
        
        n = malloc(sizeof(struct CAllocatorNode));
        n->size = size;
        n->pointer = pointer;
        n->name = name;
        n->deallocated = false;
        n->duplicate_address = false;
        cAllocatorCheckDuplicates(pointer);
        alloc_ids[alloc_track_cursor] = n;
        //printf("alloc_ids[%d]: alloc %s pointer:%p\n", alloc_track_cursor, n->name, n->pointer);
        alloc_track_cursor++;
        return n;
    } else {
        if(debug_log) {
            printf("cAllocatorAlloc error: cursor exceeds size for trackable allocations limit:%d", alloc_track_size);
        }
        return NULL;
    }
}

void cAllocatorExpandTrackSize(void) {
    int new_track_size = alloc_track_size + 10000;
    struct CAllocatorNode **new_alloc_ids = malloc(sizeof(struct CAllocatorNode*)*new_track_size);
    for(int i = 0; i < new_track_size; i++) {
        if (i < alloc_track_size) {
            new_alloc_ids[i] = alloc_ids[i];
        } else {
            new_alloc_ids[i] = NULL;
        }
    }
    alloc_track_size = new_track_size;
    free(alloc_ids);
    alloc_ids = new_alloc_ids;
    if(debug_log) {
        printf("re2_allocator_alloc new size:%d", alloc_track_size);
    }
}

void cAllocatorCheckDuplicates(void *pointer) {
    /* if a tracked and freed alloc has the same address as a new one, prevent it
     from generating errors by setting the duplicate flag.*/
    for (int i = 0; i < alloc_track_size; i++) {
        if(alloc_ids[i] != NULL) {
            if(alloc_ids[i]->pointer == pointer &&
               alloc_ids[i]->deallocated) {
                alloc_ids[i]->duplicate_address = true;
            }
        }
    }
}

static bool cAllocatorRemoveTrackedAllocation(void *pointer) {

    for (int i = 0; i < alloc_track_size; i++) {
        if(alloc_ids[i] != NULL) {
            if(alloc_ids[i]->pointer == pointer) {
                if (!alloc_ids[i]->deallocated) {
                    alloc_ids[i]->deallocated = true;
                    return true;
                } else if(!alloc_ids[i]->duplicate_address) {
                    printf("cAllocator warning: trying to free deallocated pointer. name: %s size:%zu pointer:%p alloc_node:%p\n", alloc_ids[i]->name, alloc_ids[i]->size, alloc_ids[i]->pointer, alloc_ids[i]);
                    return false;
                } else if(alloc_ids[i]->duplicate_address) {
                    // it has been replaced by another alloc. ignore it.
                }
            }
        }
    }
    if (debug_log) {
        printf("cAllocator warning: Could not find pointer:%p to dealloc\n", pointer);
    }
    return false;
}

void *cAllocatorAlloc(size_t size, char *name) {
    
    void *ptr = NULL;
    ptr = malloc(size);
    if(ptr == NULL) {
        if(debug_log) {
            printf("cAllocatorAlloc error: malloc with size %zu returned NULL.\n name:%s", size, name);
        }
    } else {
        if(track_allocations) {
            struct CAllocatorNode *node = cAllocatorNodeNew(size, ptr, name);
            if (node == NULL) {
                cAllocatorExpandTrackSize();
                cAllocatorNodeNew(size, ptr, name);
            }
        }
        alloc_count++;
    }
    return ptr;
}

void *cAllocatorFree(void *ptr) {
    if(ptr != NULL) {
        bool dealloc = true;
        if(track_allocations) {
            dealloc = cAllocatorRemoveTrackedAllocation(ptr);
        }
        if(dealloc) {
            free(ptr);
            alloc_count--;
        }
    } else {
        if(debug_log) {
            printf("cAllocatorAlloc warning: pointer cannot be deallocated because it's NULL. \n");
        }
        null_allocs++;
    }
    return NULL;
}

