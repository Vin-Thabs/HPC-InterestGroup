#include <mpi.h>
#include <stdio.h>
#include <string.h>

#define MSG_SZ 100

int main(int argc, char *argv[]) {
    int nproces, myrank, prev, next;
    char token[MSG_SZ];
    int tag = 0;  // You may define a tag for the message

    // Initialize MPI environment
    MPI_Init(&argc, &argv);
    
    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &nproces);
    
    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    // Determine the previous and next processes in the ring
    if (myrank == 0) {
        prev = nproces - 1;
    } else {
        prev = myrank - 1;
    }

    if (myrank == (nproces - 1)) {
        next = 0;
    } else {
        next = myrank + 1;
    }

    // The process with rank 0 starts the token passing
    if (myrank == 0) {
        strcpy(token, "Hello!");
        MPI_Send(token, MSG_SZ, MPI_CHAR, next, tag, MPI_COMM_WORLD);
        MPI_Recv(token, MSG_SZ, MPI_CHAR, prev, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d received token %s from process %d.\n", myrank, token, prev);
    } else {
        MPI_Recv(token, MSG_SZ, MPI_CHAR, prev, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(token, MSG_SZ, MPI_CHAR, next, tag, MPI_COMM_WORLD);
        printf("Process %d received token %s from process %d.\n", myrank, token, prev);
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}
