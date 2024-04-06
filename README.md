# Producer Consumer Problem
Justin Lyogky

CS 33211<br /> 
#### Programming Assignment #1<br /> 
The producer generates items and puts items onto the table. The consumer will pick up items. The table can only hold two items at the same time. When the table is complete, the producer will wait. When there are no items, the consumer will wait. We use semaphores to synchronize producer and consumer.  Mutual exclusion should be considered. We use threads in the producer program and consumer program. Shared memory is used for the “table”.

# Instructions
Clone the git repository with the following:
```
gh repo clone justinlyo/ProducerConsumer
````
Once you have cloned the git repository, you should be able to access the producer.cpp, consumer.cpp, and sharedMemory.hpp files. The next step is to compile these files using the following commands.
```
g++ producer.cpp -pthread -lrt -o producer
g++ consumer.cpp -pthread -lrt -o consumer
```
Once you have compiled these programs, we are able to run both the producer and consumer simultaneously with this command.
```
./producer & ./consumer &
```
The following is an example of what the output could look like; however, it should be different for everyone. Here, we are using a totalProduction value of 15 so that only 15 items are produced. 
```
-- Producer begins --
-- Consumer begins --
   Produced: 86, pos: 0
   Produced: 15, pos: 1
      Consumed: 86, pos: 0
      Consumed: 15, pos: 1
   Produced: 35, pos: 0
   Produced: 92, pos: 1
      Consumed: 35, pos: 0
      Consumed: 92, pos: 1
   Produced: 21, pos: 0
   Produced: 27, pos: 1
      Consumed: 21, pos: 0
      Consumed: 27, pos: 1
   Produced: 59, pos: 0
   Produced: 26, pos: 1
      Consumed: 59, pos: 0
      Consumed: 26, pos: 1
   Produced: 26, pos: 0
   Produced: 36, pos: 1
      Consumed: 26, pos: 0
      Consumed: 36, pos: 1
   Produced: 68, pos: 0
   Produced: 29, pos: 1
      Consumed: 68, pos: 0
      Consumed: 29, pos: 1
   Produced: 30, pos: 0
   Produced: 23, pos: 1
      Consumed: 30, pos: 0
      Consumed: 23, pos: 1
   Produced: 35, pos: 0
-- Producer ends --
      Consumed: 35, pos: 0
--Consumer ends--
```
# Video Walkthrough
add video

# Explanation
The assignment was to have a producer produce items for the consumer to consume. The producer and consumer have to be use semaphores and have mutual exclusion, as well as use threads.

## Overview
Our producer.cpp file acts as our producer. It has only one variable `produced`, which is to keep track of the amount it has produced. There are two functions, a `main()` and a `produce(void *arg)`, in which the main is in charge of creating the thread and produce is in charge of producing the items while using semaphores. We then have the sharedMemory.hpp, which holds the table, a SIZE, bufferSize, NAME of the memory, and totalProduction value. The table has three values, data, in, and out. The data variable is the most important since it is where items will be produced or consumed. The in variable will hold the index of the next free available space that the producer may be able to produce an item into. The out variable will hold the index of the next item that the consumer may be able to consume. These three values are important for the proper execution of the program, which is why they should be kept in the critical section. An explanation about what the critical section is will be further down.

## Threading
Threading allows for a single application to perform more than one task at the same time. One process can have multiple threads, which run independently from each other. For the producer-consumer problem, we use one thread for the producer and one thread for the consumer. Although this does not make much of a difference in execution, it allows us the opportunity to expand on it further if we wanted to add more than one thread for each process. However, only one thread each is required for the assignment.

In the `main()` function of the producer and consumer programs, we have the following code.
#### Producer
```
pthread_t producer;
if(pthread_create(&producer, NULL, produce, NULL)) {...}
if(pthread_join(producer, NULL){...}
```
#### Consumer
```
pthread_t consumer;
if(pthread_create(&consumer, NULL, consume, NULL)) {...}
if(pthread_join(consumer, NULL){...}
```
We first initialize a thread in both of these programs named either producer or consumer with the pthread_t. Then the pthread_create(...) will use OS calls to create a thread for the producer or consumer (depending on the program), and will make the threads executable. Finally, we use pthread_join(...) to ensure data consistency by waiting for specific threads to finish their execution, although it would not matter much since we have semaphores locking the critical section.

## Shared Memory
We need the producer and consumer to be able to both access the data table in which they will either produce or consume items. We do this in our program by using shared memory. 
In the `produce(void *arg){...}` and `consume(void *arg){...}` functions, we have the following.
```
int memory = shm_open(NAME, O_CREAT | O_RDWR, 0700);
```
We are creating a new POSIX shared memory object (or opening an existing one). The NAME variable is shared inside the sharedMemory.hpp, so that both the producer and consumer have access to it. The O_CREAT will create a shared memory object if it does not already exist and the O_RDWR will allow the object to be read and edited. The 0700 is the permissions of the object.
We then need to allocate the appropriate size of the shared memory object and we do this by the following code, which is both in the producer and consumer.
```
if(ftruncate(memory, SIZE)){...}
```
This will set the size of the shared memory object to the SIZE variable, which is in sharedMemory and is the size of the table. Now we will need to map this shared memory into the process' address space. We do that by the following.
```
struct table* sharedTable = (struct table*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memory, 0);
if (sharedTable == MAP_FAILED){...}
```
We first create the sharedTable which is now a pointer to the table stored in sharedMemory.hpp. This sharedTable is acting like a reference to to the table in the shared memory.
Finally, before we get into the semaphores, we need to edit change the starting variables of the in and out for the table. We do this only in the producer.cpp file with the following.
```
sharedTable->in = 0;
sharedTable->out = 0;
```
This will set both the in and out variables of the shared memory to zero. This will make sure that the first item produced will be put into the appropriate place as well as the first item consumed.

## Semaphores and Critical Section
(increment/decrement, locking/unlocking, difference between producer + consumer)
Semaphores are a way to get processes to synchronize control amongst each other. They are especially useful when using and changing shared memory since one process waits for the other process to finish its execution before they start theirs. This helps processes not interfere with each other and ensures data consistency.

We use semaphores both in the producer.cpp and consumer.cpp files. The following code is included in both the producer and consumer as it initializes and opens up the semaphores.
```
sem_t *full = sem_open("full", O_CREAT, 0700, 0);
sem_t *empty = sem_open("empty", O_CREAT, 0700, bufferSize); 
sem_t *mutex = sem_open("mutex", O_CREAT, 0700, 1);
```
We create three different semaphores and open them up, named full, empty, and mutex. The O_CREAT will open the semaphore or create it if it has not been already. The 0700 sets the permissions for the semaphore. The full semaphore is used to show whether the data has an item in there. This is useful for the consumer as will be shown later. It is initialized to 0, meaning that it has no items in the data currently. The empty semaphore is used to show whether there is any empty spots in the data. This is useful for the producer as will be shown later. It is initialized to 2, meaning that there are 2 empty spots in the data currently. The mutex semaphore is used to show whether or not an item is in the critical section. Only one item is allowed in the critical section at a time so it is initialized to 1.

For the producer and consumer, we enter a while loop. This while loop will only run until all items are produced for the producer and until all items are consumed for the consumer. The amount of items produced is set in the shared memory before the program executes.
`while(produced < totalProduction){...}` or `while(consumed < totalProduction)`

### Producer Critical Section
Inside the producer while loop, we have to wait until there is an empty space available. We do this by calling `sem_wait(empty);`, which will check the value of empty, and if it is above 0, then it will be true and decrement the value down by one. If it is 0, then it will wait until the value increases above 0. After it confirms the empty space, it will check to see if there is a process in the critical section using `sem_wait(mutex);`. If the value of mutex is greater than 0 then it will return true, decrement the value (showing that there is now a value in the critical section), and will proceed onto the critical section. However, if the value were 0, then it would wait until the value is changed to something greater than 0.

Now in the critical section, the producer waits a random amount of time. It then randomly generates an integar value by `randomValue = rand() % 100;`. Now we need a way to store this value into the shared memory so that the consumer can access it. This is done by the following.
```
sharedTable->data[sharedTable->in] = randomValue;
```
It gets the in variable from the shared memory as an index of the data array that will be changed to the random value. Esentially, this will produce a random value and insert it into the correct place in shared memory so that it is able to be consumed by the consumer.



In this critical section, it will also increment produced, output the produced value and spot, and then changes the in variable in the shared memory to the next place. This is done by `sharedTable->in = (sharedTable->in+1)%bufferSize;`. This will increment it to the next available slot, which will either be 0 or 1.

Once it is done with all of this, it will signal the mutex and full semaphores with `sem_post(mutex);` and `sem_post(full);`, respectfully. This will cause the mutex and full semaphores to both increment by one, meaning that they are both available for the consumer at this time.
This while loop will continue until the limit of produced items has been reached.

### Consumer Critical Section
Inside the consumer while loop, we have to wait until there is an item available. We do this by calling `sem_wait(full);`, which will check the value of full and will return true if it is above 0, and false if it is below 0. If it is true, then it will proceed and decrement the value of the full semaphore by one. Then it checks to see if any process is in the critical section by `sem_wait(mutex);`, and if it is true then it will decrement the value, showing that there is now a process in here.

Now in the critical section, the consumer waits a random amount of time just like the producer. It will then output the item consumed, it's spot, and increment the consumed variable. It gets the item consumed by `sharedTable->data[sharedTable->out]`. This uses the out variable to get the index, which is used to get the next value in data that will be consumed. Once it consumed this value, it will switch the out variable to the next available spot, either 0 or 1, with `sharedTable->out = (sharedTable->out+1)%bufferSize;`.

Once it is done with all of this, it will do the same as what the producer did with signaling the mutex semaphore, but also the empty sempahore instead of the full semaphore. It will increment both of these semaphores, making them available for the producer at this time.
The while loop will continue until the limit of consumed items has been reached.

## Wrapping up

Once the while loop ends for both the producer and consumer proccess, we close the semaphores and unlink them by the following.
```
sem_close(full);
sem_close(empty);
sem_close(mutex);
sem_unlink("full");
sem_unlink("empty");
sem_unlink("mutex");
```
We also close the shared memory object and unmap the address space used by the following
```
munmap(sharedTable,SIZE);
close(memory);
shm_unlink(NAME);
```

# Conclusion
The producer and consumer problem can be solved with threads and semaphores. Each producer and consumer has its own thread, and the producer produces items to the table that the consumer consumes. This is done by semaphores, which makes sure that only one process is inside the critical section at a time. This ensures data consistency and makes sure that a variable can not be changed at the same time by a different process or thread.
