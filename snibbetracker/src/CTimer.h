//
//  CTimer.h
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//
#include <stdbool.h>

#ifndef cengine_CTimer_h
#define cengine_CTimer_h

struct CTimer
{
	double amountToWait;
	double initAmountToWait;
	double currentWaitingTime;
	bool isReady;
};

struct CTimer *cTimerNew(double wait_time);
double cTimerTimeLeft(struct CTimer *t);
void cTimerSetReady(struct CTimer *t);
unsigned char cTimerIsReady(struct CTimer *t);
void cTimerAdvance(double time, struct CTimer *t);
void cTimerResetWithTime(double time, struct CTimer *t);
void cTimerReset(struct CTimer *t);


#endif
