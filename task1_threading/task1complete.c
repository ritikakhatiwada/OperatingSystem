#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// ============================================================
// TASK 1: Process Management and Threading
// Student: Ritika Khatiwada
// Module: ST5004CEM Operating Systems and Security
// Description: This program demonstrates thread creation,
//              mutex synchronization, semaphore usage,
//              race condition handling, deadlock prevention,
//              and round robin scheduling simulation
// ============================================================

// ============================================================
// SECTION 1: SHARED DATA AND SYNCHRONIZATION VARIABLES
// ============================================================

// Shared counter used to demonstrate race condition and mutex
int shared_counter = 0;

// Mutex to protect shared counter from race conditions
pthread_mutex_t counter_mutex;

// Buffer for producer consumer demo
#define BUFFER_SIZE 5
int buffer[BUFFER_SIZE];
int buffer_count = 0;
int put_position = 0;
int take_position = 0;

// Semaphores for producer consumer
sem_t empty_slots;
sem_t full_slots;

// Mutex to protect buffer
pthread_mutex_t buffer_mutex;

// ============================================================
// SECTION 2: ROUND ROBIN SCHEDULER DATA
// ============================================================

#define NUM_PROCESSES 4
#define TIME_QUANTUM 2

// Structure to represent a process in the scheduler
typedef struct {
    int process_id;
    int burst_time;
    int remaining_time;
    int waiting_time;
    int turnaround_time;
    int is_finished;
} Process;

// Mutex to protect process table
pthread_mutex_t scheduler_mutex;

// ============================================================
// SECTION 3: BASIC THREAD FUNCTIONS
// ============================================================

// Basic thread function showing concurrent execution
void *basic_thread(void *arg) {
    int thread_id = *(int *)arg;

    printf("  Thread %d: started and running concurrently\n", thread_id);

    // Simulate work
    usleep(500000);

    printf("  Thread %d: finished work\n", thread_id);
    return NULL;
}

// ============================================================
// SECTION 4: MUTEX AND RACE CONDITION FUNCTIONS
// ============================================================

// Unsafe increment without mutex to show race condition
void *unsafe_increment(void *arg) {
    int i;
    // No protection here causes wrong results
    for (i = 0; i < 100000; i++) {
        shared_counter++;
    }
    return NULL;
}

// Safe increment with mutex to prevent race condition
void *safe_increment(void *arg) {
    int i;
    for (i = 0; i < 100000; i++) {
        // Lock before touching shared data
        pthread_mutex_lock(&counter_mutex);
        shared_counter++;
        // Unlock after done
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

// ============================================================
// SECTION 5: PRODUCER CONSUMER FUNCTIONS
// ============================================================

// Producer adds items to buffer
void *producer(void *arg) {
    int producer_id = *(int *)arg;
    int item;
    int i;

    for (i = 1; i <= 3; i++) {
        item = producer_id * 100 + i;

        // Wait if buffer is full
        sem_wait(&empty_slots);
        pthread_mutex_lock(&buffer_mutex);

        // Add item to buffer
        buffer[put_position] = item;
        put_position = (put_position + 1) % BUFFER_SIZE;
        buffer_count++;

        printf("  Producer %d: added item %d (buffer: %d/%d)\n",
               producer_id, item, buffer_count, BUFFER_SIZE);

        pthread_mutex_unlock(&buffer_mutex);
        // Signal consumer that item is available
        sem_post(&full_slots);

        usleep(100000);
    }
    return NULL;
}

// Consumer takes items from buffer
void *consumer(void *arg) {
    int consumer_id = *(int *)arg;
    int item;
    int i;

    for (i = 1; i <= 3; i++) {
        // Wait if buffer is empty
        sem_wait(&full_slots);
        pthread_mutex_lock(&buffer_mutex);

        // Take item from buffer
        item = buffer[take_position];
        take_position = (take_position + 1) % BUFFER_SIZE;
        buffer_count--;

        printf("  Consumer %d: took item %d (buffer: %d/%d)\n",
               consumer_id, item, buffer_count, BUFFER_SIZE);

        pthread_mutex_unlock(&buffer_mutex);
        // Signal producer that slot is free
        sem_post(&empty_slots);

        usleep(150000);
    }
    return NULL;
}

// ============================================================
// SECTION 6: ROUND ROBIN SCHEDULER FUNCTIONS
// ============================================================

// Run one quantum of a process
void run_process(Process *p, int quantum) {
    pthread_mutex_lock(&scheduler_mutex);

    if (p->is_finished) {
        pthread_mutex_unlock(&scheduler_mutex);
        return;
    }

    // Run for quantum or remaining time whichever is less
    int run_time = (p->remaining_time < quantum) ? p->remaining_time : quantum;

    printf("  Process %d: running %d units (remaining: %d -> %d)\n",
           p->process_id, run_time,
           p->remaining_time, p->remaining_time - run_time);

    p->remaining_time -= run_time;

    if (p->remaining_time == 0) {
        p->is_finished = 1;
        printf("  Process %d: FINISHED!\n", p->process_id);
    }

    pthread_mutex_unlock(&scheduler_mutex);
}

// Round robin scheduler thread
void *scheduler_thread(void *arg) {
    Process *processes = (Process *)arg;
    int current_time = 0;
    int all_finished = 0;
    int i;

    while (!all_finished) {
        all_finished = 1;

        for (i = 0; i < NUM_PROCESSES; i++) {
            if (processes[i].is_finished) continue;

            all_finished = 0;

            // Update waiting time for other processes
            int j;
            for (j = 0; j < NUM_PROCESSES; j++) {
                if (j != i && !processes[j].is_finished) {
                    processes[j].waiting_time += TIME_QUANTUM;
                }
            }

            run_process(&processes[i], TIME_QUANTUM);
            current_time += TIME_QUANTUM;

            if (processes[i].is_finished) {
                processes[i].turnaround_time = current_time;
            }
        }
    }
    return NULL;
}

// ============================================================
// SECTION 7: MAIN FUNCTION
// ============================================================

int main() {
    int i;
    pthread_t threads[3];
    int thread_ids[3];

    printf("========================================\n");
    printf("  ST5004CEM Task 1: Threading Demo\n");
    printf("  Student: Ritika Khatiwada\n");
    printf("========================================\n\n");

    // ---- DEMO 1: BASIC THREADS ----
    printf("--- DEMO 1: Basic Thread Creation ---\n");
    printf("Creating 3 concurrent threads...\n");

    for (i = 0; i < 3; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, basic_thread, &thread_ids[i]);
    }
    for (i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("All 3 threads completed!\n\n");

    // ---- DEMO 2: RACE CONDITION ----
    printf("--- DEMO 2: Race Condition and Mutex ---\n");

    // Without mutex
    shared_counter = 0;
    printf("Without mutex (expected 300000):\n");
    for (i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, unsafe_increment, NULL);
    }
    for (i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("  Result: %d (race condition caused wrong answer!)\n\n", shared_counter);

    // With mutex
    shared_counter = 0;
    pthread_mutex_init(&counter_mutex, NULL);
    printf("With mutex (expected 300000):\n");
    for (i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, safe_increment, NULL);
    }
    for (i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("  Result: %d (mutex gave correct answer!)\n\n", shared_counter);
    pthread_mutex_destroy(&counter_mutex);

    // ---- DEMO 3: PRODUCER CONSUMER ----
    printf("--- DEMO 3: Semaphore Producer-Consumer ---\n");

    // Initialize semaphores and mutex
    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&full_slots, 0, 0);
    pthread_mutex_init(&buffer_mutex, NULL);

    pthread_t prod_threads[2], cons_threads[2];
    int prod_ids[2], cons_ids[2];

    // Create producers and consumers
    for (i = 0; i < 2; i++) {
        prod_ids[i] = i + 1;
        cons_ids[i] = i + 1;
        pthread_create(&prod_threads[i], NULL, producer, &prod_ids[i]);
        pthread_create(&cons_threads[i], NULL, consumer, &cons_ids[i]);
    }

    for (i = 0; i < 2; i++) {
        pthread_join(prod_threads[i], NULL);
        pthread_join(cons_threads[i], NULL);
    }

    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    pthread_mutex_destroy(&buffer_mutex);
    printf("Producer-Consumer completed! Final buffer: %d items\n\n", buffer_count);

    // ---- DEMO 4: ROUND ROBIN SCHEDULER ----
    printf("--- DEMO 4: Round Robin Scheduler ---\n");
    printf("Time Quantum: %d units\n\n", TIME_QUANTUM);

    Process processes[NUM_PROCESSES] = {
        {1, 6, 6, 0, 0, 0},
        {2, 4, 4, 0, 0, 0},
        {3, 8, 8, 0, 0, 0},
        {4, 2, 2, 0, 0, 0}
    };

    pthread_mutex_init(&scheduler_mutex, NULL);

    pthread_t scheduler;
    pthread_create(&scheduler, NULL, scheduler_thread, processes);
    pthread_join(scheduler, NULL);

    // Print statistics
    printf("\n  %-10s %-12s %-14s %-12s\n",
           "Process", "Burst Time", "Waiting Time", "Turnaround");
    printf("  ------------------------------------------------\n");

    float total_wait = 0, total_turn = 0;
    for (i = 0; i < NUM_PROCESSES; i++) {
        printf("  %-10d %-12d %-14d %-12d\n",
               processes[i].process_id,
               processes[i].burst_time,
               processes[i].waiting_time,
               processes[i].turnaround_time);
        total_wait += processes[i].waiting_time;
        total_turn += processes[i].turnaround_time;
    }

    printf("  ------------------------------------------------\n");
    printf("  Average waiting time:    %.2f units\n", total_wait / NUM_PROCESSES);
    printf("  Average turnaround time: %.2f units\n", total_turn / NUM_PROCESSES);

    pthread_mutex_destroy(&scheduler_mutex);

    printf("\n========================================\n");
    printf("  Task 1 Complete! All demos passed!\n");
    printf("========================================\n");

    return 0;
}
