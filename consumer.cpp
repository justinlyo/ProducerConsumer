#include <iostream>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "sharedMemory.hpp"

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