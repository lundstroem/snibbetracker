//
//  CTimer.c
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <stdio.h>
#include "CTimer.h"
#include "CAllocator.h"

struct CTimer *cTimerNew(double wait_time)
{
	struct CTimer *t = cAllocatorAlloc(sizeof(struct CTimer), "CTimer");
    t->amountToWait = wait_time;
    t->initAmountToWait = wait_time;
    t->currentWaitingTime = 0;
    t->isReady = false;
    return t;
}

double cTimerTimeLeft(struct CTimer *t)
{
    return t->amountToWait-t->currentWaitingTime;
}


void cTimerSetReady(struct CTimer *t)
{
	t->currentWaitingTime = t->amountToWait+1;
}

unsigned char cTimerIsReady(struct CTimer *t)
{
	if(t->currentWaitingTime > t->amountToWait) {
		t->isReady = true;
	}
	return t->isReady;
}

void cTimerAdvance(double time, struct CTimer *t)
{
	//if the timer has not finished, advance.
	if(t->isReady == 0) {
		t->currentWaitingTime += time;
	}
}

void cTimerResetWithTime(double time, struct CTimer *t)
{
	t->amountToWait = time;
    t->initAmountToWait = time;
	t->isReady = false;
	t->currentWaitingTime = 0;
}

void cTimerReset(struct CTimer *t)
{
	t->amountToWait = t->initAmountToWait;
	t->isReady = false;
	t->currentWaitingTime = 0;
}
