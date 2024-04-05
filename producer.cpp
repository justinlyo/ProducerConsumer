#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "sharedMemory.hpp"

int main() {
    std::cout << "Producer begins\n";
    // Opening memory up
    int memory = shm_open(NAME, O_CREAT | O_RDWR, 0700);

    // Allocate appropriate amount of space
    if(ftruncate(memory, SIZE)){
        std::cout << "Producer: ftruncate failed\n";
        exit(-1);
    }
    // Map sharedTable
    struct table* sharedTable = (struct table*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memory, 0);
    if (sharedTable == MAP_FAILED){
        std::cout << "Producer: Map Failed\n";
        exit(-1);
    }


    // Close the shared memory object
    munmap(sharedTable,SIZE);
    close(memory);
    shm_unlink(NAME);

    return 0;
}