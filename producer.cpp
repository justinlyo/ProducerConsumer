#include <iostream>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "sharedMemory.hpp"

int produced = 0;

void* produce(void *arg) {
    // Opens memory space up and creates a new memory object
    int memory = shm_open(NAME, O_CREAT | O_RDWR, 0700);

    // Allocate memory to SIZE
    if(ftruncate(memory, SIZE)){
        std::cout << "Producer: ftruncate failed\n";
        exit(-1);
    }

    // Will create a reference to the table in shared memory here
    struct table* sharedTable = (struct table*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memory, 0);
    if (sharedTable == MAP_FAILED){
        std::cout << "Producer: Map Failed\n";
        exit(-1);
    }

    // Initialize starting variables.
    sharedTable->in = 0;
    sharedTable->out = 0;

    // Open semaphores
    sem_t *full = sem_open("full", O_CREAT, 0700, 0); // Semaphore for when table is full. If value > 0, then there is an item to consume
    sem_t *empty = sem_open("empty", O_CREAT, 0700, bufferSize); // Semaphore for when table is empty. If value > 0, then there is room to produce
    sem_t *mutex = sem_open("mutex", O_CREAT, 0700, 1); // Semaphore for critical section

    // While loop until total amount produced is produced
    while(produced < totalProduction) {
        // Wait for empty semaphore to be ready (if not already). Will wait until there is an empty space, so when empty > 0.
        sem_wait(empty); // Will decrement

        // Wait for critical section semaphore to be ready (if not already). Will wait until there is no one in the critical section, so when value > 0.
        sem_wait(mutex); // Will decrement
        sleep(rand()%2);
        // Enter critical Section

        // Generate random value
        int randomValue = rand() % 100;

        // Insert random value into shared memory
        sharedTable->data[sharedTable->in] = randomValue;

        // Output produced item
        std::cout << "   Produced: " << sharedTable->data[sharedTable->in] << ", pos: " << sharedTable->in << '\n';
        produced++;

        // Changes in to the next available spot
        sharedTable->in = (sharedTable->in+1)%bufferSize;

        // -- Leaving critical section --
        // Signal the critical section that it is leaving. 
        sem_post(mutex); // Will increment

        // Signal the full semaphore.
        sem_post(full); // Will increment

    }

    // Close semaphores
    sem_close(full);
    sem_close(empty);
    sem_close(mutex);

    // Unlink semaphores
    sem_unlink("full");
    sem_unlink("empty");
    sem_unlink("mutex");

    // Close the shared memory object
    munmap(sharedTable,SIZE);
    close(memory);
    shm_unlink(NAME);


    return nullptr;
}




int main() {
    std::cout << "-- Producer begins --\n";
    // Initialize producer thread
    pthread_t producer;
    // Creates the producer thread
    if(pthread_create(&producer, NULL, produce, NULL)){
        std::cout << "Producer: could not create thread\n";
        exit(-1);
    }
    // Joins the producer thread
    if(pthread_join(producer, NULL)){
        std::cout << "Producer: could not join thread\n";
        exit(-1);
    }

    std::cout << "-- Producer ends --\n";
    return 0;
}
