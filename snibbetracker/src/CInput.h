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

#ifndef HelloSDL_CInputKeys_h
#define HelloSDL_CInputKeys_h

struct CInput {
	bool mouse1;
	bool mouse2;
    bool key_0;
    bool key_1;
    bool key_2;
    bool key_3;
    bool key_4;
    bool key_5;
    bool key_6;
    bool key_7;
    bool key_8;
    bool key_9;
    bool key_a;
    bool key_b;
    bool key_c;
    bool key_d;
    bool key_e;
    bool key_f;
    bool key_g;
    bool key_h;
    bool key_i;
    bool key_j;
    bool key_k;
    bool key_l;
    bool key_m;
    bool key_n;
    bool key_o;
    bool key_p;
    bool key_q;
    bool key_r;
    bool key_s;
    bool key_t;
    bool key_u;
    bool key_v;
    bool key_w;
    bool key_x;
    bool key_y;
    bool key_z;
    bool key_space;
    bool key_plus;
    bool key_minus;
    bool key_tab;
    bool key_lgui;
    bool key_lctrl;
    bool key_escape;
    bool key_return;
    bool key_left;
    bool key_right;
    bool key_up;
    bool key_down;
    bool key_lshift;
    bool key_home;
    bool key_end;
    bool key_backspace;
    bool key_delete;
    bool key_comma;
    bool key_period;
    bool key_f1;
    bool key_f2;
    bool key_f3;
    bool key_f4;
    bool key_f5;
    bool key_f6;
    bool key_f7;
    bool key_f8;
    bool key_f9;
    bool key_f10;
    bool key_f11;
    bool key_f12;
    
    bool mouse1_lock;
    bool mouse2_lock;
    bool key_lock_0;
    bool key_lock_1;
    bool key_lock_2;
    bool key_lock_3;
    bool key_lock_4;
    bool key_lock_5;
    bool key_lock_6;
    bool key_lock_7;
    bool key_lock_8;
    bool key_lock_9;
    bool key_lock_a;
    bool key_lock_b;
    bool key_lock_c;
    bool key_lock_d;
    bool key_lock_e;
    bool key_lock_f;
    bool key_lock_g;
    bool key_lock_h;
    bool key_lock_i;
    bool key_lock_j;
    bool key_lock_k;
    bool key_lock_l;
    bool key_lock_m;
    bool key_lock_n;
    bool key_lock_o;
    bool key_lock_p;
    bool key_lock_q;
    bool key_lock_r;
    bool key_lock_s;
    bool key_lock_t;
    bool key_lock_u;
    bool key_lock_v;
    bool key_lock_w;
    bool key_lock_x;
    bool key_lock_y;
    bool key_lock_z;
    bool key_lock_space;
    bool key_lock_plus;
    bool key_lock_minus;
    bool key_lock_tab;
    bool key_lock_lgui;
    bool key_lock_lctrl;
    bool key_lock_escape;
    bool key_lock_return;
    bool key_lock_left;
    bool key_lock_right;
    bool key_lock_up;
    bool key_lock_down;
    bool key_lock_lshift;
    bool key_lock_home;
    bool key_lock_end;
    bool key_lock_backspace;
    bool key_lock_delete;
    bool key_lock_comma;
    bool key_lock_period;
    bool key_lock_f1;
    bool key_lock_f2;
    bool key_lock_f3;
    bool key_lock_f4;
    bool key_lock_f5;
    bool key_lock_f6;
    bool key_lock_f7;
    bool key_lock_f8;
    bool key_lock_f9;
    bool key_lock_f10;
    bool key_lock_f11;
    bool key_lock_f12;
    
    bool mouse1_pending_lock;
    bool mouse2_pending_lock;
    bool key_pending_lock_0;
    bool key_pending_lock_1;
    bool key_pending_lock_2;
    bool key_pending_lock_3;
    bool key_pending_lock_4;
    bool key_pending_lock_5;
    bool key_pending_lock_6;
    bool key_pending_lock_7;
    bool key_pending_lock_8;
    bool key_pending_lock_9;
    bool key_pending_lock_a;
    bool key_pending_lock_b;
    bool key_pending_lock_c;
    bool key_pending_lock_d;
    bool key_pending_lock_e;
    bool key_pending_lock_f;
    bool key_pending_lock_g;
    bool key_pending_lock_h;
    bool key_pending_lock_i;
    bool key_pending_lock_j;
    bool key_pending_lock_k;
    bool key_pending_lock_l;
    bool key_pending_lock_m;
    bool key_pending_lock_n;
    bool key_pending_lock_o;
    bool key_pending_lock_p;
    bool key_pending_lock_q;
    bool key_pending_lock_r;
    bool key_pending_lock_s;
    bool key_pending_lock_t;
    bool key_pending_lock_u;
    bool key_pending_lock_v;
    bool key_pending_lock_w;
    bool key_pending_lock_x;
    bool key_pending_lock_y;
    bool key_pending_lock_z;
    bool key_pending_lock_space;
    bool key_pending_lock_plus;
    bool key_pending_lock_minus;
    bool key_pending_lock_tab;
    bool key_pending_lock_lgui;
    bool key_pending_lock_lctrl;
    bool key_pending_lock_escape;
    bool key_pending_lock_return;
    bool key_pending_lock_left;
    bool key_pending_lock_right;
    bool key_pending_lock_up;
    bool key_pending_lock_down;
    bool key_pending_lock_lshift;
    bool key_pending_lock_home;
    bool key_pending_lock_end;
    bool key_pending_lock_backspace;
    bool key_pending_lock_delete;
    bool key_pending_lock_comma;
    bool key_pending_lock_period;
    bool key_pending_lock_f1;
    bool key_pending_lock_f2;
    bool key_pending_lock_f3;
    bool key_pending_lock_f4;
    bool key_pending_lock_f5;
    bool key_pending_lock_f6;
    bool key_pending_lock_f7;
    bool key_pending_lock_f8;
    bool key_pending_lock_f9;
    bool key_pending_lock_f10;
    bool key_pending_lock_f11;
    bool key_pending_lock_f12;

    // add more keys as needed.
    
    int mouse_x;
    int mouse_y;
};

struct CInput *cInputNew(void);
void cInputApplyPendingLocks(struct CInput *input);
void cInputResetKeys(struct CInput *input);
void *cInputCleanup(struct CInput* input, int max_touches);

#endif
