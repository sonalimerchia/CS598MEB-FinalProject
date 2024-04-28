#ifndef _QUEUE_H_
#define _QUEUE_H_ 

#include <pthread.h>

typedef struct queue_node {
    struct queue_node* next;
    void* value;
    void (*destroyer) (void*);
} queue_node_t;

typedef struct queue {
    queue_node_t* head;
    queue_node_t* tail;
    pthread_cond_t cond;
    pthread_mutex_t lock;
    size_t capacity;
    size_t num_waiting;
} queue_t;

queue_t* queue_init(size_t capacity);

void queue_push(queue_t*, void*, void (*) (void*));

void* queue_pop(queue_t*);

void queue_destroy(queue_t*);

#endif // _QUEUE_H_