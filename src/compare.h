#ifndef _COMPARE_H_
#define _COMPARE_H_

#include <stdbool.h>

#include "cn_profile.h"
#include "queue.h"
#include "utils.h"

#define ZCNT 0
#define CNT 1 
#define CN3 2

typedef struct task {
    cn_profile_t* p1; 
    cn_profile_t* p2;
    int16_t* dest;
    int16_t* dest2; // only for zcnt
    time_t* elapsed;
} task_t;

int main(int argc, char** argv);
void parse_args(int argc, char** argv, char**, char**);

void* worker_routine(void* arg);

void generate_tasks(cn_profile_data_t*, int16_t** distances, time_t* times);
size_t calculate_num_tasks(cn_profile_data_t* data);
size_t calculate_num_zcnt_jobs(cn_profile_data_t* data);
size_t calculate_num_cnt_jobs(cn_profile_data_t* data);
size_t calculate_num_cn3_jobs(cn_profile_data_t* data);
void destroy_task(void* task);

#endif // _COMPARE_H_