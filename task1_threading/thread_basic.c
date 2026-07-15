#include <stdio.h>      // for printf
#include <stdlib.h>     // for exit
#include <pthread.h>    // for thread functions
#include <unistd.h>     // for sleep

// This function is what each thread will run when it starts
// 'arg' is the data we pass to the thread (in our case, the thread number)
void *thread_function(void *arg) {
    
    // Convert the argument to an integer so we can use it as a thread ID
    int thread_id = *(int *)arg;
    
    // Tell us which thread just started
    printf("Thread %d: Starting work...\n", thread_id);
    
    // Pretend the thread is doing some heavy work by sleeping for 1 second
    // In real life this could be reading a file, doing a calculation, etc.
    sleep(1);
    
    // Tell us this thread is done
    printf("Thread %d: Finished work!\n", thread_id);
    
    // Threads must return NULL when they finish
    return NULL;
}

int main() {
    
    // pthread_t is a special type that holds thread information
    // We create an array to hold 3 threads
    pthread_t threads[3];
    
    // We need to pass each thread its own ID number
    // We use an array so each thread gets its own memory address
    int thread_ids[3];
    
    int i;

    printf("Main program: Starting, will create 3 threads\n");

    // Loop to create 3 threads one by one
    for (i = 0; i < 3; i++) {
        
        // Assign thread number (1, 2, 3) to each slot
        thread_ids[i] = i + 1;
        
        // pthread_create actually creates and starts the thread
        // - &threads[i]     : where to store thread info
        // - NULL            : default settings
        // - thread_function : the function the thread will run
        // - &thread_ids[i]  : data we pass to the thread
        if (pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]) != 0) {
            // If thread creation fails, print error and stop
            printf("Error: Could not create thread %d\n", i + 1);
            return 1;
        }
        
        printf("Main program: Thread %d has been created\n", i + 1);
    }

    // Now we wait for all 3 threads to finish before the main program exits
    // Without this, main could exit before threads finish their work
    for (i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
        printf("Main program: Thread %d has completed\n", i + 1);
    }

    printf("Main program: All 3 threads finished successfully!\n");
    return 0;
}
