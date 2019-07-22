//
//  CInputKeys.c
//  HelloSDL
//
//  Created by d d on 2/16/13.
//  Copyright (c) 2013 Student. All rights reserved.
//

#include <stdio.h>
#include "CInput.h"
#include "CAllocator.h"

struct CInput *cInputNew(void) {
    
    struct CInput *input = cAllocatorAlloc(sizeof(struct CInput), "CInput");
    input->mouse1 = false;
    input->mouse2 = false;
    
    input->key_0 = false;
    input->key_1 = false;
    input->key_2 = false;
    input->key_3 = false;
    input->key_4 = false;
    input->key_5 = false;
    input->key_6 = false;
    input->key_7 = false;
    input->key_8 = false;
    input->key_9 = false;
    input->key_a = false;
    input->key_b = false;
    input->key_c = false;
    input->key_d = false;
    input->key_e = false;
    input->key_f = false;
    input->key_g = false;
    input->key_h = false;
    input->key_i = false;
    input->key_j = false;
    input->key_k = false;
    input->key_l = false;
    input->key_m = false;
    input->key_n = false;
    input->key_o = false;
    input->key_p = false;
    input->key_q = false;
    input->key_r = false;
    input->key_s = false;
    input->key_t = false;
    input->key_u = false;
    input->key_v = false;
    input->key_w = false;
    input->key_x = false;
    input->key_y = false;
    input->key_z = false;
    input->key_space = false;
    input->key_plus = false;
    input->key_minus = false;
    input->key_tab = false;
    input->key_lgui = false;
    input->key_lctrl = false;
    input->key_escape = false;
    input->key_return = false;
    input->key_left = false;
    input->key_right = false;
    input->key_up = false;
    input->key_down = false;
    input->key_lshift = false;
    input->key_home = false;
    input->key_end = false;
    input->key_backspace = false;
    input->key_delete = false;
    input->key_comma = false;
    input->key_period = false;
    input->key_f1 = false;
    input->key_f2 = false;
    input->key_f3 = false;
    input->key_f4 = false;
    input->key_f5 = false;
    input->key_f6 = false;
    input->key_f7 = false;
    input->key_f8 = false;
    input->key_f9 = false;
    input->key_f10 = false;
    input->key_f11 = false;
    input->key_f12 = false;
    
    // key locks. if locked, key_up must be called before key can be active again.
    input->mouse1_lock = false;
    input->mouse2_lock = false;
    input->key_lock_0 = false;
    input->key_lock_1 = false;
    input->key_lock_2 = false;
    input->key_lock_3 = false;
    input->key_lock_4 = false;
    input->key_lock_5 = false;
    input->key_lock_6 = false;
    input->key_lock_7 = false;
    input->key_lock_8 = false;
    input->key_lock_9 = false;
    input->key_lock_a = false;
    input->key_lock_b = false;
    input->key_lock_c = false;
    input->key_lock_d = false;
    input->key_lock_e = false;
    input->key_lock_f = false;
    input->key_lock_g = false;
    input->key_lock_h = false;
    input->key_lock_i = false;
    input->key_lock_j = false;
    input->key_lock_k = false;
    input->key_lock_l = false;
    input->key_lock_m = false;
    input->key_lock_n = false;
    input->key_lock_o = false;
    input->key_lock_p = false;
    input->key_lock_q = false;
    input->key_lock_r = false;
    input->key_lock_s = false;
    input->key_lock_t = false;
    input->key_lock_u = false;
    input->key_lock_v = false;
    input->key_lock_w = false;
    input->key_lock_x = false;
    input->key_lock_y = false;
    input->key_lock_z = false;
    input->key_lock_space = false;
    input->key_lock_plus = false;
    input->key_lock_minus = false;
    input->key_lock_tab = false;
    input->key_lock_lgui = false;
    input->key_lock_lctrl = false;
    input->key_lock_escape = false;
    input->key_lock_return = false;
    input->key_lock_left = false;
    input->key_lock_right = false;
    input->key_lock_up = false;
    input->key_lock_down = false;
    input->key_lock_lshift = false;
    input->key_lock_home = false;
    input->key_lock_end = false;
    input->key_lock_backspace = false;
    input->key_lock_delete = false;
    input->key_lock_comma = false;
    input->key_lock_period = false;
    input->key_lock_f1 = false;
    input->key_lock_f2 = false;
    input->key_lock_f3 = false;
    input->key_lock_f4 = false;
    input->key_lock_f5 = false;
    input->key_lock_f6 = false;
    input->key_lock_f7 = false;
    input->key_lock_f8 = false;
    input->key_lock_f9 = false;
    input->key_lock_f10 = false;
    input->key_lock_f11 = false;
    input->key_lock_f12 = false;
    
    input->mouse1_pending_lock = false;
    input->mouse2_pending_lock = false;
    input->key_pending_lock_0 = false;
    input->key_pending_lock_1 = false;
    input->key_pending_lock_2 = false;
    input->key_pending_lock_3 = false;
    input->key_pending_lock_4 = false;
    input->key_pending_lock_5 = false;
    input->key_pending_lock_6 = false;
    input->key_pending_lock_7 = false;
    input->key_pending_lock_8 = false;
    input->key_pending_lock_9 = false;
    input->key_pending_lock_a = false;
    input->key_pending_lock_b = false;
    input->key_pending_lock_c = false;
    input->key_pending_lock_d = false;
    input->key_pending_lock_e = false;
    input->key_pending_lock_f = false;
    input->key_pending_lock_g = false;
    input->key_pending_lock_h = false;
    input->key_pending_lock_i = false;
    input->key_pending_lock_j = false;
    input->key_pending_lock_k = false;
    input->key_pending_lock_l = false;
    input->key_pending_lock_m = false;
    input->key_pending_lock_n = false;
    input->key_pending_lock_o = false;
    input->key_pending_lock_p = false;
    input->key_pending_lock_q = false;
    input->key_pending_lock_r = false;
    input->key_pending_lock_s = false;
    input->key_pending_lock_t = false;
    input->key_pending_lock_u = false;
    input->key_pending_lock_v = false;
    input->key_pending_lock_w = false;
    input->key_pending_lock_x = false;
    input->key_pending_lock_y = false;
    input->key_pending_lock_z = false;
    input->key_pending_lock_space = false;
    input->key_pending_lock_plus = false;
    input->key_pending_lock_minus = false;
    input->key_pending_lock_tab = false;
    input->key_pending_lock_lgui = false;
    input->key_pending_lock_lctrl = false;
    input->key_pending_lock_escape = false;
    input->key_pending_lock_return = false;
    input->key_pending_lock_left = false;
    input->key_pending_lock_right = false;
    input->key_pending_lock_up = false;
    input->key_pending_lock_down = false;
    input->key_pending_lock_lshift = false;
    input->key_pending_lock_home = false;
    input->key_pending_lock_end = false;
    input->key_pending_lock_backspace = false;
    input->key_pending_lock_delete = false;
    input->key_pending_lock_comma = false;
    input->key_pending_lock_period = false;
    input->key_pending_lock_f1 = false;
    input->key_pending_lock_f2 = false;
    input->key_pending_lock_f3 = false;
    input->key_pending_lock_f4 = false;
    input->key_pending_lock_f5 = false;
    input->key_pending_lock_f6 = false;
    input->key_pending_lock_f7 = false;
    input->key_pending_lock_f8 = false;
    input->key_pending_lock_f9 = false;
    input->key_pending_lock_f10 = false;
    input->key_pending_lock_f11 = false;
    input->key_pending_lock_f12 = false;
    
    input->mouse_x = 0;
    input->mouse_y = 0;
    input->touches = NULL;
    input->ended_touches = NULL;
    
    return input;
}

void cInputApplyPendingLocks(struct CInput *input) {
    
    input->mouse1_lock = input->mouse1_pending_lock;
    input->mouse2_lock = input->mouse2_pending_lock;
    input->key_lock_0 = input->key_pending_lock_0;
    input->key_lock_1 = input->key_pending_lock_1;
    input->key_lock_2 = input->key_pending_lock_2;
    input->key_lock_3 = input->key_pending_lock_3;
    input->key_lock_4 = input->key_pending_lock_4;
    input->key_lock_5 = input->key_pending_lock_5;
    input->key_lock_6 = input->key_pending_lock_6;
    input->key_lock_7 = input->key_pending_lock_7;
    input->key_lock_8 = input->key_pending_lock_8;
    input->key_lock_9 = input->key_pending_lock_9;
    input->key_lock_a = input->key_pending_lock_a;
    input->key_lock_b = input->key_pending_lock_b;
    input->key_lock_c = input->key_pending_lock_c;
    input->key_lock_d = input->key_pending_lock_d;
    input->key_lock_e = input->key_pending_lock_e;
    input->key_lock_f = input->key_pending_lock_f;
    input->key_lock_g = input->key_pending_lock_g;
    input->key_lock_h = input->key_pending_lock_h;
    input->key_lock_i = input->key_pending_lock_i;
    input->key_lock_j = input->key_pending_lock_j;
    input->key_lock_k = input->key_pending_lock_k;
    input->key_lock_l = input->key_pending_lock_l;
    input->key_lock_m = input->key_pending_lock_m;
    input->key_lock_n = input->key_pending_lock_n;
    input->key_lock_o = input->key_pending_lock_o;
    input->key_lock_p = input->key_pending_lock_p;
    input->key_lock_q = input->key_pending_lock_q;
    input->key_lock_r = input->key_pending_lock_r;
    input->key_lock_s = input->key_pending_lock_s;
    input->key_lock_t = input->key_pending_lock_t;
    input->key_lock_u = input->key_pending_lock_u;
    input->key_lock_v = input->key_pending_lock_v;
    input->key_lock_w = input->key_pending_lock_w;
    input->key_lock_x = input->key_pending_lock_x;
    input->key_lock_y = input->key_pending_lock_y;
    input->key_lock_z = input->key_pending_lock_z;
    input->key_lock_space = input->key_pending_lock_space;
    input->key_lock_plus = input->key_pending_lock_plus;
    input->key_lock_minus = input->key_pending_lock_minus;
    input->key_lock_tab = input->key_pending_lock_tab;
    input->key_lock_lgui = input->key_pending_lock_lgui;
    input->key_lock_lctrl = input->key_pending_lock_lctrl;
    input->key_lock_escape = input->key_pending_lock_escape;
    input->key_lock_return = input->key_pending_lock_return;
    input->key_lock_left = input->key_pending_lock_left;
    input->key_lock_right = input->key_pending_lock_right;
    input->key_lock_up = input->key_pending_lock_up;
    input->key_lock_down = input->key_pending_lock_down;
    input->key_lock_lshift = input->key_pending_lock_lshift;
    input->key_lock_home = input->key_pending_lock_home;
    input->key_lock_end = input->key_pending_lock_end;
    input->key_lock_backspace = input->key_pending_lock_backspace;
    input->key_lock_delete = input->key_pending_lock_delete;
    input->key_lock_comma = input->key_pending_lock_comma;
    input->key_lock_period = input->key_pending_lock_period;
    input->key_lock_f1 = input->key_pending_lock_f1;
    input->key_lock_f2 = input->key_pending_lock_f2;
    input->key_lock_f3 = input->key_pending_lock_f3;
    input->key_lock_f4 = input->key_pending_lock_f4;
    input->key_lock_f5 = input->key_pending_lock_f5;
    input->key_lock_f6 = input->key_pending_lock_f6;
    input->key_lock_f7 = input->key_pending_lock_f7;
    input->key_lock_f8 = input->key_pending_lock_f8;
    input->key_lock_f9 = input->key_pending_lock_f9;
    input->key_lock_f10 = input->key_pending_lock_f10;
    input->key_lock_f11 = input->key_pending_lock_f11;
    input->key_lock_f12 = input->key_pending_lock_f12;

}

void cInputResetKeys(struct CInput *input) {
    
    input->key_0 = false;
    input->key_1 = false;
    input->key_2 = false;
    input->key_3 = false;
    input->key_4 = false;
    input->key_5 = false;
    input->key_6 = false;
    input->key_7 = false;
    input->key_8 = false;
    input->key_9 = false;
    input->key_a = false;
    input->key_b = false;
    input->key_c = false;
    input->key_d = false;
    input->key_e = false;
    input->key_f = false;
    input->key_g = false;
    input->key_h = false;
    input->key_i = false;
    input->key_j = false;
    input->key_k = false;
    input->key_l = false;
    input->key_m = false;
    input->key_n = false;
    input->key_o = false;
    input->key_p = false;
    input->key_q = false;
    input->key_r = false;
    input->key_s = false;
    input->key_t = false;
    input->key_u = false;
    input->key_v = false;
    input->key_w = false;
    input->key_x = false;
    input->key_y = false;
    input->key_z = false;
    input->key_space = false;
    input->key_plus = false;
    input->key_minus = false;
    input->key_tab = false;
    input->key_lgui = false;
    input->key_lctrl = false;
    input->key_escape = false;
    input->key_return = false;
    input->key_left = false;
    input->key_right = false;
    input->key_up = false;
    input->key_down = false;
    input->key_lshift = false;
    input->key_home = false;
    input->key_end = false;
    input->key_backspace = false;
    input->key_delete = false;
    input->key_comma = false;
    input->key_period = false;
    input->key_f1 = false;
    input->key_f2 = false;
    input->key_f3 = false;
    input->key_f4 = false;
    input->key_f5 = false;
    input->key_f6 = false;
    input->key_f7 = false;
    input->key_f8 = false;
    input->key_f9 = false;
    input->key_f10 = false;
    input->key_f11 = false;
    input->key_f12 = false;
}

void *cInputCleanup(struct CInput *input, int max_touches) {
    
    int i;
    if(input->touches != NULL) {
        for(i = 0; i < max_touches; i++) {
            if(input->touches != NULL) {
                input->touches[i] = cAllocatorFree(input->touches[i]);
            }
        }
        input->touches = cAllocatorFree(input->touches);
    }
    
    if(input->ended_touches != NULL) {
        for(i = 0; i < max_touches; i++) {
            if(input->ended_touches != NULL) {
                input->ended_touches[i] = cAllocatorFree(input->ended_touches[i]);
            }
        }
        input->ended_touches = cAllocatorFree(input->ended_touches);
    }
    
    input = cAllocatorFree(input);
    
    return input;
}



