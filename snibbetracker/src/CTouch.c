//
//  CTouch.c
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <stdio.h>
#include "CTouch.h"
#include "CAllocator.h"

struct CTouch *cTouchNew(void)
{
    struct CTouch *t = cAllocatorAlloc(sizeof(struct CTouch), "CTouch");
    t->x = 0;
    t->y = 0;
    t->began = false;
    t->began_lock = false;
    t->active = false;
    t->ended = false;
    t->spinner_button_id = -1;
    t->dragable_button_id = -1;
    t->ms_since_init = 0;
    return t;
}
