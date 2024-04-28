#ifndef _COMPARE_H_
#define _COMPARE_H_

#include <stdbool.h>

#include "cn_profile.h"
#include "queue.h"
#include "utils.h"

typedef struct task {
    bool is_zcnt;
    cn_profile_t* p1; 
    cn_profile_t* p2;
    double* dest;
    double* dest2; // only for zcnt
} task_t;

int main(int argc, char** argv);

void* worker_routine(void* arg);

void generate_tasks(cn_profile_data_t*, double** cnt, double** zcnt);

size_t calculate_num_tasks(cn_profile_data_t* data);

void destroy_task(void* task);

#endif // _COMPARE_H_