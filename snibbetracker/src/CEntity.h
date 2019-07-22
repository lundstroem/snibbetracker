//
//  CEntity.h
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <stdbool.h>

#ifndef cengine_CEntity_h
#define cengine_CEntity_h

struct CEntity
{
    int type;
    int sheet_x;
    int sheet_y;
    int sheet_y_offset;
    
    double collision_radius;
    int player_shooting;
    int ENTITY_COLLISION;
    double speed;
    int color;
    double dir_x;
    double dir_y;
    int g_x;
    int g_y;
    double vec_x;
    double vec_y;
    
    double x, y, rot;
    double xa;
    double ya;
    double rota;
    double r;
    int hp;
    
    int active;
    int hit;
    
    int xTileO;
    int yTileO;
    
    bool jump_enabled;
    bool on_ladder;
    bool gravity_active;
    bool removed;
    bool grounded;
    bool turn_left;
    bool turn_right;
    bool moving;
    
    int move_up;
    double jump_speed;
    double gravity;
    
    struct CAnimation *current_animation;
    struct CAnimationFrame *current_animation_frame;
    
};


void cEntityCleanup(void);
struct CEntity *cEntityNew(int _id);
void cEntityUpdate(struct CEntity *e, double dt);
void cEntityUpdatePos(struct CEntity *e);
void cEntityRemove(struct CEntity *e);
int cEntityInBounds(int x, int y);
void cEntityMove(struct CEntity *e, double dt);
void cEntityRenderBoundingBox(unsigned int **raster, struct CEntity *e, int width, int height, int camera_x, int camera_y);
int cEntityIsFree(struct CEntity *e, double xx, double yy);
void cEntityCollide(struct CEntity *a, struct CEntity *b);
int cEntityIntersects(struct CEntity* e, double x1,
                      double y1, double radius);
struct CListQueue *cEntityGetHashList(int x, int y);
struct CAnimationFrame *cEntityGetAnimationFrame(struct CEntity *e, double dt);


#endif
