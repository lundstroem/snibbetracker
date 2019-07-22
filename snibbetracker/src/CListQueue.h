//
//  CListQueue.h
//  cengine
//
//  Created by Harry Lundstrom on 9/24/14.
//  Copyright (c) 2014 Harry Lundstrom. All rights reserved.
//
#include <stdbool.h>

#ifndef cengine_CListQueue_h
#define cengine_CListQueue_h

struct CLinearNode
{
    bool active;
    void *data;
    struct CLinearNode *next;
};

struct CListQueue
{
    struct CLinearNode *first;
    struct CLinearNode *last;
    int size;
};

void cListQueueCleanup(void);
struct CListQueue *cListQueueNew(void);
struct CLinearNode *cLinearNodeNew(void);
void cListQueueInsert(struct CListQueue *queue, void *data, bool beginning);
void cListQueueInsertAtIndex(struct CListQueue *queue, void *data, int index);
int cListQueueRemoveNode(struct CListQueue *queue, void *data, bool dealloc);
int cListQueueRemoveNodeAtIndex(struct CListQueue *queue, int index, bool dealloc);
void cListQueueClearListQueue(struct CListQueue *queue, bool dealloc);

#endif
