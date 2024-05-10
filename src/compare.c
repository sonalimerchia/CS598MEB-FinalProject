#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "compare.h"
#include "read_csv.h"
#include "queue.h"
#include "cn_profile.h"
#include "zcnt.h"
#include "cnt.h"
#include "cn3.h"
#include "utils.h"

const size_t NUM_WORKERS = 20;

int mode = -1;

int main(int argc, char** argv) {
    // Parse arguments
    char* input_file = NULL;
    char* output_prefix = NULL;
    parse_args(argc, argv, &input_file, &output_prefix);
    if (!input_file || !output_prefix || mode == -1) {
        PRINT("Usage Error: ./compare -f <input_file> -o <output_prefix> (-z -c or -t)");
        return -1;
    }

    // Read in data
    LOG("[main]: CSV_FILE: %s", input_file);
    cn_profile_data_t* data = read_cn_profiles(input_file);

    // Generate places to store final data
    LOG("[main]: Creating result storage (%p)", data);
    int16_t** distances = make_distance_matrix(data->num_cells);
    size_t num_jobs = calculate_num_tasks(data);
    time_t* times = calloc(num_jobs, sizeof(time_t));

    data->queue = queue_init(NUM_WORKERS);
    generate_tasks(data, distances, times);
    LOG("[main]: Generated tasks");

    // Generate threads
    pthread_t threads[NUM_WORKERS];
    // Create threads
    for (size_t i = 0; i < NUM_WORKERS; ++i) {
        pthread_create(&threads[i], NULL, worker_routine, data);
    }
    LOG("[main]: Spun up worker threads");

    // Finish threads
    for (size_t i = 0; i < NUM_WORKERS; ++i) {
        pthread_join(threads[i], NULL);
    }
    LOG("[main]: Closed worker threads");

    if (mode == ZCNT) {
        save_distance_matrix(distances, output_prefix, "ZCNT-DIST", data->num_cells);
        save_time_data(times, output_prefix, "ZCNT-TIMES", num_jobs);
    } else if (mode == CNT) {
        save_distance_matrix(distances, output_prefix, "CNT-DIST", data->num_cells);
        save_time_data(times, output_prefix, "CNT-TIMES", num_jobs);
    } else if (mode == CN3) {
        save_distance_matrix(distances, output_prefix, "CN3-DIST", data->num_cells);
        save_time_data(times, output_prefix, "CN3-TIMES", num_jobs);
    }
    LOG("[main]: Saved distance/time data");

    destroy_distance_matrix(distances, data->num_cells);
    queue_destroy(data->queue);
    destroy_cp_profiles(data);
    free(times);
    times = NULL;

    free(input_file);
    input_file = NULL;
    free(output_prefix);
    output_prefix = NULL;

    return 0;
}

void parse_args(int argc, char** argv, char** input_file, char** output_prefix) {
    int opt; 
    int is_zcnt = 0;
    int is_cnt = 0;
    int is_cn3 = 0;
      
    while((opt = getopt(argc, argv, "f:o:zct")) != -1) {  
        switch(opt)  {  
            case 'f':  
                LOG("[parse_args]: filename (%s)", optarg);  
                if (optarg && optarg[0] != '?') {
                    *(input_file) = strdup(optarg);
                }
                break;  
            case 'o':
                LOG("[parse_args]: prefix (%s)", optarg);
                if (optarg && optarg[0] != '?') {
                    *(output_prefix) = strdup(optarg);
                }
                break;
            case 'z': 
                LOG("[parse_args]: zcnt mode");
                is_zcnt = 1;
                break;
            case 'c': 
                LOG("[parse_args]: cnt mode");
                is_cnt = 1;
                break;
            case 't': 
                LOG("[parse_args]: cn3 mode");
                is_cn3 = 1;
                break;
        }   
    }

    if (is_zcnt + is_cnt + is_cn3 != 1) {
        PRINT("Should only pick one kind of distance (-z for zcnt, -c for cnt, -t for cn3)");
        return;
    } 

    if (is_zcnt) {
        mode = ZCNT;
    } else if (is_cnt) {
        mode = CNT;
    } else if (is_cn3) {
        mode = CN3;
    }
}

void* worker_routine(void* arg) {
    cn_profile_data_t* data = arg;
    queue_t* q = data->queue; 
    task_t* task = queue_pop(q);
    size_t num_finished = 0;

    while (task) {
        struct timespec start, end;
        int16_t dist = 0;
        clock_gettime(CLOCK_MONOTONIC, &start);
        if (mode == ZCNT) {
            // ZCNT Distance
            dist = zcnt_distance(task->p1, task->p2, data->num_chrom, data->num_loci);
        } else if (mode == CNT) {
            // CNT Distance
            dist = cnt_distance(task->p1, task->p2, data->num_chrom, data->num_loci);
        } else if (mode == CN3 && data->num_chrom == 1) {
            // CN3 Distance
            dist = cn3_distance(task->p1, task->p2, data->B, data->num_loci);
        } else {
            PRINT("CN3 Distance not set up for multi-chromosomal data");
            exit(1);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);

        if (task->dest) {
            *(task->dest) = dist;
        }

        if (task->dest2) {
            *(task->dest2) = dist;
        }
        *(task->elapsed) = timespecDiff(&end, &start);

        destroy_task(task);
        task = NULL;
        task = queue_pop(q);

        num_finished += 1;
    }

    LOG("[worker_routine]: Worker thread finished %zu jobs", num_finished);

    return NULL;
}

void generate_tasks(cn_profile_data_t* data, int16_t** distances, time_t* times) {
    size_t num_tasks = calculate_num_tasks(data);

    LOG("[generate_tasks]: goal tasks %zu", num_tasks);
    size_t task_ct = 0;

    for (size_t i = 0; i < data->num_cells; ++i) {
        for (size_t j = 0; j < data->num_cells; ++j) {
            // Generate ZCNT distances (symmetric so only need one direction)
            if (mode == ZCNT && i < j) {
                LOG("[generate_tasks]: ZCNT pair (%zu, %zu)", i, j);
                task_t* task = calloc(1, sizeof(task_t));
                *task = (task_t){
                    .p1 = data->profiles + i,
                    .p2 = data->profiles + j,
                    .dest = (int16_t*)(distances[i]) + j,
                    .dest2 = (int16_t*)(distances[j]) + i,
                    .elapsed = times + task_ct,
                };

                queue_push(data->queue, task, destroy_task);
                ++task_ct;
            } else if (mode == CNT && i != j) {
                LOG("[generate_tasks]: CNT pair (%zu, %zu)", i, j);
                task_t* task = calloc(1, sizeof(task_t));
                *task = (task_t){
                    .p1 = data->profiles + i, 
                    .p2 = data->profiles + j, 
                    .dest = (int16_t*)(distances[i]) + j,
                    .dest2 = NULL,
                    .elapsed = times + task_ct,
                };
                queue_push(data->queue, task, destroy_task);
                ++task_ct;
            } else if (mode == CN3 && i < j) {
                LOG("[generate_tasks]: CN3 pair (%zu, %zu)", i, j);
                task_t* task = calloc(1, sizeof(task_t));
                *task = (task_t){
                    .p1 = data->profiles + i, 
                    .p2 = data->profiles + j, 
                    .dest = (int16_t*)(distances[i]) + j,
                    .dest2 = (int16_t*)(distances[j]) + i,
                    .elapsed = times + task_ct,
                };
                queue_push(data->queue, task, destroy_task);
                ++task_ct;
            }
        }
    }

    LOG("[generate_tasks]: Generated %zu out of %zu tasks", task_ct, num_tasks);
}

size_t calculate_num_tasks(cn_profile_data_t* data) {
    if (mode == ZCNT) {
        return calculate_num_zcnt_jobs(data);
    } else if (mode == CNT) {
        return calculate_num_cnt_jobs(data);
    } else if (mode == CN3) {
        return calculate_num_cn3_jobs(data);
    }

    return 0;
}

size_t calculate_num_zcnt_jobs(cn_profile_data_t* data) {
    return (data->num_cells * (data->num_cells - 1)) / 2;
}

size_t calculate_num_cnt_jobs(cn_profile_data_t* data) {
    return data->num_cells * (data->num_cells - 1);
}

size_t calculate_num_cn3_jobs(cn_profile_data_t* data) {
    return (data->num_cells * (data->num_cells - 1)) / 2;
}

void destroy_task(void* t) {
    free(t);
    t = NULL;
}