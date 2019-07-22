//
//  CGame.h
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <stdbool.h>
#include "CInput.h"
#include "CLabel.h"
#include "CButton.h"
#include "CEntity.h"
#include "CInput.h"

#ifndef cengine_CEngine_h
#define cengine_CEngine_h

struct CPoint {
    int x;
    int y;
};

struct CVec2 {
    double x;
    double y;
};

struct CSound {
    int type;
};


struct CPoint *cNewPoint(int x, int y);
struct CVec2 *cNewVec2(double x, double y);
struct CSound *cSoundNew(int type);
struct CListQueue *cEngineGetSfxQueue(void);
void cEngineAddSfxToQueue(int type);

struct CEngineContext
{
    int width;
    int height;
    int sprite_size;
    int sheet_width;
    int sheet_height;
    int max_touches;
    int level_width;
    int level_height;
    int max_buttons;
    bool color_mode_rgba;
    bool color_mode_argb;
    unsigned int clear_color;
    bool render_bounding_boxes;
    bool show_fps;
    bool ground_render_enabled;
    struct CInput *input;
    struct CListQueue *sfx_queue;
    void (*cEngineInitHook)(struct CEngineContext *e);
    void (*cEnginePreUpdateHook)(struct CInput *input, struct CButton **buttons, double dt);
    void (*cEngineUpdateHook)(double dt);
    void (*cEnginePreEntityRenderHook)(double dt, unsigned int **raster);
    void (*cEngineRenderHook)(double dt, unsigned int **raster);
    void (*cEngineEntityCollisionHook)(struct CEntity *a, struct CEntity *b);
    void (*cEngineEnvironmentCollisionHook)(void);
    void (*cEngineCleanupHook)(void);
    void (*cEngineSpinnerButtonMovedCallback)(struct CButton *b, int index, int touch_x, int touch_y);
    void (*cEngineSpinnerButtonEndedCallback)(struct CButton *b, int index, int touch_x, int touch_y, int ms_since_init);
    void (*cEngineDragableButtonMovedCallback)(struct CButton *b, int index, int touch_x, int touch_y);
    void (*cEngineDragableButtonEndedCallback)(struct CButton *b, int index, int touch_x, int touch_y, int ms_since_init);
};

void cEngineInit(struct CEngineContext *e);
struct CEngineContext *cEngineContextNew(void);
struct CAnimation*** cEngineGetLevel(void);
struct CListQueue* cEngineGetEntityList(void);
void cEngineLog(char *string);
void cEngineWritePixelData(unsigned int* data);
struct CEntity* cEngineAddEntity(int type, int tile_x, int tile_y);
void cEngineUpdateCamera(int x, int y);
void cEngineCenterCamera(struct CEntity *e);
void cEngineEntityCollisionCallback(struct CEntity *a, struct CEntity *b);
struct CButton* cEngineGetButton(int i);
void cEngineCleanup(void);
int cEngineGetCharPos(char c);
char cEngineGetCharPosReverse(int c);
void cEngineDrawArea(int x, int y, int w, int h, unsigned int color, unsigned int **raster2d);
void cEngineWritePixels(unsigned int **raster, int x, int y, int w, int h, unsigned int c);
void cEngineUpdateInput(double dt, unsigned int **raster);
void cEngineGameloop(double dt, unsigned int **raster);
void cEngineRenderLabelByPixelPos(unsigned int **raster, char *string, int s_x, int s_y, unsigned int color, unsigned int bg_color);
void cEngineRenderLabelWithParams(unsigned int **raster, char *string, int s_x, int s_y, unsigned int color, unsigned int bg_color);
void cEngineRenderLabel(unsigned int **raster, struct CLabel *l, unsigned int color, unsigned int bg_color);
void cEngineRenderCollisionAreas(double dt, unsigned int **raster);
void cEngineRenderEntitySprite(unsigned int **raster, int sprite_x, int sprite_y,
                               int screen_x, int screen_y,
                               int width, int height, bool camera_offset, unsigned char hit, unsigned int color, unsigned int bg_color, bool flipped_h, bool flipped_v);


#endif
