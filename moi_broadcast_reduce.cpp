#include "mpi.h"
#include <stdio.h>

int main(int argc, char **argv) {
    int n, numprocs, myrank, myval, sum;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    if (myrank == 0) {
        printf("Enter the number to be broadcasted (0 quits): ");
        scanf("%d", &n);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (n == 0) {
        MPI_Finalize();
        return 0;
    }

    myval = n * 2;
    MPI_Reduce(&myval, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (myrank == 0) {
        printf("Sum = %d\n", sum);
    }

    MPI_Finalize();
    return 0;
}
