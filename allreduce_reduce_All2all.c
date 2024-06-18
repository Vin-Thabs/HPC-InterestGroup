#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int rank, size;
    
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
    
    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    // Allocate memory for the send and receive buffers
    int *sendbuf = (int *)malloc(size * sizeof(int));
    int *recvbuf = (int *)malloc(size * sizeof(int));
    
    // Initialize the send buffer
    for (int i = 0; i < size; i++) {
        sendbuf[i] = rank * size + i; // Example values
    }
    
    // Perform Alltoall communication
    MPI_Alltoall(sendbuf, 1, MPI_INT, recvbuf, 1, MPI_INT, MPI_COMM_WORLD);
    
    // Print the received data from Alltoall
    printf("Process %d received:", rank);
    for (int i = 0; i < size; i++) {
        printf(" %d", recvbuf[i]);
    }
    printf("\n");
    
    // Perform Reduce operation
    int local_sum = 0;
    for (int i = 0; i < size; i++) {
        local_sum += sendbuf[i];
    }
    
    int global_sum;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("Global sum: %d\n", global_sum);
    }
    
    // Perform Allreduce operation
    int allreduce_sum;
    MPI_Allreduce(&local_sum, &allreduce_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    
    printf("Process %d sees the global sum as: %d\n", rank, allreduce_sum);
    
    // Clean up
    free(sendbuf);
    free(recvbuf);
    
    // Finalize the MPI environment
    MPI_Finalize();
    
    return 0;
}
