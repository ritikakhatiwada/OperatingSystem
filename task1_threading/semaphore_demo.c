#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// This is our shared buffer where producer puts items
// and consumer takes items from
#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];   // the shared buffer
int buffer_count = 0;      // how many items are currently in buffer

// Semaphores to control access to the buffer
sem_t empty_slots;   // counts how many empty slots are in buffer
sem_t full_slots;    // counts how many full slots are in buffer

// Mutex to protect buffer access
pthread_mutex_t buffer_mutex;

// Keep track of where to put and take items
int put_position = 0;
int take_position = 0;

// Producer thread: keeps creating items and putting them in buffer
void *producer(void *arg) {
    int producer_id = *(int *)arg;
    int item;
    int i;

    for (i = 1; i <= 8; i++) {
        // Create an item to produce
        item = producer_id * 100 + i;

        // Wait if buffer is full (no empty slots available)
        // This prevents producer from overfilling the buffer
        sem_wait(&empty_slots);

        // Lock mutex before touching the buffer
        pthread_mutex_lock(&buffer_mutex);

        // Put item into buffer at current position
        buffer[put_position] = item;
        put_position = (put_position + 1) % BUFFER_SIZE;
        buffer_count++;

        printf("Producer %d: added item %d to buffer (buffer has %d items)\n",
               producer_id, item, buffer_count);

        // Unlock mutex after we are done with buffer
        pthread_mutex_unlock(&buffer_mutex);

        // Signal that one more full slot is available for consumer
        sem_post(&full_slots);

        // Small sleep to make output easier to read
        usleep(100000);
    }

    printf("Producer %d: finished producing\n", producer_id);
    return NULL;
}

// Consumer thread: keeps taking items from buffer and processing them
void *consumer(void *arg) {
    int consumer_id = *(int *)arg;
    int item;
    int i;

    for (i = 1; i <= 8; i++) {
        // Wait if buffer is empty (no full slots available)
        // This prevents consumer from taking from empty buffer
        sem_wait(&full_slots);

        // Lock mutex before touching the buffer
        pthread_mutex_lock(&buffer_mutex);

        // Take item from buffer at current position
        item = buffer[take_position];
        take_position = (take_position + 1) % BUFFER_SIZE;
        buffer_count--;

        printf("Consumer %d: took item %d from buffer (buffer has %d items)\n",
               consumer_id, item, buffer_count);

        // Unlock mutex after we are done with buffer
        pthread_mutex_unlock(&buffer_mutex);

        // Signal that one more empty slot is available for producer
        sem_post(&empty_slots);

        // Small sleep to simulate processing the item
        usleep(150000);
    }

    printf("Consumer %d: finished consuming\n", consumer_id);
    return NULL;
}

int main() {
    // We will have 2 producers and 2 consumers
    pthread_t producer_threads[2];
    pthread_t consumer_threads[2];
    int producer_ids[2];
    int consumer_ids[2];
    int i;

    printf("=== Semaphore Producer-Consumer Demo ===\n");
    printf("Buffer size: %d slots\n\n", BUFFER_SIZE);

    // Initialize semaphores
    // empty_slots starts at BUFFER_SIZE because buffer is empty at start
    sem_init(&empty_slots, 0, BUFFER_SIZE);

    // full_slots starts at 0 because buffer has no items at start
    sem_init(&full_slots, 0, 0);

    // Initialize mutex
    pthread_mutex_init(&buffer_mutex, NULL);

    // Create 2 producer threads
    for (i = 0; i < 2; i++) {
        producer_ids[i] = i + 1;
        pthread_create(&producer_threads[i], NULL, producer, &producer_ids[i]);
    }

    // Create 2 consumer threads
    for (i = 0; i < 2; i++) {
        consumer_ids[i] = i + 1;
        pthread_create(&consumer_threads[i], NULL, consumer, &consumer_ids[i]);
    }

    // Wait for all producers to finish
    for (i = 0; i < 2; i++) {
        pthread_join(producer_threads[i], NULL);
    }

    // Wait for all consumers to finish
    for (i = 0; i < 2; i++) {
        pthread_join(consumer_threads[i], NULL);
    }

    // Clean up semaphores and mutex
    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    pthread_mutex_destroy(&buffer_mutex);

    printf("\nAll producers and consumers finished!\n");
    printf("Final buffer count: %d (should be 0)\n", buffer_count);

    return 0;
}
