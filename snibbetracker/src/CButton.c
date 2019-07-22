//
//  CButton.c
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "CButton.h"
#include "CLabel.h"
#include "CAllocator.h"


struct CButton *cButtonNew(void) {
    
    struct CButton *b = cAllocatorAlloc(sizeof(struct CButton), "CButton");
    b->x = 0;
    b->y = 0;
    b->width = 0;
    b->height = 0;
    b->use_gfx = false;
    b->sheet_x = -1;
    b->sheet_y = -1;
    b->sheet_pressed_x = -1;
    b->sheet_pressed_y = -1;
    b->sheet_disabled_x = -1;
    b->sheet_disabled_y = -1;
    b->pressed = false;
    b->active = false;
    b->hidden = false;
    b->toggled = false;
    b->dragable = false;
    b->dragable_offset_x = 0;
    b->dragable_offset_y = 0;
    b->disabled = false;
    b->touched_up_inside = false;
    b->whole_screen = false;
    b->spinner_button = false;
    b->spinner_button_active = false;
    b->toggle_button = false;
    b->spinner_button_pressed_x = -1; // coordinate to measure from when dragging.
    b->spinner_button_pressed_y = -1; // coordinate to measure from when dragging.
    b->tag = 0;
    b->color = 0x00000000;
    b->color_bg = 0x00000000;
    b->color_pressed = 0x00000000;
    b->color_bg_pressed = 0x00000000;
    b->color_toggled = 0x00000000;
    b->color_bg_toggled = 0x00000000;
    b->color_disabled = 0x00000000;
    b->color_disabled_pressed = 0x00000000;
    b->color_bg_pressed_disabled = 0x00000000;
    b->cstr = NULL;
    b->label_offset_x = 0;
    b->label_offset_y = 0;
    b->label_centered = false;
    b->min_width = 0;
    b->min_height = 0;
    return b;
}

void cButtonSetLabel(struct CButton *b, struct CStr *cstr) {
    

    if (b->cstr != cstr) {
        b->cstr = cstr;
    }
    
    // calculate width of button
    int len = (int)cStrLength(cstr);
    int width = 0;
    for(int i = 0; i <len; i++) {
        width += 8;
    }
    
    /* todo 
     
     - width and height should reflect the background which will be used as touch area.
     - check how touch and render works currently for label buttons.
     - default size of label buttons should be minimal touch area size.
     
     - make a kind of button which you can press and drag outside the bounds to cycle values.
        when released out of bounds the button will
     
     */
    b->width = width;
    b->height = 32;
    if(b->height < b->min_height) {
        b->height = b->min_height;
    }
    b->label_offset_y = (b->height / 2) - 5;
    if(b->min_width > b->width) {
        // center label.
        if(b->label_centered) {
            b->label_offset_x = (b->min_width - b->width) / 2;
        }
        b->width = b->min_width;
    }
    
    b->label_offset_y = (b->height / 2) - 5;
    
}

int cButtonIsInside(struct CButton *b, int x, int y) {
    
    if(x > b->x && y > b->y && x < b->x+b->width && y < b->y+b->height) {
        return 1;
    } else {
        return 0;
    }
    /*
    if(x < b->x) { return 0; }
    if(x > b->x+b->width) { return 0; }
    if(y < b->y) { return 0; }
    if(y > b->y+b->height) { return 0; }
     */
   // return 1;
}

void cButtonCleanup(struct CButton *b) {
    
    if (b->cstr != NULL) {
        b->cstr = cStrCleanup(b->cstr);
    }
}
