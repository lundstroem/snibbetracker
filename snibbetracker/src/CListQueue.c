/*
    CListQueue.c
    cengine
 
    Created by Harry Lundstrom on 9/24/14.
    Copyright (c) 2014 Harry Lundstrom. All rights reserved.
*/

#include <stdio.h>
#include "CListQueue.h"
#include "CAllocator.h"

/*
 TODO: implement pop.
 */

struct CListQueue *cListQueueNew(void)
{
    struct CListQueue *queue = cAllocatorAlloc(sizeof(struct CListQueue), "CListQueue");
    queue->first = NULL;
    queue->last = NULL;
    queue->size = 0;
	return queue;
}

struct CLinearNode **node_pool = NULL;
int node_pool_index = 0;
int node_pool_size = 100;


void cListQueueInitNodePool(void) {
    int i;
    if(node_pool == NULL) {
        /* init pool */
        node_pool = cAllocatorAlloc(node_pool_size * sizeof(struct CLinearNode*), "CLinearNode");
        if(node_pool == NULL) {
            fprintf(stderr, "node_pool out of memory\n");
        }
        for(i = 0; i < node_pool_size; i++) {
            if(node_pool != NULL) {
                node_pool[i] = cLinearNodeNew();
                if(node_pool[i] == NULL) {
                    fprintf(stderr, "node_pool out of memory\n");
                }
            }
        }
    }
}

struct CLinearNode *cListQueueGetFreeNode(void) {
    
    int i;
    int new_pool_size;
    struct CLinearNode **new_node_pool = NULL;
    
    if(node_pool == NULL) {
        cListQueueInitNodePool();
    }
    
    for(i = 0; i < node_pool_size; i++) {
        /* start looking from 0 every time, but if need optimizing, use the node_pool_index. */
        if(node_pool[i]->active == false) {
            node_pool[i]->active = true;
            return node_pool[i];
        }
    }
    
    /* could not find inactive node, need to increase pool size */
    printf("Node pool size of %i too small. Increasing. \n", node_pool_size);
    new_pool_size = node_pool_size * 2;
    new_node_pool = cAllocatorAlloc(new_pool_size * sizeof(struct CLinearNode*), "CLinearNode");
    if(new_node_pool == NULL) {
        fprintf(stderr, "node_pool out of memory\n");
    }
    
    for(i = 0; i < new_pool_size; i++) {
        if(new_node_pool != NULL) {
            if(i < node_pool_size) {
                /* move the nodes from the old pool */
                new_node_pool[i] = node_pool[i];
            } else {
                new_node_pool[i] = cLinearNodeNew();
                if(new_node_pool[i] == NULL) {
                    fprintf(stderr, "buttons out of memory\n");
                }
            }
        }
    }
    
    if(new_node_pool != NULL) {
        node_pool = cAllocatorFree(node_pool);
        node_pool = new_node_pool;
        node_pool_size = new_pool_size;
    
        /* do the search again. */
        for(i = 0; i < node_pool_size; i++) {
            /* start looking from 0 every time, but if need optimizing, use the node_pool_index. */
            if(node_pool[i]->active == false) {
                node_pool[i]->active = true;
                return node_pool[i];
            }
        }
    }
    
    printf("could not find free node");
    return NULL;
}

void cListQueueCleanup() {
    
    int i;
    /* this needs to be called after any other clearing of lists.
    dealloc pool */
    if(node_pool != NULL) {
        for(i = 0; i < node_pool_size; i++) {
            /* start looking from 0 every time, but if need optimizing, use the node_pool_index. */
            if(node_pool[i] != NULL) {
                if(node_pool[i]->active) {
                    printf("cListQueueCleanup error: active node removed from pool. Clear the pool last");
                }
                if(node_pool[i]->data != NULL) {
                    printf("cListQueueCleanup error: node with data removed from pool. Clear the pool last");
                }
                node_pool[i] = cAllocatorFree(node_pool[i]);
            }
        }
        node_pool = cAllocatorFree(node_pool);
    }
}


void cListQueueRecycleNode(struct CLinearNode *node) {
    node->active = false;
    node->data = NULL;
    node->next = NULL;
}

struct CLinearNode *cLinearNodeNew() {
    struct CLinearNode *node = cAllocatorAlloc(sizeof(struct CLinearNode), "CLinearNode");
    node->active = false;
    node->data = NULL;
    node->next = NULL;
    return node;
}

void cListQueueInsert(struct CListQueue *queue, void *data, bool beginning)
{
    struct CLinearNode *node = cListQueueGetFreeNode();
    node->data=data;
    
    if(queue->first == NULL && queue->last == NULL) {
        queue->first = node;
        queue->first->next = NULL;
        queue->last = node;
        queue->last->next = NULL;
    } else if(beginning) {
        node->next = queue->first;
        queue->first = node;
    } else {
        node->next = NULL;
        queue->last->next = node;
        queue->last = node;
    }
    
    queue->size++;
}

void cListQueueInsertAtIndex(struct CListQueue *queue, void *data, int index)
{
    struct CLinearNode *node = cAllocatorAlloc(sizeof(struct CLinearNode), "CLinearNode");
	node->data=data;
    
    if(queue->first == NULL && queue->last == NULL) {
        queue->first = node;
        queue->first->next = NULL;
        queue->last = node;
        queue->last->next = NULL;
        queue->size++;
    } else if(index == 0) {
        node->next = queue->first;
        queue->first = node;
        queue->size++;
    } else {
        
        int counter = 0;
        struct CLinearNode *previous = NULL;
        struct CLinearNode *q_node = queue->first;
        while (q_node != NULL) {
            
            if(q_node->next == NULL) {
                /* this is the last node. */
                if(counter == index) {
                    /* glue it in. */
                    previous->next = node;
                    node->next = q_node;
                    queue->size++;
                    return;
                } else {
                    /* The index is too large. Just glue it in last. */
                    node->next = NULL;
                    queue->last->next = node;
                    queue->last = node;
                    queue->size++;
                    return;
                }
            } else if(index == counter) {
                previous->next = node;
                node->next = q_node;
                queue->size++;
                return;
            }
            
            previous = q_node;
            q_node = q_node->next;
            counter++;
        }
    }
}

static void cListQueueFreeData(struct CLinearNode *node)
{
    if(node != NULL) {
        if(node->data != NULL) {
            node->data = cAllocatorFree(node->data);
            node->data = NULL;
        }
        cListQueueRecycleNode(node);
    }
}

static void cListQueueFreeNode(struct CLinearNode* node) {
    if(node != NULL) {
        if(node->data != NULL) {
            /* When we don't have responsibility to free the data. Just null the pointer and free the node.*/
            node->data = NULL;
        }
        cListQueueRecycleNode(node);
    }
}

int cListQueueRemoveNode(struct CListQueue *queue, void *data, bool dealloc)
{
    struct CLinearNode *first = queue->first;
    struct CLinearNode *last = queue->last;
    
    if(first != NULL && last != NULL) {
        /* if 1 item in list */
        if(first == last) {
            if(first->data != NULL) {
                if(first->data == data) {
                    
                    if(dealloc) {
                        cListQueueFreeData(first);
                    } else {
                        cListQueueFreeNode(first);
                    }
                    queue->first = NULL;
                    queue->last = NULL;
                    queue->size--;
                    return 1;
                }
            }
        }
        /* if 2 or more items */
        else
        {
            struct CLinearNode *node = queue->first;
            struct CLinearNode *previous = NULL;
            struct CLinearNode *next = NULL;
            while (node != NULL) {
                if(node->data != NULL) {
                    if(node->data == data) {
                        /* if previous is NULL this is the first item in list */
                        if(previous == NULL) {
                            queue->first = node->next;
                            if(dealloc) {
                                cListQueueFreeData(node);
                            } else {
                                cListQueueFreeNode(node);
                            }
                            queue->size--;
                            return 1;
                        } else {
                            if(node->next == NULL) {
                                /* this is the last item in the list */
                                previous->next = NULL;
                                queue->last = previous;
                                if(dealloc) {
                                    cListQueueFreeData(node);
                                } else {
                                    cListQueueFreeNode(node);
                                }
                                queue->size--;
                                return 1;
                            } else {
                                /* glue together the link and free the node. */
                                previous->next = node->next;
                                if(dealloc) {
                                    cListQueueFreeData(node);
                                } else {
                                    cListQueueFreeNode(node);
                                }
                                queue->size--;
                                return 1;
                            }
                        }
                    }
                }
                
                previous = node;
                next = node->next;
                node = next;
            }
        }
    }
    
    /* no matching data found. */
    return 0;
}

int cListQueueRemoveNodeAtIndex(struct CListQueue *queue, int index, bool dealloc)
{
    int counter = 0;
    struct CLinearNode *node = queue->first;
    while (node != NULL) {
        if(node->data != NULL) {
            if(index == counter) {
                cListQueueRemoveNode(queue, node->data, dealloc);
                return 1;
            }
        }
        
        node = node->next;
        counter++;
    }
    
    /* no matching data found */
    return 0;
}


void cListQueueClearListQueue(struct CListQueue *queue, bool dealloc)
{
    struct CLinearNode *first = queue->first;
    struct CLinearNode *last = queue->last;
    
    if(first != NULL && last != NULL) {
        /* if 1 item in list */
        if(first == last) {
            
            if(dealloc) {
                cListQueueFreeData(first);
            } else {
                cListQueueFreeNode(first);
            }
            queue->first = NULL;
            queue->last = NULL;
        } else {
            /* if 2 or more items */
            struct CLinearNode *node = queue->first;
            while (node != NULL) {
                struct CLinearNode *next = node->next;
                
                if(dealloc) {
                    cListQueueFreeData(node);
                } else {
                    cListQueueFreeNode(node);
                }
                node = next;
            }
            queue->first = NULL;
            queue->last = NULL;
        }
    }
    
    queue->size = 0;
}
