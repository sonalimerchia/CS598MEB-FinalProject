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
#include "utils.h"

const size_t NUM_WORKERS = 1;

int main(int argc, char** argv) {
    // Parse arguments
    char* input_file = NULL;
    char* output_prefix = NULL;
    parse_args(argc, argv, &input_file, &output_prefix);
    if (!input_file || !output_prefix) {
        PRINT("Usage Error: ./compare -f <input_file> -o <output_prefix>");
        return -1;
    }

    // Read in data
    LOG("[main]: CSV_FILE: %s", input_file);
    cn_profile_data_t* data = read_cn_profiles(input_file);

    // Generate places to store final data
    LOG("[main]: Creating result storage (%p)", data);
    int16_t** zcnt_distances = make_distance_matrix(data->num_cells);
    int16_t** cnt_distances = make_distance_matrix(data->num_cells);
    size_t num_zcnt_jobs = calculate_num_zcnt_jobs(data);
    size_t num_cnt_jobs = calculate_num_cnt_jobs(data);
    time_t* zcnt_times = calloc(num_zcnt_jobs, sizeof(time_t));
    time_t* cnt_times = calloc(num_cnt_jobs, sizeof(time_t));

    data->queue = queue_init(NUM_WORKERS);
    generate_tasks(data, cnt_distances, zcnt_distances, cnt_times, zcnt_times);
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

    save_distance_matrix(zcnt_distances, output_prefix, "ZCNT-DIST", data->num_cells);
    save_distance_matrix(cnt_distances, output_prefix, "CNT-DIST", data->num_cells);
    LOG("[main]: Saved distance data");

    save_time_data(zcnt_times, output_prefix, "ZCNT-TIMES", num_zcnt_jobs);
    save_time_data(cnt_times, output_prefix, "CNT-TIMES", num_cnt_jobs);
    LOG("[main]: Saved time data");

    destroy_distance_matrix(zcnt_distances, data->num_cells);
    destroy_distance_matrix(cnt_distances, data->num_cells);
    queue_destroy(data->queue);
    destroy_cp_profiles(data);

    free(zcnt_times);
    zcnt_times = NULL;
    free(cnt_times);
    cnt_times = NULL;
    free(input_file);
    input_file = NULL;
    free(output_prefix);
    output_prefix = NULL;

    return 0;
}

void parse_args(int argc, char** argv, char** input_file, char** output_prefix) {
    int opt; 
      
    while((opt = getopt(argc, argv, "f:o:")) != -1) {  
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
        }   
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
        if (task->is_zcnt) {
            // ZCNT Distance
            dist = zcnt_distance(task->p1, task->p2, data->num_loci);
        } else {
            // CNT Distance
            dist = cnt_distance(task->p1, task->p2, data->num_loci);
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

void generate_tasks(cn_profile_data_t* data, int16_t** cnt, int16_t** zcnt, time_t* cnt_times, time_t* zcnt_times) {
    size_t num_tasks = calculate_num_tasks(data);

    LOG("[generate_tasks]: goal tasks %zu", num_tasks);
    size_t zcnt_ct = 0;
    size_t cnt_ct = 0;

    for (size_t i = 0; i < data->num_cells; ++i) {
        for (size_t j = 0; j < data->num_cells; ++j) {
            // Generate ZCNT distances (symmetric so only need one direction)
            if (i < j) {
                LOG("[generate_tasks]: ZCNT pair (%zu, %zu)", i, j);
                task_t* task = calloc(1, sizeof(task_t));
                *task = (task_t){
                    .is_zcnt = true,
                    .p1 = data->profiles + i,
                    .p2 = data->profiles + j,
                    .dest = (int16_t*)(zcnt[i]) + j,
                    .dest2 = (int16_t*)(zcnt[j]) + i,
                    .elapsed = zcnt_times + zcnt_ct,
                };

                queue_push(data->queue, task, destroy_task);
                ++zcnt_ct;
            }

            if (i != j) {
                // Generate CNT distances (assymmetric so need both directions)
                LOG("[generate_tasks]: CNT pair (%zu, %zu)", i, j);
                task_t* task = calloc(1, sizeof(task_t));
                *task = (task_t){
                    .is_zcnt = false,
                    .p1 = data->profiles + i, 
                    .p2 = data->profiles + j, 
                    .dest = (int16_t*)(cnt[i]) + j,
                    .dest2 = NULL,
                    .elapsed = cnt_times + cnt_ct,
                };
                queue_push(data->queue, task, destroy_task);
                ++cnt_ct;
            }
        }
    }

    LOG("[generate_tasks]: Generated %zu out of %zu tasks", cnt_ct + zcnt_ct, num_tasks);
}

size_t calculate_num_tasks(cn_profile_data_t* data) {
    return calculate_num_zcnt_jobs(data) + calculate_num_cnt_jobs(data);
}

size_t calculate_num_zcnt_jobs(cn_profile_data_t* data) {
    return (data->num_cells * (data->num_cells - 1)) / 2;
}

size_t calculate_num_cnt_jobs(cn_profile_data_t* data) {
    return data->num_cells * (data->num_cells - 1);
}

void destroy_task(void* t) {
    free(t);
    t = NULL;
}