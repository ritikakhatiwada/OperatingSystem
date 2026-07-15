#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// This is our shared variable that all threads will try to modify
// This is dangerous without mutex because threads can interfere with each other
int shared_counter = 0;

// This is our mutex (mutual exclusion lock)
// Only one thread can hold this lock at a time
pthread_mutex_t counter_mutex;

// This function shows what happens WITHOUT mutex protection
// Multiple threads reading and writing at the same time causes wrong results
void *unsafe_increment(void *arg) {
    int thread_id = *(int *)arg;
    int i;

    for (i = 0; i < 100000; i++) {
        // DANGEROUS: no protection here
        // Two threads can read the same value at the same time
        // and both write back, losing one increment
        shared_counter++;
    }

    printf("Thread %d: finished unsafe increment\n", thread_id);
    return NULL;
}

// This function shows what happens WITH mutex protection
// Only one thread can increment at a time, so result is always correct
void *safe_increment(void *arg) {
    int thread_id = *(int *)arg;
    int i;

    for (i = 0; i < 100000; i++) {
        // LOCK the mutex before touching shared data
        // If another thread has the lock, this thread waits here
        pthread_mutex_lock(&counter_mutex);

        // Only one thread can be here at a time
        // This section is called the CRITICAL SECTION
        shared_counter++;

        // UNLOCK so other threads can take their turn
        pthread_mutex_unlock(&counter_mutex);
    }

    printf("Thread %d: finished safe increment\n", thread_id);
    return NULL;
}

int main() {
    pthread_t threads[3];
    int thread_ids[3];
    int i;

    // ---- DEMO 1: WITHOUT MUTEX (race condition) ----
    printf("\n--- WITHOUT Mutex (Race Condition Demo) ---\n");
    printf("Expected result: 300000\n");

    // Reset counter to 0
    shared_counter = 0;

    // Create 3 threads all running unsafe_increment
    for (i = 0; i < 3; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, unsafe_increment, &thread_ids[i]);
    }

    // Wait for all threads to finish
    for (i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    // Result will likely be WRONG because of race condition
    printf("Actual result without mutex: %d (probably wrong!)\n", shared_counter);

    // ---- DEMO 2: WITH MUTEX (safe) ----
    printf("\n--- WITH Mutex (Safe Demo) ---\n");
    printf("Expected result: 300000\n");

    // Reset counter to 0
    shared_counter = 0;

    // Initialize the mutex before using it
    pthread_mutex_init(&counter_mutex, NULL);

    // Create 3 threads all running safe_increment
    for (i = 0; i < 3; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, safe_increment, &thread_ids[i]);
    }

    // Wait for all threads to finish
    for (i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    // Result will ALWAYS be correct because of mutex
    printf("Actual result with mutex: %d (always correct!)\n", shared_counter);

    // Always destroy mutex when done
    pthread_mutex_destroy(&counter_mutex);

    return 0;
}
