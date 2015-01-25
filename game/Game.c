//
//  ExampleGame.c
//  iOS_GL_Frontend
//
//  Created by Harry Lundstrom on 22/09/14.
//  Copyright (c) 2014 harrylundstrom. All rights reserved.
//

#include "Game.h"
#include "CEngine.h"
#include "CAnimation.h"
#include "CAnimationFrame.h"
#include "CListQueue.h"
#include "CTimer.h"
#include "CLabel.h"
#include <stdlib.h>

/* Create entities with id's, level etc */
void cEngineInitHook(struct CEngineContext *e) {
    
}


void cEnginePreUpdateHook(struct CButton **buttons, double dt) {
  
}


void cEngineUpdateHook(double dt) {
    
    /* center camera on specific entity or coordinate */
    /* handle animations */
    /* handle input */
    
}

void cEnginePreEntityRenderHook(double dt, unsigned int **raster) {

}

void cEngineRenderHook(double dt, unsigned int **raster) {

}

void cEngineEntityCollisionHook(struct CEntity *a, struct CEntity *b) {
    /* collision response */
    //printf("collision");
}

void cEngineEnvironmentCollisionHook(void) {
    
}

void cEngineCleanupHook(void) {

}





