#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "compare.h"
#include "read_csv.h"
#include "queue.h"
#include "cn_profile.h"
#include "zcnt.h"
#include "cnt.h"
#include "utils.h"

const size_t NUM_WORKERS = 1;

int main(int argc, char** argv) {
    if (argc < 2) {
        PRINT("Incorrect Usage: ./main [data.csv]");
    }

    LOG("[main] CSV_FILE: %s", argv[1]);
    cn_profile_data_t* data = read_cn_profiles(argv[1]);
    LOG("[main] READ DATA (%p)", data);

    // Generate worker-queue infrastructure
    int16_t** zcnt_distances = make_distance_matrix(data->num_cells);
    int16_t** cnt_distances = make_distance_matrix(data->num_cells);

    size_t num_zcnt_jobs = calculate_num_zcnt_jobs(data);
    size_t num_cnt_jobs = calculate_num_cnt_jobs(data);
    time_t* zcnt_times = calloc(num_zcnt_jobs, sizeof(time_t));
    time_t* cnt_times = calloc(num_cnt_jobs, sizeof(time_t));

    data->queue = queue_init(NUM_WORKERS);
    LOG("[main] GENERATED QUEUE");

    generate_tasks(data, cnt_distances, zcnt_distances, cnt_times, zcnt_times);
    LOG("[main] GENERATED TASKS");

    // Generate threads
    pthread_t threads[NUM_WORKERS];
    
    // Create threads
    for (size_t i = 0; i < NUM_WORKERS; ++i) {
        pthread_create(&threads[i], NULL, worker_routine, data);
    }

    // Finish threads
    for (size_t i = 0; i < NUM_WORKERS; ++i) {
        pthread_join(threads[i], NULL);
    }

    save_distance_matrix(zcnt_distances, "ZCNT-DIST", data->num_cells);
    save_distance_matrix(cnt_distances, "CNT-DIST", data->num_cells);

    save_time_data(zcnt_times, "ZCNT-TIMES", num_zcnt_jobs);
    save_time_data(cnt_times, "CNT-TIMES", num_cnt_jobs);

    destroy_cp_profiles(data);

    return 0;
}

void* worker_routine(void* arg) {
    cn_profile_data_t* data = arg;
    queue_t* q = data->queue; 
    task_t* task = queue_pop(q);
    size_t num_finished = 0;

    while (task) {
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        if (task->is_zcnt) {
            // ZCNT Distance
            time_t start = time(NULL);
            int16_t dist = zcnt_distance(task->p1, task->p2, data->num_loci);
            time_t end = time(NULL);
            *(task->dest) = dist;
            *(task->dest2) = dist;
            *(task->elapsed) = end - start;
        } else {
            // CNT Distance
            time_t start = time(NULL);
            *(task->dest) = cnt_distance(task->p1, task->p2, data->num_loci);
            time_t end = time(NULL);
            *(task->elapsed) = end - start;
        }
        clock_gettime(CLOCK_MONOTONIC, &end);

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