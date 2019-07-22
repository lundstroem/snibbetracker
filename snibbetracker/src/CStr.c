//
//  CAnimation.c
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include "CAllocator.h"
#include "CStr.h"

// CStr

/*
 The snprintf() function is similar to fprintf(), except that snprintf() places the generated output (up to the specified maximum number of characters) into the character array pointed to by buf, instead of writing it to a file. The snprintf() function is similar to sprintf(), but with boundary checking. A null character is placed at the end of the generated character string.
 
 Returns:
 The number of characters that would have been written into the array, not counting the terminating null character, had count been large enough. It does this even if count is zero; in this case buf can be NULL.
 
 If an error occurred, snprintf() returns a negative value and sets errno.
 */


void cStrTests(void) {
    
    printf("cStrTests start\n");
    cPrint("hejtrtrt:%d",3);
    cPrint("3334");
    
    struct CStr *str = cStrPrint(NULL, "plupp");
    char c = cStrCharAt(str, 5);
    cPrint("char at index 1:%c", c);
    cPrint("char at index 1:%d", c);
    cStrCleanup(str);
    
    int a;
    a=0;
    while(a<=255)
    {
        printf("%d = %c\\n \n",a,a);
        a++;
    }
}

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


/*
struct CStr *cStrNew(size_t size, const char *fmt,...) {
    
    struct CStr *cstr = cAllocatorAlloc(sizeof(struct CStr), "CStr");
    char *temp_chars = cAllocatorAlloc((sizeof(char)*size)+1, "cstr file name chars");
    cstr->chars = temp_chars;
    cstr->size = size;
    if (cstr != NULL) {
        if (cstr->chars != NULL) {
            size_t ret;
            va_list ap;
            va_start(ap, fmt);
            ret = vsnprintf(cstr->chars, size, fmt, ap);
            va_end(ap);
        }
    }
    return cstr;
}*/

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


