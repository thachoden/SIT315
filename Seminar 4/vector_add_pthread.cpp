// SIT315 Seminar 4 - Parallel vector addition using pthreads
// Usage: ./vector_add_pthread [num_threads] [size]
// The vectors are split into num_threads equal partitions (data decomposition).
// Each thread fills its own partition of v1 and v2 with random numbers and then
// adds them into v3. No two threads touch the same elements, so no locks are needed.

#include <iostream>
#include <cstdlib>
#include <chrono>
#include <pthread.h>
#include <time.h>

using namespace std::chrono;
using namespace std;

// Shared (read-only pointers) between all threads, all thread can write only to it index range
int *v1, *v2, *v3;

// Work description for one thread: the half-open range [start, end) of indexes it owns
struct Task
{
    unsigned long start;
    unsigned long end;
    unsigned int seed;      // private seed so each thread has its own random sequence
    long long partialSum;   // checksum of this partition (used to verify the result)
};

// Thread function: generate the random input for this partition, then add it
void *worker(void *arg)
{
    Task *task = (Task *)arg;
    long long sum = 0;

    // rand_r() is thread-safe because the state (seed) is private to this thread.
    // rand() must not be used here: it shares one hidden global state between threads.
    for (unsigned long i = task->start; i < task->end; i++)
    {
        v1[i] = rand_r(&task->seed) % 100;
        v2[i] = rand_r(&task->seed) % 100;
    }

    // Add the two partitions element by element
    for (unsigned long i = task->start; i < task->end; i++)
    {
        v3[i] = v1[i] + v2[i];
        sum += v3[i];
    }

    task->partialSum = sum; //pass the sum back to task struct
    return NULL;
}

int main(int argc, char *argv[])
{
    int numThreads = (argc > 1) ? atoi(argv[1]) : 4; //if the argument is include, take that as number of thread, otherwise, 4 will be by default
    unsigned long size = (argc > 2) ? strtoul(argv[2], NULL, 10) : 100000000; //same as the thread number, if the size is include, take it, otherwise 100m is default
    if (numThreads < 1) numThreads = 1; //if number of threads less than 1, it will be forced to be at least 1.

    auto start = high_resolution_clock::now();// start the clock right before fire the first step

    // Sequential part: allocate the three vectors
    v1 = (int *)malloc(size * sizeof(int));
    v2 = (int *)malloc(size * sizeof(int));
    v3 = (int *)malloc(size * sizeof(int));

    //1 thread will handle 1 task
    pthread_t *threads = new pthread_t[numThreads];
    Task *tasks = new Task[numThreads];

    // Decomposition: split [0, size) into numThreads partitions of (almost) equal size
    unsigned long partitionSize = size / numThreads;
    unsigned int baseSeed = time(0);

    // Parallel part: create one thread per partition
    for (int t = 0; t < numThreads; t++)
    {
        tasks[t].start = t * partitionSize;
        tasks[t].end = (t == numThreads - 1) ? size : (t + 1) * partitionSize;   // last thread takes the remainder
        tasks[t].seed = baseSeed + t;
        tasks[t].partialSum = 0;
        pthread_create(&threads[t], NULL, worker, &tasks[t]);
    }

    // Sequential part: wait for every thread to finish, then combine the partial results
    long long checksum = 0;
    for (int t = 0; t < numThreads; t++)
    {
        pthread_join(threads[t], NULL);
        checksum += tasks[t].partialSum;
    }
    //end the clock exactly when we have the result, so futher calculation time will not be included
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);// calculate duration and convert to ms

    // Verification (not timed): recompute the sum of v1 + v2 sequentially and compare
    long long expected = 0;
    for (unsigned long i = 0; i < size; i++) expected += v1[i] + v2[i];

    cout << "Threads: " << numThreads
         << " | Partition size: " << partitionSize
         << " | Time taken: " << duration.count() << " microseconds"
         << " | Check: " << (checksum == expected ? "PASS" : "FAIL") << endl;

    free(v1); free(v2); free(v3);
    delete[] threads; delete[] tasks;// drop the memory to avoid leaking
    return 0;
}
