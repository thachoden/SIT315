#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>


using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size)
{
    for (int i = 0; i < size; i++)
    {
        // Fill each element with a pseudo-random integer between 0 and 99
        vector[i] = rand() % 100;
    }
}


int main(){

    unsigned long size = 100000000;

    srand(time(0));

    int *v1, *v2, *v3;

    // Record the start time (high-resolution clock) so the execution time can be measured
    auto start = high_resolution_clock::now();

    // Dynamically allocate memory on the heap for the two input vectors and the result vector
    v1 = (int *) malloc(size * sizeof(int *));
    v2 = (int *) malloc(size * sizeof(int *));
    v3 = (int *) malloc(size * sizeof(int *));


    randomVector(v1, size);

    randomVector(v2, size);


    // Add the two vectors element by element and store the result in v3 (sequential loop)
    for (int i = 0; i < size; i++)
    {
        v3[i] = v1[i] + v2[i];
    }

    auto stop = high_resolution_clock::now();

    // Calculate the elapsed time (stop - start) and convert it to microseconds
    auto duration = duration_cast<microseconds>(stop - start);


    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    return 0;
}