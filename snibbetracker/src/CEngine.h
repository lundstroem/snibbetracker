/*
 
 MIT License
 
 Copyright (c) 2019 Harry Lundström
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
 */

#include <stdbool.h>
#include <stddef.h>
#include "CInput.h"

#ifndef cengine_CEngine_h
#define cengine_CEngine_h

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

struct CLabel
{
    struct CStr *str;
    int x_offset_max; // for scrolltext
    int x;
    int y;
    bool scrolling;
};

struct CLabel *cLabelNew(struct CStr *cstr);
void cLabelSetLabel(struct CLabel *l, struct CStr *cstr);
void cLabelActivateScrolling(struct CLabel *l, int start_x);
void cLabelCleanup(struct CLabel *l);

struct CStr {
    char *chars;
    int size;
};

void cStrTests(void);
int cPrint(const char *fmt,...);
void cStrResize(struct CStr *cstr, size_t size);
struct CStr *cStrPrint(struct CStr *cstr, const char *fmt,...);
struct CStr *cStrPrintWithSize(size_t size, struct CStr *cstr, const char *fmt,...);
unsigned long cStrLength(struct CStr *cstr);
char cStrCharAt(struct CStr *cstr, int pos);
struct CStr *cStrCleanup(struct CStr *cstr);

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
    bool color_mode_rgba;
    bool color_mode_argb;
    unsigned int clear_color;
    bool show_fps;
    bool ground_render_enabled;
    struct CInput *input;
    void (*cEngineInitHook)(struct CEngineContext *e);
    void (*cEnginePreUpdateHook)(struct CInput *input, double dt);
    void (*cEngineUpdateHook)(double dt);
    void (*cEngineRenderHook)(double dt, unsigned int **raster);
    void (*cEngineEnvironmentCollisionHook)(void);
    void (*cEngineCleanupHook)(void);
};

void cEngineInit(struct CEngineContext *e);
struct CEngineContext *cEngineContextNew(void);
void cEngineLog(char *string);
void cEngineWritePixelData(unsigned int* data);
void cEngineUpdateCamera(int x, int y);
void cEngineCleanup(void);
int cEngineGetCharPos(char c);
char cEngineGetCharPosReverse(int c);
void cEngineDrawArea(int x, int y, int w, int h, unsigned int color, unsigned int **raster2d);
void cEngineUpdateInput(double dt, unsigned int **raster);
void cEngineGameloop(double dt, unsigned int **raster);
void cEngineRenderLabelByPixelPos(unsigned int **raster, char *string, int s_x, int s_y, unsigned int color, unsigned int bg_color);
void cEngineRenderLabelWithParams(unsigned int **raster, char *string, int s_x, int s_y, unsigned int color, unsigned int bg_color);
void cEngineRenderLabel(unsigned int **raster, struct CLabel *l, unsigned int color, unsigned int bg_color);

#endif
