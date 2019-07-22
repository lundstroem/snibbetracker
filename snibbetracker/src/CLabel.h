//
//  CLabel.h
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#ifndef cengine_CLabel_h
#define cengine_CLabel_h

#include <stdio.h>
#include <stdbool.h>
#include "CStr.h"

struct CLabel
{
    struct CStr *str;
    int x_offset_max; // for scrolltext
    int x;
    int y;
    bool scrolling;
};

struct CLabel *cLabelNew(struct CStr *cstr);
void cLabelSetLabel(struct CLabel *l, struct CStr *cstr);
void cLabelActivateScrolling(struct CLabel *l, int start_x);
void cLabelCleanup(struct CLabel *l);

#endif