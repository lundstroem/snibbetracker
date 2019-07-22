//
//  CAnimation.c
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <stdio.h>
#include "CAnimation.h"
#include "CAnimationFrame.h"
#include "CListQueue.h"
#include "CAllocator.h"

struct CAnimation *cAnimationNew(double dt, int frames)
{    
    struct CAnimation *anim = cAllocatorAlloc(sizeof(struct CAnimation), "CAnimation");
    anim->currentFrame = 0;
    anim->currentFrameTime = 0;
    anim->currentLoop = 0;
    anim->frameLength = dt;
    anim->nrOfFrames = frames;
    anim->size = 0;
    anim->paused = false;
    anim->flipped_h = false;
    anim->flipped_v = false;
    anim->nrOfLoops = 0;
    anim->isReady = false;
    anim->frames = cAllocatorAlloc(frames * sizeof(struct CAnimationFrame*), "CAnimationFrame");
    anim->currentFrameTime += dt;
    anim->default_frame.x = 0;
    anim->default_frame.y = 0;
    anim->collision_id = 0;
    return anim;
}

void cAnimationInsertFrame(struct CAnimation *anim, int index, int sheet_x, int sheet_y)
{
    struct CAnimationFrame *f = cAnimationFrameNew();
    f->x = sheet_x;
    f->y = sheet_y;
    f->size = anim->size;
    anim->frames[index] = f;
}

struct CAnimationFrame* cAnimationGetCurrentFrame(struct CAnimation *anim)
{
    if(anim != NULL) {
        return anim->frames[anim->currentFrame];
    } else {
        return NULL;
    }
}

void cAnimationAdvance(double dt, struct CAnimation *anim)
{
    if(!anim->paused) {
        anim->currentFrameTime += dt;
        if(anim->currentFrameTime > anim->frameLength) {
            //if were at the last frame, change to the first if the time is up for the current frame.
            //else advance to the next frame.
            if(anim->currentFrame == anim->nrOfFrames-1) {
                //if looping forever
                if(anim->nrOfLoops == 0) {
                    anim->currentFrame = 0;
                } else {
                    //increase loop count
                    anim->currentLoop++;
                        
                    //loop again if we havent reached total number of loops.
                    if(anim->currentLoop < anim->nrOfLoops)
                        anim->currentFrame = 0;
                    else anim->isReady = true;
                }
            }
            else anim->currentFrame += 1;
                
            //reset frametime
            anim->currentFrameTime = 0;
        }
    }
}

void cAnimationRewind(struct CAnimation *anim)
{
	anim->currentFrame = 0;
	anim->currentFrameTime = 0;
}

void *cAnimationFree(struct CAnimation *anim)
{
    if(anim != NULL) {
        if(anim->frames != NULL) {
            anim->frames = cAllocatorFree(anim->frames);
        }
        cAllocatorFree(anim);
    }
    return NULL;
}

