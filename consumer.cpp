#include <iostream>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "sharedMemory.hpp"

int consumed = 0;

int main() {
    std::cout << "Consumer begins\n";
    // Opening memory up
    int memory = shm_open(NAME, O_CREAT | O_RDWR, 0700);

    // Allocate appropriate amount of space
    if(ftruncate(memory, SIZE)){
        std::cout << "Consumer: ftruncate failed\n";
        exit(-1);
    }
    // Map sharedTable
    struct table* sharedTable = (struct table*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memory, 0);
    if (sharedTable == MAP_FAILED){
        std::cout << "Consumer: Map Failed\n";
        exit(-1);
    }

    // Open semaphores
    sem_t *full = sem_open("full", O_CREAT, 0700, 0); // Semaphore for when table is full (+ helps with entering critical section)
    sem_t *empty = sem_open("empty", O_CREAT, 0700, bufferSize); // Semaphore for when table is empty (+ helps with entering crtiical section)
    sem_t *mutex = sem_open("mutex", O_CREAT, 0700, 1); // Semaphore for critical section


    int count = 10;
    while(count >= 0) {
        // Wait for full semaphore to be ready (if not already). It will decrement the semaphore.
        sem_wait(full);

        // Wait for critical section semaphore to be ready (if not already). It will decrement the semaphore.
        sem_wait(mutex);

        // -- Entering critical section --

        
        // Output consumed item
        std::cout << "\tConsumed: " << sharedTable->data[sharedTable->out] << '\n';

        // Increment consumed
        consumed++;

        // Changes out to the next spot
        sharedTable->out = (sharedTable->out+1)%bufferSize;

        // -- Leaving critical section --
        // Signal the critical section that it is leaving. It will increment the semaphore.
        sem_post(mutex);

        // Signal the empty semaphore. It will increment the semaphore.
        sem_post(empty);
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
    std::cout << "Consumer ends\n";
    return 0;
}