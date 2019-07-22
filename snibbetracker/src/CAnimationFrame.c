/*
    CAnimationFrame.c
    cengine
 
    Created by Harry Lundstrom on 9/24/14.
    Copyright (c) 2014 Harry Lundstrom. All rights reserved.
*/

#include <stdio.h>
#include "CAnimationFrame.h"
#include "CAllocator.h"

struct CAnimationFrame *cAnimationFrameNew(void) {
    
    struct CAnimationFrame *t = cAllocatorAlloc(sizeof(struct CAnimationFrame), "CAnimationFrame 2");
    t->x = 0;
    t->y = 0;
    t->size = 0;
    t->time_ms = 0;
    return t;
}

void cAnimationFrameUpdate(struct CAnimationFrame *f, int x, int y) {
    
    f->x = x;
    f->y = y;
}