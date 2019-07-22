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

#endif
