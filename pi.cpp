#include <stdio.h>
#include <omp.h>

//this program uses the Single Program Multiple Data program(SPMD), each thread redundantly executes the same code.
//to avoid false sharing we will use synchonization

static long num_steps = 100000;
double step;

#define NUM_THREADS 2

int main() {
    double pi = 0.0;
    step = 1.0 / (double)num_steps;

    omp_set_num_threads(NUM_THREADS);
    
    #pragma omp parallel
    {
        int i, id, tthreads;
        double x,sum;

        tthreads = omp_get_num_threads();
        id = omp_get_thread_num();

        for (i = id,sum = 0.0; i < num_steps; i += tthreads) {
            x = (i + 0.5) * step;
            sum += 4.0 / (1.0 + x * x);
        }

        //the reason for not putting the sum+=... inside the the critical its because it will serialize the program , 
        //so the computation time might take longer than a serial program 
        //becareful where you place the critical construct

        #pragma omp critical
        {
             pi += sum * step;
        }
    }

    printf("PI: %lf\n", pi);
    return 0;
}
