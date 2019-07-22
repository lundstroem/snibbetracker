//
//  CEntity.c
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "CEntity.h"
#include "CListQueue.h"
#include "CEngine.h"
#include "CAnimation.h"
#include "CAllocator.h"

#define PLAYER 0
#define ENEMY 1
#define PLAYER_PROJECTILE 2
#define ENEMY_PROJECTILE 3
#define PARTICLE 4
#define GOLD 5
#define BLOCK 7

// collision types
#define COL_FREE 0
#define COL_ENVIRONMENT 1
#define COL_ENTITY 2

#define LEVEL_WIDTH 64
#define LEVEL_HEIGHT 64

#define TILE_SIZE 16

struct CListQueue ***hashes = NULL;

struct CEntity *cEntityNew(int type)
{
    struct CEntity *e = cAllocatorAlloc(sizeof(struct CEntity), "CEntity");
    
    e->sheet_x = 0;
    e->sheet_y = 14;
    e->type = type;
    
    e->collision_radius = 0.4;
    e->player_shooting = 0;
    e->ENTITY_COLLISION = 1;
    e->speed = 2;
    e->color = 0xffffff;
    e->dir_x = 0;
    e->dir_y = 0;
    
    e->g_x = 1;
    e->g_y = 0;
   
    e->vec_x = 0;
    e->vec_y = 0;
    
    e->x = 0;
    e->y = 0;
    e->rot = 0;
    e->xa = 0;
    e->ya = 0;
    e->rota = 0;
    e->r = 0.4;
    e->hp = 0;
    
    e->active = true;
    e->hit = 0;
    e->xTileO = -1;
    e->yTileO = -1;
    
    e->jump_enabled = false;
    e->on_ladder = false;
    e->gravity_active = false;
    e->removed = false;
    
    e->grounded = false;
    e->turn_left = false;
    e->turn_right = false;
    e->moving = false;
    
    e->move_up = false;
    e->jump_speed = 0;
    e->gravity = 0.2;
    
    e->current_animation = NULL;
    e->current_animation_frame = NULL;
    
    cEntityUpdatePos(e);
    
    return e;
}


void cEntityUpdate(struct CEntity *e, double dt) {
    
    if (e->current_animation != NULL) {
        cAnimationAdvance(dt, e->current_animation);
        e->current_animation_frame = cAnimationGetCurrentFrame(e->current_animation);
        //printf("current frame: %d %d", e->current_animation_frame->x, e->current_animation_frame->y);
    }
}

/*
 Entity.prototype.setMoveTarget = function(target_x, target_y) {
 var g_vec_x = target_x - this.x;
 var g_vec_y = target_y - this.y;
 var scale_factor = 1.0;
 var length = Math.sqrt( g_vec_x*g_vec_x + g_vec_y*g_vec_y );
 g_vec_x = (g_vec_x/length) * scale_factor;
 g_vec_y = (g_vec_y/length) * scale_factor;
 this.vec_x = g_vec_x;
 this.vec_y = g_vec_y;
	}
 */

struct CListQueue *cEntityGetHashList(int x, int y)
{
    if(hashes == NULL) {
        hashes = cAllocatorAlloc(LEVEL_WIDTH * sizeof(struct CListQueue**), "CListQueue hashlist");
        if(hashes == NULL) {
            fprintf(stderr, "getHashList out of memory\n");
        }
        int i;
        for(i = 0; i < LEVEL_WIDTH; i++) {
            if(hashes != NULL)
            {
                hashes[i] = cAllocatorAlloc(LEVEL_HEIGHT * sizeof(struct CListQueue*), "CListQueue hashlist");
                if(hashes[i] == NULL)
                {
                    fprintf(stderr, "getHashList out of memory\n");
                }
            }
        }
        
        for(int s_x = 0; s_x < LEVEL_WIDTH; s_x++) {
            for(int s_y = 0; s_y < LEVEL_HEIGHT; s_y++) {
                hashes[s_x][s_y] = NULL;
            }
        }
    }
    
    if(hashes[x][y] == NULL) {
        hashes[x][y] = cListQueueNew();
    }
        
    return hashes[x][y];
}

struct CAnimationFrame *cEntityGetAnimationFrame(struct CEntity *e, double dt) {
    
    //return cAnimationGetCurrentFrame(e->current_animation);
    return e->current_animation_frame;
}

void cEntityRemove(struct CEntity *e)
{
    struct CListQueue *q = cEntityGetHashList(e->xTileO, e->yTileO);
    cListQueueRemoveNode(q, e, 0);
}
 
    /*
    public GDAnimationFrame getFrame(int delta) {
        return null;
    }
    
    public boolean grounded = false;
    public boolean move_up = false;
    public double jump_speed = 0;
    public double gravity = 0.2;
    */



void environmentCollision(void) {
    
    //printf("environment collision\n");
}

void cEntityCleanup(void) {
    if(hashes != NULL) {
        for(int s_x = 0; s_x < LEVEL_WIDTH; s_x++) {
            for(int s_y = 0; s_y < LEVEL_HEIGHT; s_y++) {
                if(hashes[s_x][s_y] != NULL) {
                    cListQueueClearListQueue(hashes[s_x][s_y], 0);
                    hashes[s_x][s_y] = cAllocatorFree(hashes[s_x][s_y]);
                }
            }
            hashes[s_x] = cAllocatorFree(hashes[s_x]);
        }
        hashes = cAllocatorFree(hashes);
    }
}



void cEntityMove(struct CEntity *e, double dt) {
      
    e->ya = 0;
    e->xa = 0;
    
    if(e->gravity_active && e->grounded && e->move_up) {
        e->jump_speed = 6;
        e->grounded = false;
    }
    
    e->xa += e->vec_x;
    e->ya += e->vec_y;
    
    // player can cancel the jump by not pushing up and make
    // a small jump instead.
    if(e->jump_enabled) {
        if(!e->move_up && e->jump_speed > 1) {
            e->jump_speed = 1;
        }
    }
    
    /*
     if (is_in_air) {
     y += (speed_y - 0.5*acceleration*delta_time)*delta_time;
     speed_y -= acceleration*delta_time;
     }
     */
    
    
    // if the entity should be affected by gravity.
    if(e->gravity_active && !e->on_ladder) {
        e->jump_speed -= e->gravity;
        e->ya -= e->jump_speed;
    } else {
    }
    
    int i;

    int xSteps = (int) (abs((int)e->xa * 100) + 1);
    for (i = xSteps; i > 0; i--) {
        double xxa = e->xa;
        int col_type = cEntityIsFree(e, e->x + xxa * i / xSteps, e->y);
        if (col_type == COL_FREE) {
            e->x += xxa * i / xSteps;
            break;
        }
        if (col_type == COL_ENTITY) {
            e->xa = 0;
        }
        else if(col_type == COL_ENVIRONMENT) {
            environmentCollision();
            e->xa = 0;
        }
    }
    
    e->grounded = false;
    int ySteps = (int) (abs((int)e->ya * 100) + 1);
    for (i = ySteps; i > 0; i--) {
        double yya = e->ya;
        int col_type = cEntityIsFree(e, e->x, e->y + yya * i / ySteps);
        if (col_type == COL_FREE) {
            e->y += yya * i / ySteps;
            break;
        } else if(col_type == COL_ENTITY) {
            //falling
            if(e->jump_enabled) {
                if(e->jump_speed < 0) {
                    e->jump_speed = 0;
                    e->grounded = true;
                }
                else {
                    e->jump_speed = 0;
                }
            }
        } else if(col_type == COL_ENVIRONMENT) {
            environmentCollision();
            //falling
                if(e->jump_speed < 0) {
                    e->jump_speed = 0;
                    e->grounded = true;
                } else {
                    e->jump_speed = 0;
                }
        }
    }
    
    e->vec_x = 0;
    e->vec_y = 0;
    e->on_ladder = false;
    
    cEntityUpdatePos(e);
}
       
void cEntityUpdatePos(struct CEntity *e) {
    
    int xTile = (int) (e->x/TILE_SIZE + 0.5);
    int yTile = (int) (e->y/TILE_SIZE + 0.5);
    
    if (xTile != e->xTileO || yTile != e->yTileO) {
        if(cEntityInBounds(e->xTileO, e->yTileO)) {
            struct CListQueue *q = cEntityGetHashList(e->xTileO, e->yTileO);
            cListQueueRemoveNode(q, e, 0);
        }
        
        e->xTileO = xTile;
        e->yTileO = yTile;
        
        if(cEntityInBounds(e->xTileO, e->yTileO)) {
            if (!e->removed) {
                struct CListQueue *q = cEntityGetHashList(e->xTileO, e->yTileO);
                cListQueueInsert(q, e, 1);
            }
        }
    }
}

int cEntityInBounds(int x, int y) {
    
    if(x > -1 && y > -1 && x < LEVEL_WIDTH && y < LEVEL_HEIGHT) {
       // printf("jEntity in bounds: %i %i \n", x, y);
        return 1;
    } else {
       // printf("jEntity not in bounds: %i %i \n", x, y);
        return 0;
    }
}

void cEntityRenderBoundingBox(unsigned int **raster, struct CEntity *e, int width, int height, int camera_x, int camera_y) {
    
   // double r = 0.45;
    // create the bounding box of this entity
    /*
    int x0 = e->x+8-4;
    int x1 = e->x+5;
    int y0 = e->y+8-7;
    int y1 = e->y+8+8;*/

    double r = 0.45;
   // double r2 = 0.2;
    // create the bounding box of this entity
    double d_x0 = ((e->x/TILE_SIZE) + 0.5 - r);
    double d_x1 = ((e->x/TILE_SIZE) + 0.5 + r);
    double d_y0 = ((e->y/TILE_SIZE) + 0.5 - r);
    double d_y1 = ((e->y/TILE_SIZE) + 0.5 + r);

    d_x0 *= TILE_SIZE;
    d_x1 *= TILE_SIZE;
    d_y0 *= TILE_SIZE;
    d_y1 *= TILE_SIZE;
    
    d_x0 -= camera_x;
    d_x1 -= camera_x;
    d_y0 -= camera_y;
    d_y1 -= camera_y;
    
    int x0 = (int)(d_x0);
    int x1 = (int)(d_x1);
    int y0 = (int)(d_y0);
    int y1 = (int)(d_y1);
    
    x1 += 1;
    y1 += 1;
    /*
    if(x0 > -1 && x0 < width && y0 > -1 && y0 < height) {
        raster[x0][y0] = 0xFFFFFFEE;
    }
    if(x1 > -1 && x1 < width && y0 > -1 && y0 < height) {
        raster[x1][y0] = 0xFFFFFFEE;
    }
    if(x0 > -1 && x0 < width && y1 > -1 && y1 < height) {
        raster[x0][y1] = 0xFFFFFFEE;
    }
    if(x1 > -1 && x1 < width && y1 > -1 && y1 < height) {
        raster[x1][y1] = 0xFFFFFFEE;
    }*/
    
    for (int i_x = x0; i_x < x1; i_x++) {
        for (int i_y = y0; i_y < y1; i_y++) {
            if(i_x > -1 && i_x < width && i_y > -1 && i_y < height) {
                raster[i_x][i_y] = 0xFFFFFFEE;
            }
        }
    }
}

int cEntityIsFree(struct CEntity *e, double xx, double yy) {
    
    double r = 0.45;
  //  double r2 = 0.2;
    // create the bounding box of this entity
    int x0 = (int) (floor((xx/TILE_SIZE) + 0.5 - r));
    int x1 = (int) (floor((xx/TILE_SIZE) + 0.5 + r));
    int y0 = (int) (floor((yy/TILE_SIZE) + 0.5 - r));
    int y1 = (int) (floor((yy/TILE_SIZE) + 0.5 + r));
    
    
    /*
    if(type == PARTICLE) {
        x0 = (int) (Math.floor((xx/TILE_SIZE) + 0.5 - r));
        x1 = (int) (Math.floor((xx/TILE_SIZE) + 0.5 - r));
        y0 = (int) (Math.floor((yy/TILE_SIZE) + 0.5 - r));
        y1 = (int) (Math.floor((yy/TILE_SIZE) + 0.5 - r));
    }
     */
    
    // if this entity is out of bounds, inactivate it and return.
    if(!cEntityInBounds(x0, y0)) { /*e->active = 0;*/ return COL_FREE; }
    if(!cEntityInBounds(x1, y0)) { /*e->active = 0;*/ return COL_FREE; }
    if(!cEntityInBounds(x0, y1)) { /*e->active = 0;*/ return COL_FREE; }
    if(!cEntityInBounds(x1, y1)) { /*e->active = 0;*/ return COL_FREE; }
    
    // check if projectiles intersect with blue blocks in
    // the environment.
    /*
    if(type == PLAYER_PROJECTILE || type == ENEMY_PROJECTILE) {
        if (level[x0][y0] == LevelBlocks.BLUE_BLOCK && active) {
            RasterPlatformerComponent.changeLevelTile(x0, y0, 0);
            return COL_ENVIRONMENT;
        }
        if (level[x1][y0] == LevelBlocks.BLUE_BLOCK && active) {
            RasterPlatformerComponent.changeLevelTile(x1, y0, 0);
            return COL_ENVIRONMENT;
        }
        if (level[x0][y1] == LevelBlocks.BLUE_BLOCK && active) {
            RasterPlatformerComponent.changeLevelTile(x0, y1, 0);
            return COL_ENVIRONMENT;
        }
        if (level[x1][y1] == LevelBlocks.BLUE_BLOCK && active) {
            RasterPlatformerComponent.changeLevelTile(x1, y1, 0);
            return COL_ENVIRONMENT;
        }
        return COL_FREE;
    }
     */
    
    struct CAnimation ***level = cEngineGetLevel();
    
    // if any other collision with environment.
    if (level[x0][y0]->collision_id > 0) return COL_ENVIRONMENT;
    if (level[x1][y0]->collision_id > 0) return COL_ENVIRONMENT;
    if (level[x0][y1]->collision_id > 0) return COL_ENVIRONMENT;
    if (level[x1][y1]->collision_id > 0) return COL_ENVIRONMENT;
    
    /*
    if(type == PARTICLE) {
        // particles can still collide with environment, but not other entities
        // so if this entity is a particle and we have not encountered
        // environment collision yet, return.
        return COL_FREE;
    }
     */
    
    // check if this entity is colliding with another entity.
    
    int xc = (int) (floor(xx/TILE_SIZE + 0.5));
    int yc = (int) (floor(yy/TILE_SIZE + 0.5));
    int rr = 2;
    
    for (int z = yc - rr; z <= yc + rr; z++) {
        for (int x = xc - rr; x <= xc + rr; x++) {
            if(cEntityInBounds(x, z)) {
                //struct CListQueue *es = hashes[x][z];
                struct CListQueue *es = hashes[x][z];
                if(es != NULL) {
                    struct CLinearNode *node = es->first;
                    while(node != NULL) {
                        struct CEntity *e_list = node->data;
                        
                        if (e_list != e) {
                            // entity cannot intersect with itself.
                        
                            if(cEntityIntersects(e, e_list->x, e_list->y, e_list->collision_radius)) {
                                cEngineEntityCollisionCallback(e_list, e);
                                return COL_FREE;
                            }
                        }
                        node = node->next;
                    }
                }
            }
        }
    }
    
    
    return COL_FREE;
}

void cEntityCollide(struct CEntity *a, struct CEntity *b) {
    
    
}

int cEntityIntersects(struct CEntity* e, double x1, double y1, double radius) {
    
    double r = TILE_SIZE*radius;
    if (e->x + r <= x1) return 0;
    if (e->x - r >= x1+r) return 0;
    if (e->y + r <= y1) return 0;
    if (e->y - r >= y1+r) return 0;
    return 1;
}

/*

    protected int isFree(double xx, double yy) {
        
        if(type == GOLD || type == BLOCK) {
        	// exclude these types of entity entirely.
        	// the blue blocks only use environment collision
        	// so we only have to check if the projectiles intersect
        	// with that kind of environment.
        	
            return COL_FREE;
        }
        
        // create the bounding box of this entity
        int x0 = (int) (Math.floor((xx/TILE_SIZE) + 0.5 - 0.2));
        int x1 = (int) (Math.floor((xx/TILE_SIZE) + 0.5 + 0.2));
        int y0 = (int) (Math.floor((yy/TILE_SIZE) + 0.5 - r));
        int y1 = (int) (Math.floor((yy/TILE_SIZE) + 0.5 + r));
        
        if(type == PARTICLE) {
            x0 = (int) (Math.floor((xx/TILE_SIZE) + 0.5 - r));
            x1 = (int) (Math.floor((xx/TILE_SIZE) + 0.5 - r));
            y0 = (int) (Math.floor((yy/TILE_SIZE) + 0.5 - r));
            y1 = (int) (Math.floor((yy/TILE_SIZE) + 0.5 - r));
        }
        
        // if this entity is out of bounds, inactivate it and return.
        if(!inBounds(x0, y0)) { active = false; return COL_FREE; }
        if(!inBounds(x1, y0)) { active = false; return COL_FREE; }
        if(!inBounds(x0, y1)) { active = false; return COL_FREE; }
        if(!inBounds(x1, y1)) { active = false; return COL_FREE; }
        
        // check if projectiles intersect with blue blocks in
        // the environment.
        if(type == PLAYER_PROJECTILE || type == ENEMY_PROJECTILE) {
            if (level[x0][y0] == LevelBlocks.BLUE_BLOCK && active) {
                RasterPlatformerComponent.changeLevelTile(x0, y0, 0);
                return COL_ENVIRONMENT;
            }
            if (level[x1][y0] == LevelBlocks.BLUE_BLOCK && active) {
                RasterPlatformerComponent.changeLevelTile(x1, y0, 0);
                return COL_ENVIRONMENT;
            }
            if (level[x0][y1] == LevelBlocks.BLUE_BLOCK && active) {
                RasterPlatformerComponent.changeLevelTile(x0, y1, 0);
                return COL_ENVIRONMENT;
            }
            if (level[x1][y1] == LevelBlocks.BLUE_BLOCK && active) {
                RasterPlatformerComponent.changeLevelTile(x1, y1, 0);
                return COL_ENVIRONMENT;
            }
            return COL_FREE;
        }
        
        // if any other collision with environment.
        if (level[x0][y0] > Block.EMPTY) return COL_ENVIRONMENT;
        if (level[x1][y0] > Block.EMPTY) return COL_ENVIRONMENT;
        if (level[x0][y1] > Block.EMPTY) return COL_ENVIRONMENT;
        if (level[x1][y1] > Block.EMPTY) return COL_ENVIRONMENT;
        
        if(type == PARTICLE) {
            // particles can still collide with environment, but not other entities
            // so if this entity is a particle and we have not encountered
        	// environment collision yet, return.
            return COL_FREE;
        }
        
        // check if this entity is colliding with another entity.
        int xc = (int) (Math.floor(xx/TILE_SIZE + 0.5));
        int yc = (int) (Math.floor(yy/TILE_SIZE + 0.5));
        int rr = 2;
        
        for (int z = yc - rr; z <= yc + rr; z++) {
            for (int x = xc - rr; x <= xc + rr; x++) {
                if(inBounds(x, z)) {
                    List<Entity> es = hashes[x][z].list;
                    for (int i = 0; i < es.size(); i++) {
                        Entity e = es.get(i);
                        if (e == this) {
                        	// entity cannot intersect with itself, skip.
                        	continue;
                        }
                        if(intersects(e.x, e.y, e.collision_radius)) {
                            e.collide(this);
                            this.collide(e);
                            return COL_FREE;
                        }
                    }
                }
            }
        }
        
        return COL_FREE;
    }
*/
/*
    
    
    public void update(int delta) {
    	// base class function.
    }
    
    public boolean intersects(double x1,
                              double y1, double radius) {
        double r = TILE_SIZE*radius;
        if (x + r <= x1) return false;
        if (x - r >= x1+r) return false;
        if (y + r <= y1) return false;
        if (y - r >= y1+r) return false;
        return true;
    }
    */
    // to make enemies move towards the player.

/*
    public void moveToPlayer(int _scale_factor) {
        double g_vec_x = RasterPlatformerComponent.instance.player.x - x;
        double g_vec_y = RasterPlatformerComponent.instance.player.y - y;
        double scale_factor = _scale_factor;
        double length = Math.sqrt( g_vec_x*g_vec_x + g_vec_y*g_vec_y );
        float f_length = (float)length;
        g_vec_x = (g_vec_x/f_length) * scale_factor;
        g_vec_y = (g_vec_y/f_length) * scale_factor;
        xa = g_vec_x;
        ya = g_vec_y;
    }
    
    public void activateEnemyProjectile(double dest_x, double dest_y) {
        double g_vec_x = dest_x - x;
        double g_vec_y = dest_y - y;
        double scale_factor = 1f;
        double length = Math.sqrt( g_vec_x*g_vec_x + g_vec_y*g_vec_y );
        float f_length = (float)length;
        g_vec_x = (g_vec_x/f_length) * scale_factor;
        g_vec_y = (g_vec_y/f_length) * scale_factor;
        EnemyProjectile e = new EnemyProjectile(x, y, g_vec_x, g_vec_y, 0xFF00FF);
        RasterPlatformerComponent.addEntity(e);
    }
 */


// Not implemented/ Untested methods
//public boolean blocks(Entity entity, double x2, double z2, double r2)
//{
//  if(entity == this) {
//      return false;
//  }
//
//  double r = TILE_SIZE*0.4;
//  if (x + r <= x2 - r2) return false;
//  if (x - r >= x2 + r2) return false;
//  if (z + r <= z2 - r2) return false;
//  if (z - r >= z2 + r2) return false;
//  return true;
//}

//public boolean contains(double x2, double z2) {
//  if (x + r <= x2) return false;
//  if (x - r >= x2) return false;
//  if (z + r <= z2) return false;
//  if (z - r >= z2) return false;
//  return true;
//}

//public boolean isInside(double x0, double z0, double x1, double z1) {
//  if (x + r <= x0) return false;
//  if (x - r >= x1) return false;
//  if (z + r <= z0) return false;
//  if (z - r >= z1) return false;
//  return true;
//}


