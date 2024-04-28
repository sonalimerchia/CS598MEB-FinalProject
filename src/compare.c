#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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
    double** zcnt_distances = make_distance_matrix(data->num_cells);
    double** cnt_distances = make_distance_matrix(data->num_cells);

    data->queue = queue_init(NUM_WORKERS);
    LOG("[main] GENERATED QUEUE");

    generate_tasks(data, cnt_distances, zcnt_distances);
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

    destroy_cp_profiles(data);

    return 0;
}

void* worker_routine(void* arg) {
    cn_profile_data_t* data = arg;
    queue_t* q = data->queue; 
    task_t* task = queue_pop(q);
    size_t num_finished = 0;

    while (task) {
        if (task->is_zcnt) {
            // ZCNT Distance
            double dist = zcnt_distance(task->p1, task->p2, data->num_loci);
            *(task->dest) = dist;
            *(task->dest2) = dist;
            LOG("ZCNT %f", dist);
        } else {
            // CNT Distance
            *(task->dest) = cnt_distance(task->p1, task->p2, data->num_loci);
            LOG("CNT %f", *(task->dest));
        }

        destroy_task(task);
        task = NULL;
        task = queue_pop(q);

        num_finished += 1;
    }

    LOG("[worker_routine]: Worker thread finished %zu jobs", num_finished);

    return NULL;
}

void generate_tasks(cn_profile_data_t* data, double** cnt, double** zcnt) {
    size_t num_tasks = calculate_num_tasks(data);

    LOG("[generate_tasks]: goal tasks %zu", num_tasks);
    size_t task_count = 0;
    for (size_t i = 0; i < data->num_cells; ++i) {
        for (size_t j = 0; j < data->num_cells; ++j) {
            // Generate ZCNT distances (symmetric so only need one direction)
            if (i < j) {
                LOG("[generate_tasks]: ZCNT pair (%zu, %zu)", i, j);
                task_t* task = calloc(1, sizeof(task_t));
                task->is_zcnt = true;
                task->p1 = data->profiles + i;
                task->p2 = data->profiles + j;
                task->dest = (double*)(zcnt[i]) + j;
                task->dest2 = (double*)(zcnt[j]) + i;

                queue_push(data->queue, task, destroy_task);
                ++task_count;
            }

            if (i != j) {
                // Generate CNT distances (assymmetric so need both directions)
                LOG("[generate_tasks]: CNT pair (%zu, %zu)", i, j);
                task_t* task = calloc(1, sizeof(task_t));
                *task = (task_t){
                    .is_zcnt = false,
                    .p1 = data->profiles + i, 
                    .p2 = data->profiles + j, 
                    .dest = (double*)(cnt[i]) + j,
                    .dest2 = NULL,
                };
                queue_push(data->queue, task, destroy_task);
                ++task_count;
            }
        }
    }

    LOG("[generate_tasks]: Generated %zu out of %zu tasks", task_count, num_tasks);
}

size_t calculate_num_tasks(cn_profile_data_t* data) {
    size_t num_zcnt_jobs = (data->num_cells * (data->num_cells - 1)) / 2;
    size_t num_cnt_jobs = data->num_cells * (data->num_cells - 1);
    return num_cnt_jobs + num_zcnt_jobs;
}

void destroy_task(void* t) {
    free(t);
    t = NULL;
}