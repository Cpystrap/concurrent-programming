#include <stddef.h>
#include <pthread.h>
#include <stdatomic.h>
#include <malloc.h>
#include <stdlib.h>
#include <semaphore.h>
#include "common/io.h"
#include "common/sumset.h"

typedef struct {
    // Zadania to wskaźniki na ojców oraz i dla którego tworzę nowy sumset
    const Sumset *a, *b;
    int i;
} Task;

static InputData input_data;
static Solution best_solution;

// Tablica zadań i sumsetów
static Sumset* tab_sumset = NULL;
static Task* tab_tasks = NULL;

static int s = 0; // Indeks ostatniego sumsetu
static atomic_int zz; // Liczba dostępnych zadań
static int z = 0; // Indeks ostatniego zadania (tylko do dodawania zadań)

// Odblokowuje wątki które robią zadania
static sem_t main_semaphore;

// Poziom rekurencji z zadaniami
static int task_level = 3;

// Solution każdego wątku
typedef struct {
    Solution local_solution;
} ThreadData;

static void solve_classic(const Sumset* a, const Sumset* b, Solution* thread_solution)
{
    if (a->sum > b->sum)
        return solve_classic(b, a, thread_solution);

    if (is_sumset_intersection_trivial(a, b)) { // s(a) ∩ s(b) = {0}.
        for (size_t i = a->last; i <= input_data.d; ++i) {
            if (!does_sumset_contain(b, i)) {
                Sumset a_with_i;
                sumset_add(&a_with_i, a, i);
                solve_classic(&a_with_i, b, thread_solution);
            }
        }
    } else if ((a->sum == b->sum) && (get_sumset_intersection_size(a, b) == 2)) { // s(a) ∩ s(b) = {0, ∑b}.
        if (b->sum > thread_solution->sum) {
            solution_build(thread_solution, &input_data, a, b);
        }
    }
}

void* thread_function(void* arg) {
    ThreadData* thread_data = (ThreadData*)arg;
    while (true) {
        int task_idx = atomic_fetch_sub(&zz, 1) - 1;
        if (task_idx < 0)   // nie ma już więcej zadań
            break;

        const Sumset* a = tab_tasks[task_idx].a;
        const Sumset* b = tab_tasks[task_idx].b;
        int i = tab_tasks[task_idx].i;

        if (a->sum > b->sum) {
            const Sumset* tmp = a;
            a = b;
            b = tmp;
        }

        Sumset a_with_i;
        sumset_add(&a_with_i, a, i);
        solve_classic(&a_with_i, b, &thread_data->local_solution);
    }
    return NULL;
}

static void solve(const Sumset* a, const Sumset* b, int level) {
    if (a->sum > b->sum)
        return solve(b, a, level);

    if (is_sumset_intersection_trivial(a, b)) { // s(a) ∩ s(b) = {0}.
        for (size_t i = a->last; i <= input_data.d; ++i) {
            if (!does_sumset_contain(b, i)) {
                if (level <= task_level - 1) {
                    sumset_add(&tab_sumset[s], a, i);
                    s++;
                    solve(&tab_sumset[s - 1], b, level + 1);
                } else if (level == task_level) {
                    tab_tasks[z] = (Task){a, b, i};
                    z++;
                }
            }
        }
    } else if ((a->sum == b->sum) && (get_sumset_intersection_size(a, b) == 2)) {
        if (b->sum > best_solution.sum)
            solution_build(&best_solution, &input_data, a, b);
    }
}

void* main_solver_thread(void* arg) {
    ThreadData* thread_data = (ThreadData*)arg;
    solve(&input_data.a_start, &input_data.b_start, 1);
    zz = z;

    sem_post(&main_semaphore);

    while (true) {
        int task_idx = atomic_fetch_sub(&zz, 1) - 1;
        if (task_idx < 0)   // nie ma już więcej zadań
            break;

        const Sumset* a = tab_tasks[task_idx].a;
        const Sumset* b = tab_tasks[task_idx].b;
        int i = tab_tasks[task_idx].i;

        if (a->sum > b->sum) {
            const Sumset* tmp = a;
            a = b;
            b = tmp;
        }

        Sumset a_with_i;
        sumset_add(&a_with_i, a, i);
        solve_classic(&a_with_i, b, &thread_data->local_solution);
    }

    return NULL;
}

int main() {
    input_data_read(&input_data);
    // input_data_init(&input_data, 1, 3, (int[]){0}, (int[]){0});
    solution_init(&best_solution);

    size_t max_tasks = input_data.d * input_data.d * input_data.d;
    size_t tab_sumset_size = input_data.d * input_data.d + input_data.d;

    tab_sumset = (Sumset*)malloc(tab_sumset_size * sizeof(Sumset));
    if (tab_sumset == NULL) {
        exit(1);
    }
    tab_tasks = (Task*)malloc(max_tasks * sizeof(Task));
    if (tab_tasks == NULL) {
        free(tab_sumset);
        exit(1);
    }

    sem_init(&main_semaphore, 0, 0);
    const int thread_count = input_data.t;
    pthread_t threads[thread_count];
    ThreadData thread_data[thread_count];

    for (int i = 0; i < thread_count; ++i) {
        solution_init(&thread_data[i].local_solution);
    }

    for (int i = 0; i < thread_count; ++i) {
        if (i == 0) {
            pthread_create(&threads[i], NULL, main_solver_thread, &thread_data[i]);
            sem_wait(&main_semaphore);
        } else {
            pthread_create(&threads[i], NULL, thread_function, &thread_data[i]);
        }
    }

    for (int i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Zbieranie wynikow
    size_t max_ind = 0;
    int best_sum = thread_data[0].local_solution.sum;
    for (size_t i = 1; i < thread_count; ++i) {
        if (thread_data[i].local_solution.sum > best_sum) {
            max_ind = i;
            best_sum = thread_data[i].local_solution.sum;
        }
    }
    if (best_sum > best_solution.sum) {
        best_solution = thread_data[max_ind].local_solution;
    }

    free(tab_sumset);
    free(tab_tasks);

    solution_print(&best_solution);
    return 0;
}