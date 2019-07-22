//
//  CAnimation.h
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <stdbool.h>
#include "CAnimationFrame.h"

#ifndef cengine_CAnimation_h
#define cengine_CAnimation_h

struct CAnimation
{
	double frameLength;  //how many ms every frame should take
    int nrOfFrames;   //the number of frames the animation consists of.
    int size;
    bool paused;
    int currentFrame; //the current frame in the animation.
    double currentFrameTime; //the time of the current frame.
    int nrOfLoops;		//how many times will the animation loop. Set to 0
    //to loop indefinitely.
    int currentLoop;
    bool isReady;
    struct CAnimationFrame **frames;
    struct CAnimationFrame default_frame;
    int collision_id;
    bool flipped_h;
    bool flipped_v;
};

struct CAnimation *cAnimationNew(double dt, int frames);
void cAnimationInsertFrame(struct CAnimation *anim, int index, int sheet_x, int sheet_y);
struct CAnimationFrame* cAnimationGetCurrentFrame(struct CAnimation *anim);
void cAnimationAdvance(double dt, struct CAnimation *anim);
void cAnimationRewind(struct CAnimation *anim);
void *cAnimationFree(struct CAnimation *anim);

#endif
