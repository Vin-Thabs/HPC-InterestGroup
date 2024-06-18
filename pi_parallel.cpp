#include <stdio.h>
#include <omp.h>

//this program uses the SPMD process
//there is false sharing on this code
static long num_steps = 100000;
double step;

#define NUM_THREADS 2

int main() {
    int i, nthreads;
    double pi, sum[NUM_THREADS], Runtime, start_time;

    step = 1.0 / (double)num_steps;

    omp_set_num_threads(NUM_THREADS);
    
    #pragma omp parallel
    {
        start_time = omp_get_wtime();
        int i, id, tthreads;
        double x;

        tthreads = omp_get_num_threads();
        id = omp_get_thread_num();

        if (id == 0) nthreads = tthreads;//declare the nthreads value to be able to use it outside the parallel region
        for (i = id, sum[id] = 0.0; i < num_steps; i = i + tthreads) {
            x = (i + 0.5) * step;
            sum[id] += 4.0 / (1.0 + x * x);
        }
       Runtime += omp_get_wtime() -start_time;
    }

    for (i = 0, pi = 0.0; i < nthreads; i++)
        pi += step * sum[i];

    printf("PI: %lf\n", pi);
    printf("runtime %f\n", Runtime);
    return 0;
}
