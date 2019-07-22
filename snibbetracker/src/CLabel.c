//
//  CLabel.c
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "CLabel.h"
#include "CAllocator.h"

struct CLabel *cLabelNew(struct CStr *cstr) {
    
    struct CLabel *t = cAllocatorAlloc(sizeof(struct CLabel), "CLabel");
    //char *temp_chars = cAllocatorAlloc(sizeof(char)*64, "file name chars");
    //t->string = temp_chars;
    //snprintf(t->string, 63, "%s", string);
    //int len = (int)strlen(temp_chars);
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
    
    //if (l->string == NULL) {
    //    char *temp_chars = cAllocatorAlloc(sizeof(char)*64, "file name chars");
    //    l->string = temp_chars;
    //}
    //snprintf(l->string, 63, "%s", string);
    //int len = (int)strlen(l->string);
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
