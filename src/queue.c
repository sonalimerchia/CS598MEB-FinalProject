#include <stdlib.h>
#include <pthread.h>

#include "queue.h"
#include "utils.h"

queue_t* queue_init(size_t capacity) {
    queue_t* q = calloc(sizeof(queue_t), 1);
    q->capacity = capacity;
    
    pthread_mutex_init(&(q->lock), NULL);
    pthread_cond_init(&(q->cond), NULL);
    return q;
}

void queue_push(queue_t* q, void* value, void (*destroyer) (void*)) {
    if (!q) {
        LOG("[queue_push]: Passed null queue to method");
        return;
    }

    queue_node_t* new_node = calloc(1, sizeof(queue_node_t));
    new_node->value = value;
    new_node->destroyer = destroyer;

    pthread_mutex_lock(&(q->lock));

    if (q->head == NULL) {
        // initially empty
        q->head = new_node;
        q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }

    pthread_cond_signal(&(q->cond));
    pthread_mutex_unlock(&(q->lock));
    LOG("[queue_push]: Pushed %p", value);
}

void* queue_pop(queue_t* q) {
    pthread_mutex_lock(&(q->lock));
    
    if (q->head == NULL) {
        // If all helpers are blocking
        q->num_waiting += 1;
        LOG("[queue_pop]: Waiting %zu", q->num_waiting);
        if (q->num_waiting == q->capacity) {
            pthread_cond_broadcast(&(q->cond));
        }

        // Wait until either all are blocking or there is an available node
        while (q->num_waiting < q->capacity && q->head == NULL) {
            pthread_cond_wait(&(q->cond), &(q->lock));
        }

        // Done with all tasks
        if (q->head == NULL) {
            LOG("[queue_pop]: Waiting hit capacity, returning null");
            pthread_mutex_unlock(&(q->lock));
            return NULL;
        }

        LOG("[queue_pop]: Done waiting");
        q->num_waiting -= 1;
    }

    queue_node_t* popped = q->head;
    void* ret_val = popped->value;
    q->head = popped->next;
    if (!q->head) {
        q->tail = NULL;
    } 
    free(popped);
    popped = NULL;
    
    pthread_mutex_unlock(&(q->lock));

    LOG("[queue_pop]: Popped %p", ret_val);
    return ret_val;
}

void queue_destroy(queue_t* q) {
    for (queue_node_t* cur = q->head; cur;) {
        cur->destroyer(cur->value);
        cur->value = NULL;
        cur->destroyer = NULL;
        
        queue_node_t* prev = cur;
        cur = cur->next;
        free(prev);
        prev = NULL;
    }
}