#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Number of processes to simulate
#define NUM_PROCESSES 4

// Time quantum is how long each process gets to run before switching
// In real OS this is measured in milliseconds
#define TIME_QUANTUM 2

// Structure to represent a process
typedef struct {
    int process_id;        // unique ID for this process
    int burst_time;        // total CPU time this process needs
    int remaining_time;    // how much CPU time is still needed
    int waiting_time;      // how long process waited in queue
    int turnaround_time;   // total time from start to finish
    int is_finished;       // 1 if process is done, 0 if still running
} Process;

// Mutex to protect process table when threads access it
pthread_mutex_t scheduler_mutex;

// This simulates one round robin turn for a process
void run_process(Process *p, int quantum) {

    // Lock before reading/writing process data
    pthread_mutex_lock(&scheduler_mutex);

    // If process is already finished skip it
    if (p->is_finished) {
        pthread_mutex_unlock(&scheduler_mutex);
        return;
    }

    // Calculate how much time this process will run this turn
    // It runs for quantum time or remaining time whichever is less
    int run_time = (p->remaining_time < quantum) ? p->remaining_time : quantum;

    printf("Process %d: running for %d units (remaining: %d -> %d)\n",
           p->process_id, run_time, p->remaining_time, p->remaining_time - run_time);

    // Reduce remaining time by how much we just ran
    p->remaining_time -= run_time;

    // Check if process has finished
    if (p->remaining_time == 0) {
        p->is_finished = 1;
        printf("Process %d: FINISHED!\n", p->process_id);
    }

    pthread_mutex_unlock(&scheduler_mutex);

    // Simulate actual CPU work with sleep
    sleep(run_time);
}

// Thread function that runs the round robin scheduler
void *scheduler_thread(void *arg) {
    Process *processes = (Process *)arg;
    int current_time = 0;
    int all_finished = 0;
    int i;

    printf("\n=== Round Robin Scheduler Starting ===\n");
    printf("Number of processes: %d\n", NUM_PROCESSES);
    printf("Time quantum: %d units\n\n", TIME_QUANTUM);

    // Keep running until all processes are finished
    while (!all_finished) {
        all_finished = 1;

        // Go through each process one by one (round robin order)
        for (i = 0; i < NUM_PROCESSES; i++) {

            // Skip finished processes
            if (processes[i].is_finished) {
                continue;
            }

            // At least one process is not finished
            all_finished = 0;

            // Update waiting time for all other unfinished processes
            int j;
            for (j = 0; j < NUM_PROCESSES; j++) {
                if (j != i && !processes[j].is_finished) {
                    processes[j].waiting_time += TIME_QUANTUM;
                }
            }

            // Run this process for one quantum
            run_process(&processes[i], TIME_QUANTUM);
            current_time += TIME_QUANTUM;

            // Calculate turnaround time if process just finished
            if (processes[i].is_finished) {
                processes[i].turnaround_time = current_time;
            }
        }
    }

    printf("\n=== All Processes Completed ===\n");
    return NULL;
}

int main() {
    int i;

    // Create 4 processes with different burst times
    Process processes[NUM_PROCESSES] = {
        {1, 6, 6, 0, 0, 0},   // Process 1 needs 6 units of CPU time
        {2, 4, 4, 0, 0, 0},   // Process 2 needs 4 units of CPU time
        {3, 8, 8, 0, 0, 0},   // Process 3 needs 8 units of CPU time
        {4, 2, 2, 0, 0, 0}    // Process 4 needs 2 units of CPU time
    };

    // Initialize mutex
    pthread_mutex_init(&scheduler_mutex, NULL);

    // Print initial process information
    printf("=== Process Table ===\n");
    printf("%-12s %-12s\n", "Process ID", "Burst Time");
    printf("------------------------\n");
    for (i = 0; i < NUM_PROCESSES; i++) {
        printf("%-12d %-12d\n", processes[i].process_id, processes[i].burst_time);
    }

    // Create scheduler thread
    pthread_t scheduler;
    pthread_create(&scheduler, NULL, scheduler_thread, processes);

    // Wait for scheduler to finish
    pthread_join(scheduler, NULL);

    // Print final statistics
    printf("\n=== Final Statistics ===\n");
    printf("%-12s %-12s %-14s %-14s\n",
           "Process", "Burst Time", "Waiting Time", "Turnaround");
    printf("------------------------------------------------\n");

    int total_waiting = 0;
    int total_turnaround = 0;

    for (i = 0; i < NUM_PROCESSES; i++) {
        printf("%-12d %-12d %-14d %-14d\n",
               processes[i].process_id,
               processes[i].burst_time,
               processes[i].waiting_time,
               processes[i].turnaround_time);
        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;
    }

    printf("------------------------------------------------\n");
    printf("Average waiting time:    %.2f units\n", (float)total_waiting / NUM_PROCESSES);
    printf("Average turnaround time: %.2f units\n", (float)total_turnaround / NUM_PROCESSES);

    // Clean up mutex
    pthread_mutex_destroy(&scheduler_mutex);

    return 0;
}
