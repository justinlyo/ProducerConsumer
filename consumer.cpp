#include <iostream>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "sharedMemory.hpp"

int consumed = 0;

void* consume(void *arg) {
    // Opens memory space up and creates a new memory object
    int memory = shm_open(NAME, O_CREAT | O_RDWR, 0700);

    // Allocate memory to SIZE
    if(ftruncate(memory, SIZE)){
        std::cout << "Consumer: ftruncate failed\n";
        exit(-1);
    }

    // Will create a reference to the table in shared memory here
    struct table* sharedTable = (struct table*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memory, 0);
    if (sharedTable == MAP_FAILED){
        std::cout << "Consumer: Map Failed\n";
        exit(-1);
    }

    // Open semaphores
    sem_t *full = sem_open("full", O_CREAT, 0700, 0); // Semaphore for when table is full. If value > 0, then there is an item to consume
    sem_t *empty = sem_open("empty", O_CREAT, 0700, bufferSize); // Semaphore for when table is empty. If value > 0, then there is room to produce
    sem_t *mutex = sem_open("mutex", O_CREAT, 0700, 1); // Semaphore for critical section

    // While loop until total amount produced is consumed
    while(consumed < totalProduction) {
        // Will wait for full semaphore to be ready (if not already). Will wait until there is an item to consume, so when full value > 0.
        sem_wait(full); // Will decrement

        // Wait for critical section semaphore to be ready (if not already). Will wait until there is no one in the critical section, so when value > 0.
        sem_wait(mutex); // Will decrement
        // -- Entering critical section --
        sleep(rand()%2);
        // Output consumed item
        std::cout << "      Consumed: " << sharedTable->data[sharedTable->out] << ", pos: " << sharedTable->out << '\n';
        consumed++;

        // Changes out to the next spot
        sharedTable->out = (sharedTable->out+1)%bufferSize;

        // -- Leaving critical section --
        // Signal the critical section that it is leaving.
        sem_post(mutex); // Will increment

        // Signal the empty semaphore.
        sem_post(empty); // Will increment
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
    std::cout << "-- Consumer begins --\n";
    // Initialize consumer thread
    pthread_t consumer;
    // Creates the consumer thread
    if(pthread_create(&consumer, NULL, consume, NULL)){
        std::cout << "Consumer: could not create thread\n";
        exit(-1);
    }
    // Joins the consumer threads
    if(pthread_join(consumer, NULL)){
        std::cout << "Consumer: could not join thread\n";
        exit(-1);
    }

    std::cout << "--Consumer ends--\n";
    return 0;
}
