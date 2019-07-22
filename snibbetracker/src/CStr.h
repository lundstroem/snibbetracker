//
//  CAnimation.h
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#ifndef cengine_CStr_h
#define cengine_CStr_h

struct CStr {
    char *chars;
    int size;
};

void cStrTests(void);
int cPrint(const char *fmt,...);
//struct CStr *cStrNew(size_t size, const char *fmt,...);
void cStrResize(struct CStr *cstr, size_t size);
struct CStr *cStrPrint(struct CStr *cstr, const char *fmt,...);
struct CStr *cStrPrintWithSize(size_t size, struct CStr *cstr, const char *fmt,...);
unsigned long cStrLength(struct CStr *cstr);
char cStrCharAt(struct CStr *cstr, int pos);
struct CStr *cStrCleanup(struct CStr *cstr);

#endif
