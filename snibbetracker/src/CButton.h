//
//  CButton.h
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <stdbool.h>
#include "CStr.h"

#ifndef cengine_CButton_h
#define cengine_CButton_h

struct CButton {
    
    int x;
    int y;
    int width;
    int height;
    int use_gfx;
    int sheet_x;
    int sheet_y;
    int sheet_pressed_x;
    int sheet_pressed_y;
    int sheet_disabled_x;
    int sheet_disabled_y;
    bool pressed;
    bool active;
    bool hidden;
    bool toggled;
    bool dragable;
    int dragable_offset_x;
    int dragable_offset_y;
    bool disabled;
    bool touched_up_inside;
    bool whole_screen;
    bool spinner_button;
    bool spinner_button_active;
    bool toggle_button;
    int spinner_button_pressed_x; // coordinate to measure from when dragging.
    int spinner_button_pressed_y; // coordinate to measure from when dragging.
    int tag;
    unsigned int color;
    unsigned int color_bg;
    unsigned int color_pressed;
    unsigned int color_bg_pressed;
    unsigned int color_toggled;
    unsigned int color_bg_toggled;
    unsigned int color_bg_pressed_disabled;
    unsigned int color_disabled;
    unsigned int color_disabled_pressed;
    struct CStr *cstr;
    int label_offset_x;
    int label_offset_y;
    bool label_centered;
    int min_width;
    int min_height;
};

struct CButton *cButtonNew(void);
int cButtonIsInside(struct CButton *b, int x, int y);
void cButtonSetLabel(struct CButton *b, struct CStr *cstr);
void cButtonCleanup(struct CButton *b);

#endif
