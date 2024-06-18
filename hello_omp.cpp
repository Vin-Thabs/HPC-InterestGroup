#include<stdio.h>
#include<omp.h>

int main()
{
    // omp_set_num_threads(4);
    #pragma omp parallel num_threads(4)
    //start of the parallel region
    {

        //Runtime library functionto return a thread number
        int ID = omp_get_thread_num();
        printf("Hello World! (Thread %d)\n ", ID);
        printf("Hello Thabs! (Thread %d)\n ", ID);
    }
    //end the parallel region
}