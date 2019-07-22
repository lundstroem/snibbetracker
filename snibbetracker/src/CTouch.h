//
//  CTouch.h
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//
#include <stdbool.h>

#ifndef cengine_CTouch_h
#define cengine_CTouch_h

struct CTouch
{
	int x;
    int y;
    int ms_since_init;
    bool active;
    bool began;
    bool began_lock;
    bool ended;
    int spinner_button_id;
    int dragable_button_id;
};

struct CTouch *cTouchNew(void);

#endif
