//
//  ExampleGame.h
//  iOS_GL_Frontend
//
//  Created by Harry Lundstrom on 22/09/14.
//  Copyright (c) 2014 harrylundstrom. All rights reserved.
//

#ifndef __iOS_GL_Frontend__ExampleGame__
#define __iOS_GL_Frontend__ExampleGame__

#include <stdio.h>
#include "CButton.h"
#include "CEntity.h"
#include "CEngine.h"

void cEngineInitHook(struct CEngineContext *e);
void cEnginePreUpdateHook(struct CButton **buttons, double dt);
void cEngineUpdateHook();
void cEnginePreEntityRenderHook(double dt, unsigned int **raster);
void cEngineRenderHook(double dt, unsigned int **raster);
void cEngineEntityCollisionHook(struct CEntity *a, struct CEntity *b);
void cEngineEnvironmentCollisionHook();
void cEngineCleanupHook();

#endif /* defined(__iOS_GL_Frontend__ExampleGame__) */
