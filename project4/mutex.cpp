#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <omp.h>
#include <time.h>

using std::cerr;
using std::endl;
using std::cout;

const int NUM_THREADS = 3;
int main() {
    omp_set_num_threads(NUM_THREADS);
    
}