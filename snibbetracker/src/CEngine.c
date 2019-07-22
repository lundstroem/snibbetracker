//
//  CEngine.c
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CEngine.h"
#include "CListQueue.h"
#include "CEntity.h"
#include "CButton.h"
#include "CAnimationFrame.h"
#include "CLabel.h"
#include "CAnimation.h"
#include "CTouch.h"
#include "CAllocator.h"
#include "CSynth.h"


//#define use_hooks
/* add this for game logic hooks */
//#include "Game.h"


/* task list
 
 
done
- fps monitoring (with CLabel)
- entity collision (with id's/callback)
- performance test on hardware
- separate game logic from template/engine
- need to make screen size settings configurable in each client, outside of the engine.

-------------
 
- investigate possibility for a toggle for portrait/landscape. 
 if not, make a new template (with engine in separate repo). 
 Create new project/repo for each game to not have to switch provision/assets etc.
 

- test listqueue resize

 
 
 */


//static int cEngineTileCoordId(int choord_x, int choord_y);
//static int cEngineIsValidTileCoord(int choord_x, int choord_y);
//static void cEngineprocessDynamicColMap();
static void cEngineShowFPS(double dt, unsigned int **raster);
static void cEngineInitData(void);
static void cEngineRender(double dt, unsigned int **raster, struct CInput *input);
static void cEngineRenderEntityList(double dt, unsigned int **raster);
static void cEngineRenderGround(double dt, unsigned int **raster);
static void cEngineUpdate(double dt, struct CInput *input);
static void cEngineClearScreen(unsigned int **raster);
static void cEngineRenderGUI(unsigned int **raster);
static void cEngineRenderSprite(unsigned int **raster, int sprite_x, int sprite_y,
                         int screen_x, int screen_y
                         ,unsigned char camera_offset,
                         int width, int height, unsigned char hit, unsigned int color, unsigned int bg_color);

static struct CListQueue* entity_list = NULL;
static unsigned int **sprite_pixel_data = NULL;
static struct CAnimation ***level = NULL;
static int camera_x = 0;
static int camera_y = 0;
static struct CButton **buttons = NULL;
static struct CEngineContext *engine = NULL;

void cEngineInit(struct CEngineContext *e) {
    

    engine = e;
    
    cEngineInitData();
    entity_list = cListQueueNew();
    srand ( 666 );

    // custom init
    if(engine->cEngineInitHook != NULL) {
        engine->cEngineInitHook(e);
    }
}

struct CEngineContext *cEngineContextNew(void) {
    
    struct CEngineContext *c = cAllocatorAlloc(sizeof(struct CEngineContext), "CEngineContext");
    c->width = 144;
    c->height = 256;
    c->sprite_size = 16;
    c->sheet_width = 1024;
    c->sheet_height = 1024;
    c->max_touches = 8;
    c->level_width = 64;
    c->level_height = 64;
    c->max_buttons = 10;
    // TODO use these in rendering.
    c->color_mode_rgba = true;
    c->color_mode_argb = false;
    c->clear_color = 0x000000FF;
    c->render_bounding_boxes = false;
    c->show_fps = false;
    c->input = NULL;
    c->sfx_queue = NULL;
    c->ground_render_enabled = true;
    c->cEngineInitHook = NULL;
    c->cEnginePreUpdateHook = NULL;
    c->cEngineUpdateHook = NULL;
    c->cEnginePreEntityRenderHook = NULL;
    c->cEngineRenderHook = NULL;
    c->cEngineEntityCollisionHook = NULL;
    c->cEngineEnvironmentCollisionHook = NULL;
    c->cEngineCleanupHook = NULL;
    return c;
}

void cEngineLog(char *string) {
    printf("cEngineLog:%s\n", string);
}

void cEngineCleanup(void) {
    
    if(engine->cEngineCleanupHook != NULL) {
        engine->cEngineCleanupHook();
    }
   
    for(int i = 0; i < engine->max_buttons; i++) {
        if(buttons != NULL) {
            cButtonCleanup(buttons[i]);
            buttons[i] = cAllocatorFree(buttons[i]);
        }
    }
    buttons = cAllocatorFree(buttons);
    
    for(int i = 0; i < engine->sheet_width; i++) {
        if(sprite_pixel_data != NULL) {
            sprite_pixel_data[i] = cAllocatorFree(sprite_pixel_data[i]);
        }
    }
    sprite_pixel_data = cAllocatorFree(sprite_pixel_data);
    
    
    for(int s_x = 0; s_x < engine->level_width; s_x++) {
        for(int s_y = 0; s_y < engine->level_height; s_y++) {
            if(level != NULL && level[s_x] != NULL) {
                level[s_x][s_y] = cAnimationFree(level[s_x][s_y]);
            }
        }
    }
    for(int i = 0; i < engine->level_width; i++) {
        if(level != NULL) {
            level[i] = cAllocatorFree(level[i]);
        }
    }
    level = cAllocatorFree(level);
    
    cListQueueClearListQueue(entity_list, 1);
    entity_list = cAllocatorFree(entity_list);
    
    cListQueueClearListQueue(engine->sfx_queue, 1);
    engine->sfx_queue = cAllocatorFree(engine->sfx_queue);
    
    cInputCleanup(engine->input, engine->max_touches);
    engine->input = NULL;
    
    cEntityCleanup();
    cListQueueCleanup();
    
    if(engine != NULL) {
        engine = cAllocatorFree(engine);
    }
}

void cEngineDrawArea(int x, int y, int w, int h, unsigned int color, unsigned int **raster2d) {
    
    if (x > -1 && y > -1 && x < engine->width && y < engine->height
        && x+w > -1 && y+h > -1 && x+w < engine->width && y+h < engine->height) {
        for (int r_x = x; r_x < x+w; r_x++) {
            for (int r_y = y; r_y < y+h; r_y++) {
                raster2d[r_x][r_y] = color;
            }
        }
    }
}

void cEngineInitData(void) {
    
    // alloc buttons
    buttons = cAllocatorAlloc(engine->max_buttons * sizeof(struct CButton*), "CButton");
    if(buttons == NULL) {
        fprintf(stderr, "buttons out of memory\n");
    }
    
    int i;
    
    for(i = 0; i < engine->max_buttons; i++) {
        if(buttons != NULL) {
            buttons[i] = cButtonNew();
            if(buttons[i] == NULL) {
                fprintf(stderr, "buttons out of memory\n");
            }
        }
    }
    
    // alloc sprite sheet
    sprite_pixel_data = cAllocatorAlloc(engine->sheet_width * sizeof(unsigned int*), "sprite pixel data");
	if(sprite_pixel_data == NULL) {
		fprintf(stderr, "sheet out of memory\n");
	}
	for(i = 0; i < engine->sheet_width; i++) {
        if(sprite_pixel_data != NULL) {
            sprite_pixel_data[i] = cAllocatorAlloc(engine->sheet_height * sizeof(unsigned int), "sprite pixel data");
            if(sprite_pixel_data[i] == NULL)
            {
                fprintf(stderr, "sheet out of memory\n");
            }
        }
	}
    
    if(sprite_pixel_data != NULL) {
        for(int x = 0; x < engine->sheet_width; x++) {
            for(int y = 0; y < engine->sheet_height; y++) {
                if(sprite_pixel_data[x] != NULL) {
                    sprite_pixel_data[x][y] = 0;
                }
            }
        }
    }
        
    if(level == NULL) {
        level = cAllocatorAlloc(engine->level_width * sizeof(struct CAnimation**), "CAnimations");
        if(level == NULL) {
            fprintf(stderr, "level out of memory\n");
        }
        int i;
        for(i = 0; i < engine->level_width; i++) {
            if(level != NULL)
            {
                level[i] = cAllocatorAlloc(engine->level_height * sizeof(struct CAnimation*), "CAnimation");
                if(level[i] == NULL)
                {
                    fprintf(stderr, "level out of memory\n");
                }
            }
        }
        
        for(int s_x = 0; s_x < engine->level_width; s_x++) {
            for(int s_y = 0; s_y < engine->level_height; s_y++) {
                if(level != NULL && level[s_x] != NULL) {
                    level[s_x][s_y] = NULL;
                }
            }
        }
    }
    
    engine->input = cInputNew();
    
    engine->input->touches = cAllocatorAlloc(engine->max_touches * sizeof(struct CTouch*), "CTouch array 1");
    if(engine->input->touches == NULL) {
        fprintf(stderr, "touchlist out of memory\n");
    }

    for(i = 0; i < engine->max_touches; i++) {
        if(engine->input->touches != NULL) {
             engine->input->touches[i] = cTouchNew();
            if( engine->input->touches[i] == NULL) {
                fprintf(stderr, "touchlist out of memory\n");
            }
        }
    }
    
    engine->input->ended_touches = cAllocatorAlloc(engine->max_touches * sizeof(struct CTouch*), "CTouch array 2");
    if(engine->input->ended_touches == NULL) {
        fprintf(stderr, "ended touchlist out of memory\n");
    }
    for(i = 0; i < engine->max_touches; i++) {
        if(engine->input->ended_touches != NULL) {
            engine->input->ended_touches[i] = cTouchNew();
            if(engine->input->ended_touches[i] == NULL) {
                fprintf(stderr, "ended touchlist out of memory\n");
            }
        }
    }
    
    engine->sfx_queue = cListQueueNew();
}

struct CPoint *cNewPoint(int x, int y) {
   
    struct CPoint *p = cAllocatorAlloc(sizeof(struct CSound), "CPoint");
    p->x = x;
    p->y = y;
    return p;
}

struct CVec2 *cNewVec2(double x, double y) {
    
    struct CVec2 *p = cAllocatorAlloc(sizeof(struct CVec2), "CVec2");
    p->x = x;
    p->y = y;
    return p;
}


struct CSound *cSoundNew(int type) {
    
    struct CSound *s = cAllocatorAlloc(sizeof(struct CSound), "CSound");
    s->type = type;
    return s;
}

void cEngineAddSfxToQueue(int type) {
    
    cListQueueInsert(engine->sfx_queue, cSoundNew(type), false);
}

struct CListQueue *cEngineGetSfxQueue(void) {
    
    return engine->sfx_queue;
}

struct CButton* cEngineGetButton(int i) {
    
    if(i >= engine->max_buttons || i < 0) {
        printf("cannot get button of index:%d\n", i);
        return NULL;
    } else {
        return buttons[i];
    }
}

struct CAnimation*** cEngineGetLevel(void) {
    
    return level;
}

struct CListQueue* cEngineGetEntityList(void) {
    
    return entity_list;
}

// only used for init
void cEngineWritePixelData(unsigned int* ptr) {
    
    for (int s_x = 0; s_x < engine->sheet_width; s_x++) {
        for (int s_y = 0; s_y < engine->sheet_height; s_y++) {
            //sprite_pixel_data[s_x][s_y] = 0xFFFFFF00;
            sprite_pixel_data[s_x][s_y] = ptr[s_x+s_y*engine->sheet_width];
        }
    }
}

struct CEntity* cEngineAddEntity(int type, int tile_x, int tile_y) {
    
    struct CEntity *e = cEntityNew(type);
    e->x = tile_x * engine->sprite_size;
    e->y = tile_y * engine->sprite_size;
    cListQueueInsert(entity_list, e, 0);
    return e;
}

// called from CEntity.
void cEngineEntityCollisionCallback(struct CEntity *a, struct CEntity *b) {

    if(engine->cEngineEntityCollisionHook != NULL) {
        engine->cEngineEntityCollisionHook(a, b);
    }
}

void cEngineUpdateInput(double dt, unsigned int **raster) {
    cEngineUpdate(dt, engine->input);
}
void cEngineGameloop(double dt, unsigned int **raster) {
    cEngineRender(dt, raster, engine->input);
    if(engine->render_bounding_boxes) {
        cEngineRenderCollisionAreas(dt, raster);
    }
}

void cEngineResetButtons(void) {
    
    for(int i = 0; i < engine->max_buttons; i++) {
        struct CButton *b = buttons[i];
        if(!b->spinner_button_active && !b->dragable) {
            b->pressed = false;
            b->touched_up_inside = false;
        }
    }
}

void cEngineUpdate(double dt, struct CInput *input) {
    
    cEngineResetButtons();
    for(int i = 0; i < engine->max_touches; i++) {
        struct CTouch *t = input->touches[i];
        if(t->active) {
            for(int a = 0; a < engine->max_buttons; a++) {
                struct CButton *b = buttons[a];
                if(b->active && !b->hidden) {
                    if(t->spinner_button_id == -1 && t->dragable_button_id == -1) {
                        if(cButtonIsInside(b, t->x, t->y)) {
                            if(b->spinner_button /*&& t->began*/) {
                                b->spinner_button_pressed_x = t->x;
                                b->spinner_button_pressed_y = t->y;
                                b->spinner_button_active = true;
                                b->pressed = true;
                                t->spinner_button_id = a;
                                t->ms_since_init = 0;
                                //printf("spinner button init %d %d\n", t->x, t->y);
                            } if(b->dragable /*&& t->began*/) {
                                t->dragable_button_id = a;
                                b->pressed = true;
                                b->dragable_offset_x = t->x - b->x;
                                b->dragable_offset_y = t->y - b->y;
                                //printf("dragable button init %d %d\n", t->x, t->y);
                            } else if(!b->spinner_button && !b->dragable) {
                                b->pressed = true;
                            }
                        } else if(b->whole_screen) {
                            b->pressed = true;
                        }
                    } else if(b->spinner_button_active) {
                        t->ms_since_init += dt;
                        if(engine->cEngineSpinnerButtonMovedCallback != NULL) {
                            engine->cEngineSpinnerButtonMovedCallback(b, a, t->x, t->y);
                        }
                    } else if(b->dragable) {
                        // todo, make pressed x/y to offset touch to button move.
                        if(b->pressed) {
                            t->ms_since_init += dt;
                            b->x = t->x - b->dragable_offset_x;
                            b->y = t->y - b->dragable_offset_y;
                            //printf("dragable");
                            if(engine->cEngineDragableButtonMovedCallback != NULL) {
                                engine->cEngineDragableButtonMovedCallback(b, a, t->x, t->y);
                            }
                        }
                    }
                }
            }
            t->began = false;
        } else if(t->ended) {
            
            //spinner button
            if (t->spinner_button_id > -1) {
               
                buttons[t->spinner_button_id]->spinner_button_active = false;
                if(engine->cEngineSpinnerButtonEndedCallback != NULL) {
                    engine->cEngineSpinnerButtonEndedCallback(buttons[t->spinner_button_id], t->spinner_button_id, t->x, t->y, t->ms_since_init);
                }
                t->spinner_button_id = -1;
                //printf("spinner button active = false\n");
            } else if (t->dragable_button_id > -1) {
                
                
                buttons[t->dragable_button_id]->pressed = false;
               // buttons[t->spinner_button_id]->spinner_button_active = false;
                if(engine->cEngineDragableButtonEndedCallback != NULL) {
                    engine->cEngineDragableButtonEndedCallback(buttons[t->dragable_button_id], t->dragable_button_id, t->x, t->y, t->ms_since_init);
                }
                t->dragable_button_id = -1;
                //printf("dragable button active = false\n");
            } else {
                for(int a = 0; a < engine->max_buttons; a++) {
                    struct CButton *b = buttons[a];
                    if(b->active && !b->hidden /*&& !b->disabled*/) {
                        if(cButtonIsInside(b, t->x, t->y)) {
                            b->touched_up_inside = true;
                            b->toggled = !b->toggled;
                        } else if(b->whole_screen) {
                            b->touched_up_inside = true;
                            b->toggled = !b->toggled;
                        }
                    }
                }
            }
        }
    }
    
    if(engine->cEnginePreUpdateHook != NULL) {
        engine->cEnginePreUpdateHook(engine->input, buttons, dt);
    }
    
    struct CLinearNode *node = entity_list->first;
    while(node != NULL) {
        struct CEntity *e = node->data;
        cEntityUpdate(e, dt);
        cEntityMove(e, dt);
        node = node->next;
    }
    
    if(engine->cEngineUpdateHook != NULL) {
        engine->cEngineUpdateHook(dt);
    }
}

void cEngineCenterCamera(struct CEntity *e) {
    camera_x = (int)(e->x - engine->width/2 + engine->sprite_size);
    camera_y = (int)(e->y - engine->height/2 + engine->sprite_size);
}

void cEngineUpdateCamera(int x, int y) {
    camera_x = x;
    camera_y = y;
}

void cEngineRender(double dt, unsigned int **raster, struct CInput *input) {
    
    cEngineClearScreen(raster);
    if(engine->ground_render_enabled) {
        cEngineRenderGround(dt, raster);
    }
    
    if(engine->cEnginePreEntityRenderHook != NULL) {
        engine->cEnginePreEntityRenderHook(dt, raster);
    }
    
    cEngineRenderEntityList(dt, raster);
    cEngineRenderGUI(raster);
    
    if(engine->cEngineRenderHook != NULL) {
        engine->cEngineRenderHook(dt, raster);
    }
    
    if(engine->show_fps) {
        cEngineShowFPS(dt, raster);
    }
}

void cEngineShowFPS(double dt, unsigned int **raster) {
    
    static int frame_cache_counter = 0;
    static double cached_frame_time = 16;
    frame_cache_counter++;
    if(frame_cache_counter == 20) {
        cached_frame_time = dt;
        frame_cache_counter = 0;
    }
    
    char fps[20];
    double double_fps = 1000/cached_frame_time;
    sprintf(fps, "%f", double_fps);
    cEngineRenderLabelWithParams(raster, fps, 0, 0, 0x00FF00FF, 0x00FFFFFF);
}

void cEngineRenderEntitySprite(unsigned int **raster, int sprite_x, int sprite_y,
                         int screen_x, int screen_y,
                         int width, int height, bool camera_offset, unsigned char hit, unsigned int color, unsigned int bg_color, bool flipped_h, bool flipped_v) {
    
    screen_x *= width;
    screen_y *= height;
    
    for (int s_x = 0; s_x < width; s_x++) {
        for (int s_y = 0; s_y < height; s_y++) {
            int s_x_p = sprite_x+s_x;
            int s_y_p = sprite_y+s_y;
            int s_x_sheet_p = screen_x+s_x;
            int s_y_sheet_p = screen_y+s_y;
            if (flipped_h) {
                s_x_sheet_p = screen_x+width-s_x-1;
            }
            if (flipped_v) {
                s_y_sheet_p = screen_y+width-s_y-1;
            }
            if(camera_offset) {
                s_x_p -= camera_x;
                s_y_p -= camera_y;
            }
            if(s_x_p > -1 && s_y_p > -1 && s_x_p < engine->width && s_y_p < engine->height
               && s_x_sheet_p > -1 && s_y_sheet_p > -1 && s_x_sheet_p < engine->sheet_width && s_y_sheet_p < engine->sheet_height) {
               
                // TODO check alpha byte depending on setting (rgba/argb).
                unsigned int pixel_color = sprite_pixel_data[s_x_sheet_p][s_y_sheet_p];
                unsigned int alpha = pixel_color & 0xff;
                if(engine->color_mode_argb) {
                    alpha = (pixel_color >> (8*3)) & 0xff;
                }
                if(alpha > 0) {
                    raster[s_x_p][s_y_p] = pixel_color;
                }
            }
        }
    }
}


static void cEngineRenderSprite(unsigned int **raster, int sprite_x, int sprite_y,
                  int screen_x, int screen_y
                  ,unsigned char camera_offset,
                  int width, int height, unsigned char hit, unsigned int color, unsigned int bg_color) {
    
    unsigned int alpha = color & 0xff;
    unsigned int bg_alpha = bg_color & 0xff;
    if(engine->color_mode_argb) {
        alpha = (color >> (8*3)) & 0xff;
        bg_alpha = (bg_color >> (8*3)) & 0xff;
    }
    
    
   // if(alpha > 0) {
        screen_x *= width;
        screen_y *= height;
        
        for (int s_x = 0; s_x < width; s_x++) {
            for (int s_y = 0; s_y < height; s_y++) {
                //screen offsets
                int s_x_p = sprite_x+s_x;
                int s_y_p = sprite_y+s_y;
                
                //sheet offsets
                int s_x_sheet_p = screen_x+s_x;
                int s_y_sheet_p = screen_y+s_y;
                
                //camera offsets
                if(camera_offset) {
                    s_x_p -= camera_x;
                    s_y_p -= camera_y;
                }
                
                if(s_x_p > -1 && s_y_p > -1 && s_x_p < engine->width && s_y_p < engine->height
                   && s_x_sheet_p > -1 && s_y_sheet_p > -1 && s_x_sheet_p < engine->sheet_width && s_y_sheet_p < engine->sheet_height) {
                    unsigned int alpha2 = sprite_pixel_data[s_x_sheet_p][s_y_sheet_p] & 0xFF;
                    //check alpha
                    

                    // TODO make sprite render that disregards color and only reads from sheet.
                    if(alpha2 > 0) {
                        raster[s_x_p][s_y_p] = sprite_pixel_data[s_x_sheet_p][s_y_sheet_p];
                    } else {
                        if(bg_alpha > 0) {
                            raster[s_x_p][s_y_p] = bg_color;
                        }
                    
                        if(alpha2 > 0) {
                            raster[s_x_p][s_y_p] = color;
                        }
                    }
                }
                
                //raster[s_x][s_y] = 0xFF00FF00;
            }
        }
    //}
}

void cEngineClearScreen(unsigned int **raster) {
    
    for (int s_x = 0; s_x < engine->width; s_x++) {
        for (int s_y = 0; s_y < engine->height; s_y++) {
            //raster[s_x][s_y] = 0xFF000FF;
            raster[s_x][s_y] = engine->clear_color;
           // raster[s_x][s_y] = rand(); //green grass
        }
    }
}

void cEngineRenderGround(double dt, unsigned int **raster) {
    
    int camera_tile_x = camera_x/engine->sprite_size;
    int camera_tile_y = camera_y/engine->sprite_size;
    
    // render sprites
    for (int s_x = camera_tile_x; s_x < camera_tile_x+21; s_x++) {
        for (int s_y = camera_tile_y; s_y < camera_tile_y+16; s_y++) {
            if(s_x > -1 && s_y > -1 && s_x < engine->level_width && s_y < engine->level_height) {
                if(s_x > -1 && s_y > -1 && s_x < engine->level_width && s_y < engine->level_height) {
                    int tile_x = 0;
                    int tile_y = 0;
                    
                    struct CAnimation* ground = level[s_x][s_y];
                    if(ground != NULL) {
                        if(ground->nrOfFrames > 0) {
                            cAnimationAdvance(dt, ground);
                            tile_x = cAnimationGetCurrentFrame(ground)->x;
                            tile_y = cAnimationGetCurrentFrame(ground)->y;
                        } else {
                            tile_x = ground->default_frame.x;
                            tile_y = ground->default_frame.y;
                        }
                        
                        cEngineRenderSprite(raster, s_x*engine->sprite_size, s_y*engine->sprite_size, tile_x, tile_y, 1, engine->sprite_size, engine->sprite_size, 0, 0, 0);
                    }
                }
            }
        }
    }
}

void cEngineRenderEntityList(double dt, unsigned int **raster) {
    
    struct CLinearNode *node = entity_list->first;
    while(node != NULL) {
        struct CEntity *e = node->data;
        if(e->active) {
            struct CAnimationFrame *f = cEntityGetAnimationFrame(e, dt);
            if(f != NULL) {
                if(e->current_animation != NULL) {
                    // offset for larger sprite size so that ground collision is on the bottom.
                    int y_sprite = (int)e->y-(e->current_animation->size-16);
                    
                    //TODO: center ground tile depending on size, and also be able to render it for debug.
                    int x_sprite = (int)e->x-(e->current_animation->size/2)+10;
                    if(e->current_animation->flipped_h) {
                        x_sprite = (int)e->x-(e->current_animation->size/2)+6;
                    }
                    // TODO: be able to render all collision areas from the editor.
                    /*
                     move these vars to entity to set them outside of the engine.
                     make a separete offset for flipped_h.
                     */
                    
                    cEngineRenderEntitySprite(raster, x_sprite, y_sprite, f->x, f->y, f->size, f->size, true, (unsigned char)e->hit, -1, -1, e->current_animation->flipped_h, e->current_animation->flipped_v);
                } else {
                    // render frame?
                    if(e->current_animation_frame != NULL) {
                        cEngineRenderSprite(raster, (int)e->x, (int)e->y, e->current_animation_frame->x, e->current_animation_frame->y, true, 16, 16, 0, 0, 0);
                    }
                }
            } else {
                cEngineLog("frame for current entity is NULL\n");
            }
        }
        node = node->next;
    }
}

void cEngineRenderCollisionAreas(double dt, unsigned int **raster) {
    
    struct CLinearNode *node = entity_list->first;
    while(node != NULL) {
        struct CEntity *e = node->data;
        if(e->active) {
           // cEngineRenderSprite(raster, (int)e->x, (int)e->y, 0, 0, 1, 16, 16, 0, 0xFFFFFFEE, 0xFFFFFFEE);
            cEntityRenderBoundingBox(raster, e, engine->width, engine->height, camera_x, camera_y);
        }
        node = node->next;
    }
}

void cEngineRenderLabelWithParams(unsigned int **raster, char *string, int s_x, int s_y, unsigned int color, unsigned int bg_color) {
    
    // TODO set depending on alpha byte setting.
    unsigned int alpha = color & 0xff;
    if(engine->color_mode_argb) {
        alpha = (color >> (8*3)) & 0xff;
    }
    if(alpha > 0) {
        int len = (int)strlen(string);
        for(int i = 0; i < len; i++) {
            char c = string[i];
            int x_char_pos = cEngineGetCharPos(c);
            cEngineRenderSprite(raster, (s_x*8)+(i*8), s_y*12, x_char_pos, 0, 1, 8, 12, 0, color, bg_color);
        }
    }
}

void cEngineRenderLabel(unsigned int **raster, struct CLabel *l, unsigned int color, unsigned int bg_color) {
    
    int len = (int)cStrLength(l->str);
    
    if(l->scrolling == 1) {
        l->x--;
        if (l->x < -l->x_offset_max) {
            l->x = engine->width;
        }
    }
    
    for(int i = 0; i < len; i++) {
        char c = l->str->chars[i];
        int x_char_pos = cEngineGetCharPos(c);
        if(l->x+(i*8) > -engine->sprite_size && l->x < engine->width+engine->sprite_size) {
            cEngineRenderSprite(raster, l->x+(i*8), l->y, x_char_pos, 0, 1, 8, 12, 0, color, bg_color);
        }
    }
}

void cEngineRenderLabelByPixelPos(unsigned int **raster, char *string, int s_x, int s_y, unsigned int color, unsigned int bg_color) {
    
    // TODO set depending on alpha byte setting.
    unsigned int alpha = color & 0xff;
    if(engine->color_mode_argb) {
        alpha = (color >> (8*3)) & 0xff;
    }
    
    if(alpha > 0) {
        int len = (int)strlen(string);
        for(int i = 0; i <len; i++) {
            char c = string[i];
            int x_char_pos = cEngineGetCharPos(c);
            //    cEngineRenderSprite(raster, s_x+(i*8), s_y, x_char_pos, 0, 1, 8, 9, 0, 0xFF0000FF, 0x00FF00FF);
            cEngineRenderSprite(raster, s_x+(i*8), s_y, x_char_pos, 0, 1, 8, 12, 0, color, bg_color);
        }
    }
}


//int cEngineTileCoordId(int coord_x, int coord_y)
//{
//    if (coord_x < 0 || coord_y < 0 ||
//        coord_x >= LEVEL_WIDTH ||
//        coord_y >= LEVEL_HEIGHT) {
//        //LogTrace(@"tileCoordId out of range");
//        return 0;
//    } else {
//        /*
//         Entity *e = level[choord_x][choord_y];
//         
//         if(e != NULL)
//         return e.tile_id;
//         else return 0;
//         */
//        return 0;
//    }
//}

int cEngineGetCharPos(char c)
{
    switch (c) {
        case 'A': return 0; break;
        case 'B': return 1; break;
        case 'C': return 2; break;
        case 'D': return 3; break;
        case 'E': return 4; break;
        case 'F': return 5; break;
        case 'G': return 6; break;
        case 'H': return 7; break;
        case 'I': return 8; break;
        case 'J': return 9; break;
        case 'K': return 10; break;
        case 'L': return 11; break;
        case 'M': return 12; break;
        case 'N': return 13; break;
        case 'O': return 14; break;
        case 'P': return 15; break;
        case 'Q': return 16; break;
        case 'R': return 17; break;
        case 'S': return 18; break;
        case 'T': return 19; break;
        case 'U': return 20; break;
        case 'V': return 21; break;
        case 'W': return 22; break;
        case 'X': return 23; break;
        case 'Y': return 24; break;
        case 'Z': return 25; break;
            /*
        case '\x8F': return 26; break;
        case '\x8E': return 27; break;
        case '\x99': return 28; break;
             */
        case 'a': return 29; break;
        case 'b': return 30; break;
        case 'c': return 31; break;
        case 'd': return 32; break;
        case 'e': return 33; break;
        case 'f': return 34; break;
        case 'g': return 35; break;
        case 'h': return 36; break;
        case 'i': return 37; break;
        case 'j': return 38; break;
        case 'k': return 39; break;
        case 'l': return 40; break;
        case 'm': return 41; break;
        case 'n': return 42; break;
        case 'o': return 43; break;
        case 'p': return 44; break;
        case 'q': return 45; break;
        case 'r': return 46; break;
        case 's': return 47; break;
        case 't': return 48; break;
        case 'u': return 49; break;
        case 'v': return 50; break;
        case 'w': return 51; break;
        case 'x': return 52; break;
        case 'y': return 53; break;
        case 'z': return 54; break;
            /*
        case '\x86': return 55; break;
        case '\x84': return 56; break;
        case '\x94': return 57; break;
             */
        case ' ': return 58; break;
        case '0': return 59; break;
        case '1': return 60; break;
        case '2': return 61; break;
        case '3': return 62; break;
        case '4': return 63; break;
        case '5': return 64; break;
        case '6': return 65; break;
        case '7': return 66; break;
        case '8': return 67; break;
        case '9': return 68; break;
        case '.': return 69; break;
        case ',': return 70; break;
        case ':': return 71; break;
        case '-': return 72; break;
        case '\'': return 73; break;
        case '!': return 74; break;
        case '"': return 75; break;
        case '#': return 76; break;
        case '?': return 77; break;
        case '/': return 78; break;
        case '\\': return 79; break;
        case '[': return 80; break;
        case ']': return 81; break;
        case '(': return 82; break;
        case ')': return 83; break;
        case '%': return 84; break;
        case '_': return 85; break;
        case '+': return 86; break;
        case '|': return 87; break;
        case '=': return 88; break;
        
        case '^': return 89; break;
        case '`': return 90; break;
        case '<': return 91; break;
        case '>': return 92; break;
            
        default: return 77;
            break;
    }
}

char cEngineGetCharPosReverse(int c)
{
    switch (c) {
        case 0: return 'A'; break;
        case 1: return 'B'; break;
        case 2: return 'C'; break;
        case 3: return 'D'; break;
        case 4: return 'E'; break;
        case 5: return 'F'; break;
        case 6: return 'G'; break;
        case 7: return 'H'; break;
        case 8: return 'I'; break;
        case 9: return 'J'; break;
        case 10: return 'K'; break;
        case 11: return 'L'; break;
        case 12: return 'M'; break;
        case 13: return 'N'; break;
        case 14: return 'O'; break;
        case 15: return 'P'; break;
        case 16: return 'Q'; break;
        case 17: return 'R'; break;
        case 18: return 'S'; break;
        case 19: return 'T'; break;
        case 20: return 'U'; break;
        case 21: return 'V'; break;
        case 22: return 'W'; break;
        case 23: return 'X'; break;
        case 24: return 'Y'; break;
        case 25: return 'Z'; break;
            /*
             case '\x8F': return 26; break;
             case '\x8E': return 27; break;
             case '\x99': return 28; break;
             */
        case 29: return 'a'; break;
        case 30: return 'b'; break;
        case 31: return 'c'; break;
        case 32: return 'd'; break;
        case 33: return 'e'; break;
        case 34: return 'f'; break;
        case 35: return 'g'; break;
        case 36: return 'h'; break;
        case 37: return 'i'; break;
        case 38: return 'j'; break;
        case 39: return 'k'; break;
        case 40: return 'l'; break;
        case 41: return 'm'; break;
        case 42: return 'n'; break;
        case 43: return 'o'; break;
        case 44: return 'p'; break;
        case 45: return 'q'; break;
        case 46: return 'r'; break;
        case 47: return 's'; break;
        case 48: return 't'; break;
        case 49: return 'u'; break;
        case 50: return 'v'; break;
        case 51: return 'w'; break;
        case 52: return 'x'; break;
        case 53: return 'y'; break;
        case 54: return 'z'; break;
            /*
             case '\x86': return 55; break;
             case '\x84': return 56; break;
             case '\x94': return 57; break;
             */
        case 58: return ' '; break;
        case 59: return '0'; break;
        case 60: return '1'; break;
        case 61: return '2'; break;
        case 62: return '3'; break;
        case 63: return '4'; break;
        case 64: return '5'; break;
        case 65: return '6'; break;
        case 66: return '7'; break;
        case 67: return '8'; break;
        case 68: return '9'; break;
        case 69: return '.'; break;
        case 70: return ','; break;
        case 71: return ':'; break;
        case 72: return '-'; break;
        case 73: return '\''; break;
        case 74: return '!'; break;
        case 75: return '"'; break;
        case 76: return '#'; break;
        case 77: return '?'; break;
        case 78: return '/'; break;
        case 79: return '\\'; break;
        case 80: return '['; break;
        case 81: return ']'; break;
        case 82: return '('; break;
        case 83: return ')'; break;
        case 84: return '%'; break;
        case 85: return '_'; break;
        case 86: return '+'; break;
        case 87: return '|'; break;
        case 88: return '='; break;
            
        case 89: return '^'; break;
        case 90: return '`'; break;
        case 91: return '<'; break;
        case 92: return '>'; break;
            
        default: return 77;
            break;
    }
}


void cEngineWritePixels(unsigned int **raster, int x, int y, int w, int h, unsigned int c) {
    
   // c = 0x000000FF;
   // unsigned int b_1 = (c >> (8*0)) & 0xff;
   // unsigned int b_2 = (c >> (8*1)) & 0xff;
   // unsigned int b_3 = (c >> (8*2)) & 0xff;
   // unsigned int b_4 = (c >> (8*3)) & 0xff;
    // TODO set depending on alpha byte setting.
    //unsigned int alpha = (c >> (8*3)) & 0xff;
   // unsigned int alpha = (c >> (8*3)) & 0xff;
    
    //int alpha = (c & 0xff);
    
  //  printf("1:%d 2:%d 3:%d 4:%d\n", b_1, b_2, b_3, b_4);
    
    unsigned int alpha = c & 0xFF;
    if(engine->color_mode_argb) {
        alpha = (c >> (8*3)) & 0xff;
    }
    if(alpha > 0) {
        for(int s_x = x; s_x < x+w; s_x++) {
            for(int s_y = y; s_y < y+h; s_y++) {
                if(s_x > -1 && s_y > -1 && s_x < engine->width && s_y < engine->height) {
                    raster[s_x][s_y] = c;
                }
            }
        }
    }
}

void cEngineRenderGUI(unsigned int **raster) {
    
    for(int b = 0; b < engine->max_buttons; b++) {
        struct CButton* button = buttons[b];
        
        if(button->active && !button->hidden && !button->whole_screen) {
          
            if(button->cstr != NULL) {
                int c = 0;
                int c_bg = 0;
                if(button->toggle_button) {
                    if(button->pressed) {
                        c = button->color_pressed;
                        c_bg = button->color_bg_pressed;
                    } else {
                        if(button->toggled) {
                            c = button->color_toggled;
                            c_bg = button->color_bg_toggled;
                        } else {
                            c = button->color;
                            c_bg = button->color_bg;
                        }
                    }
                } else {
                    if(button->pressed) {
                        c = button->color_pressed;
                        c_bg = button->color_bg_pressed;
                        if (button->disabled) {
                            c = button->color_disabled_pressed;
                            c_bg = button->color_bg_pressed_disabled;
                        }
                    } else {
                        c = button->color;
                        c_bg = button->color_bg;
                        if (button->disabled) {
                            c = button->color_disabled;
                        }
                    }
                }
                cEngineWritePixels(raster, button->x, button->y, button->width, button->height, c_bg);
                
                /*
                if(button->pressed) {
                    cEngineRenderLabelByPixelPos(raster, button->label->string, button->x+button->label_offset_x, button->y+button->label_offset_y, button->color_pressed, button->color_bg_pressed);
                
                } else {*/
                if (button->cstr->chars != NULL) {
                    cEngineRenderLabelByPixelPos(raster, button->cstr->chars, button->x+button->label_offset_x, button->y+button->label_offset_y, c, c_bg);
                }
                
               // }
            } else if(button->use_gfx) {
                
                if (button->pressed) {
                    cEngineRenderSprite(raster, button->x, button->y, button->sheet_pressed_x, button->sheet_pressed_y, 0, button->width, button->height, 0, 0xFFFFFFFF, 0);
                } else if(button->disabled) {
                    cEngineRenderSprite(raster, button->x, button->y, button->sheet_disabled_x, button->sheet_disabled_y, 0, button->width, button->height, 0, 0xFFFFFFFF, 0);
                } else {
                    cEngineRenderSprite(raster, button->x, button->y, button->sheet_x, button->sheet_y, 0, button->width, button->height, 0, 0xFFFFFFFF, 0);
                }
           
            } else {
                int c = 0;
                if(button->pressed) {
                    c = button->color_pressed;
                } else {
                    c = button->color;
                }
            
                cEngineWritePixels(raster, button->x, button->y, button->width, button->height, c);
            }
        }
    }
}


