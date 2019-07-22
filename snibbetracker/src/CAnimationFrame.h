//
//  CAnimationFrame.h
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <stdbool.h>

#ifndef cengine_CAnimationFrame_h
#define cengine_CAnimationFrame_h

struct CAnimationFrame {
    /* TODO: make collision areas in editor that are frame dependant.
        make them different categories that can be differentiated
        with colors. If the areas are to large, they are split into smaller
        areas internally and moved related to parent position.
     */
	int x;
    int y;
    int size;
    int time_ms;
    bool active; //used in editor
};

struct CAnimationFrame *cAnimationFrameNew(void);
void cAnimationFrameUpdate(struct CAnimationFrame *f, int x, int y);

#endif
