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
#include <stdarg.h>
#include "CEngine.h"
#include "CAllocator.h"
#include "CSynth.h"

// CTimer

struct CTimer *cTimerNew(double wait_time) {
    struct CTimer *t = cAllocatorAlloc(sizeof(struct CTimer), "CTimer");
    t->amountToWait = wait_time;
    t->initAmountToWait = wait_time;
    t->currentWaitingTime = 0;
    t->isReady = false;
    return t;
}

double cTimerTimeLeft(struct CTimer *t) {
    return t->amountToWait-t->currentWaitingTime;
}


void cTimerSetReady(struct CTimer *t) {
    t->currentWaitingTime = t->amountToWait+1;
}

unsigned char cTimerIsReady(struct CTimer *t) {
    if(t->currentWaitingTime > t->amountToWait) {
        t->isReady = true;
    }
    return t->isReady;
}

void cTimerAdvance(double time, struct CTimer *t) {
    //if the timer has not finished, advance.
    if(t->isReady == 0) {
        t->currentWaitingTime += time;
    }
}

void cTimerResetWithTime(double time, struct CTimer *t) {
    t->amountToWait = time;
    t->initAmountToWait = time;
    t->isReady = false;
    t->currentWaitingTime = 0;
}

void cTimerReset(struct CTimer *t) {
    t->amountToWait = t->initAmountToWait;
    t->isReady = false;
    t->currentWaitingTime = 0;
}

// CLabel

struct CLabel *cLabelNew(struct CStr *cstr) {
    
    struct CLabel *t = cAllocatorAlloc(sizeof(struct CLabel), "CLabel");
    if (cstr != NULL) {
        if (cstr->chars != NULL) {
            int length = (int)cStrLength(cstr);
            t->x_offset_max = length*8;
            t->scrolling = false;
            return t;
        }
    }
    return NULL;
    
}

void cLabelSetLabel(struct CLabel *l, struct CStr *cstr) {
    
    if (cstr != NULL) {
        if (cstr->chars != NULL) {
            int length = (int)cStrLength(cstr);
            l->x_offset_max = length*8;
            l->scrolling = false;
        }
    }
}

void cLabelActivateScrolling(struct CLabel *l, int start_x) {
    
    l->scrolling = true;
    l->x = start_x;
}

void cLabelCleanup(struct CLabel *l) {
    
    if (l->str != NULL) {
        l->str = cStrCleanup(l->str);
    }
}

// CStr

int cPrint(const char *fmt,...) {
    
    size_t count = 255;
    char chars[256];
    size_t ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vsnprintf(chars, count, fmt, ap);
    va_end(ap);
    printf("str:%s\n",chars);
    return (int)ret;
}

void cStrResize(struct CStr *cstr, size_t size) {
    
    if (cstr != NULL) {
        char *temp_chars = cAllocatorAlloc((sizeof(char)*size)+1, "cstr resize file name chars");
        if (cstr->chars != NULL) {
            cstr->chars = cAllocatorFree(cstr->chars);
        }
        cstr->size = (int)size;
        cstr->chars = temp_chars;
    }
}

struct CStr *cStrPrintWithSize(size_t size, struct CStr *cstr, const char *fmt,...) {
    
    if (cstr == NULL) {
        const size_t default_size = size;
        cstr = cAllocatorAlloc(sizeof(struct CStr), "CStr");
        char *temp_chars = cAllocatorAlloc((sizeof(char)*default_size)+1, "cstr file name chars");
        cstr->chars = temp_chars;
        cstr->size = (int)default_size;
    }
    
    if (cstr != NULL) {
        if (cstr->chars != NULL) {
            size_t count = cstr->size;
            size_t ret;
            va_list ap;
            va_start(ap, fmt);
            ret = vsnprintf(cstr->chars, count, fmt, ap);
            va_end(ap);
        }
    }
    return cstr;
}

struct CStr *cStrPrint(struct CStr *cstr, const char *fmt,...) {
    
    if (cstr == NULL) {
        const size_t default_size = 20;
        cstr = cAllocatorAlloc(sizeof(struct CStr), "CStr");
        char *temp_chars = cAllocatorAlloc((sizeof(char)*default_size)+1, "cstr file name chars");
        cstr->chars = temp_chars;
        cstr->size = default_size;
    }
    
    if (cstr != NULL) {
        if (cstr->chars != NULL) {
            size_t count = cstr->size;
            size_t ret;
            va_list ap;
            va_start(ap, fmt);
            ret = vsnprintf(cstr->chars, count, fmt, ap);
            va_end(ap);
        }
    }
    return cstr;
}

struct CStr *cStrCleanup(struct CStr *cstr) {
    
    if (cstr != NULL) {
        if(cstr->chars != NULL) {
            cstr->chars = cAllocatorFree(cstr->chars);
        }
        cAllocatorFree(cstr);
    }
    return NULL;
}

unsigned long cStrLength(struct CStr *cstr) {
    
    unsigned long length = 0;
    if (cstr != NULL) {
        if(cstr->chars != NULL) {
            length = strlen(cstr->chars);
        }
    }
    return length;
}

char cStrCharAt(struct CStr *cstr, int pos) {
    
    char ret = 0;
    if (cstr != NULL) {
        if(pos < cstr->size) {
            ret = cstr->chars[pos];
        }
    }
    return ret;
}

// CEngine

static void cEngineShowFPS(double dt, unsigned int **raster);
static void cEngineInitData(void);
static void cEngineRender(double dt, unsigned int **raster, struct CInput *input);
static void cEngineUpdate(double dt, struct CInput *input);
static void cEngineClearScreen(unsigned int **raster);
static void cEngineRenderSprite(unsigned int **raster, int sprite_x, int sprite_y,
                         int screen_x, int screen_y
                         ,unsigned char camera_offset,
                         int width, int height, unsigned char hit, unsigned int color, unsigned int bg_color);

static unsigned int **sprite_pixel_data = NULL;
static int camera_x = 0;
static int camera_y = 0;
static struct CEngineContext *engine = NULL;

void cEngineInit(struct CEngineContext *e) {
    

    engine = e;
    
    cEngineInitData();
    srand(666);

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
    // TODO use these in rendering.
    c->color_mode_rgba = true;
    c->color_mode_argb = false;
    c->clear_color = 0x000000FF;
    c->show_fps = false;
    c->input = NULL;
    c->ground_render_enabled = true;
    c->cEngineInitHook = NULL;
    c->cEnginePreUpdateHook = NULL;
    c->cEngineUpdateHook = NULL;
    c->cEngineRenderHook = NULL;
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
   
    for(int i = 0; i < engine->sheet_width; i++) {
        if(sprite_pixel_data != NULL) {
            sprite_pixel_data[i] = cAllocatorFree(sprite_pixel_data[i]);
        }
    }
    sprite_pixel_data = cAllocatorFree(sprite_pixel_data);
    
    cInputCleanup(engine->input, engine->max_touches);
    engine->input = NULL;
    
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
    
    int i;
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
    
    engine->input = cInputNew();
}

// only used for init
void cEngineWritePixelData(unsigned int* ptr) {
    
    for (int s_x = 0; s_x < engine->sheet_width; s_x++) {
        for (int s_y = 0; s_y < engine->sheet_height; s_y++) {
            sprite_pixel_data[s_x][s_y] = ptr[s_x+s_y*engine->sheet_width];
        }
    }
}

void cEngineUpdateInput(double dt, unsigned int **raster) {
    cEngineUpdate(dt, engine->input);
}
void cEngineGameloop(double dt, unsigned int **raster) {
    cEngineRender(dt, raster, engine->input);
}

void cEngineUpdate(double dt, struct CInput *input) {
    
    if(engine->cEnginePreUpdateHook != NULL) {
        engine->cEnginePreUpdateHook(engine->input, dt);
    }
    
    if(engine->cEngineUpdateHook != NULL) {
        engine->cEngineUpdateHook(dt);
    }
}

void cEngineUpdateCamera(int x, int y) {
    camera_x = x;
    camera_y = y;
}

void cEngineRender(double dt, unsigned int **raster, struct CInput *input) {
    
    cEngineClearScreen(raster);
    
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
    
    screen_x *= width;
    screen_y *= height;
    
    for (int s_x = 0; s_x < width; s_x++) {
        for (int s_y = 0; s_y < height; s_y++) {
            // screen offsets
            int s_x_p = sprite_x+s_x;
            int s_y_p = sprite_y+s_y;
            
            // sheet offsets
            int s_x_sheet_p = screen_x+s_x;
            int s_y_sheet_p = screen_y+s_y;
            
            // camera offsets
            if(camera_offset) {
                s_x_p -= camera_x;
                s_y_p -= camera_y;
            }
            
            if(s_x_p > -1 && s_y_p > -1 && s_x_p < engine->width && s_y_p < engine->height
               && s_x_sheet_p > -1 && s_y_sheet_p > -1 && s_x_sheet_p < engine->sheet_width && s_y_sheet_p < engine->sheet_height) {
                unsigned int alpha2 = sprite_pixel_data[s_x_sheet_p][s_y_sheet_p] & 0xFF;

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
        }
    }
}

void cEngineClearScreen(unsigned int **raster) {
    
    for (int s_x = 0; s_x < engine->width; s_x++) {
        for (int s_y = 0; s_y < engine->height; s_y++) {
            raster[s_x][s_y] = engine->clear_color;
        }
    }
}

void cEngineRenderLabelWithParams(unsigned int **raster, char *string, int s_x, int s_y, unsigned int color, unsigned int bg_color) {
    
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
    
    unsigned int alpha = color & 0xff;
    if(engine->color_mode_argb) {
        alpha = (color >> (8*3)) & 0xff;
    }
    
    if(alpha > 0) {
        int len = (int)strlen(string);
        for(int i = 0; i <len; i++) {
            char c = string[i];
            int x_char_pos = cEngineGetCharPos(c);
            cEngineRenderSprite(raster, s_x+(i*8), s_y, x_char_pos, 0, 1, 8, 12, 0, color, bg_color);
        }
    }
}

int cEngineGetCharPos(char c) {
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

