/* File:     sieve_mpi.c
 * Purpose:  Use MPI to implement a parallel version of the
 *           sieve of Eratosthens algorithm
 *
 * Input:
 * Output:   The total number of primes in the range 2 to n
 *
 * Compile:  mpicc -g -Wall -o sieve_mpi sieve_mpi.c -lm
 * Run:      mpiexec -n <number of processes> ./sieve_mpi <n>
 *           such as mpiexec -n 4 ./sieve_mpi 1000000
 *
 *Algorithm: (See Lec7 slides)
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
	int count;
	double elapsed_time;
	int first;
	int global_count;
	int high_value, low_value;
	int i, id, index, n, p;
	char *marked;
	int k, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Barrier(MPI_COMM_WORLD);

	if(argc != 2){
		if(!id) printf("Command line: %s <n>\n", argv[0]);
		MPI_Finalize();
		exit(1);
	}

	n = atoi(argv[1]);
	elapsed_time = -MPI_Wtime();
	low_value = 2 + id * (n-1)/p;
	if((n-1)%p && id==(p-1))
		high_value = n;
	else
		high_value = 2 + ((id + 1) * (n-1)/p - 1);
	size = high_value - low_value + 1;

	if((2 + size) < (int) sqrt( (double)n)){
		if(!id) printf("Too many processes");
		MPI_Finalize();
		exit(1);
	}

	marked = (char *)malloc(size);
	if(marked == NULL){
		printf("Cannot allocate enough memory\n");
		MPI_Finalize();
		exit(1);
	}
	// initialize the array elements as unmarked
	for(i = 0; i < size; i++)
		marked[i] = 0;

	if(!id)
		// only process 0 has this value
		index = 0;
	// starts from 2
	k = 2;
	do{
		// determine where to start marking for my portion
		if(k * k > low_value)
			first = k*k - low_value;
		else{
			if(!(low_value % k))
				first = 0;
			else
				first = k - (low_value % k);
		}
		// mark multiples of k in my portion
		for(i = first; i < size; i+=k)
			marked[i]=1;
		// find the next smallest unmarked number
		// in process 0's portion
		if(!id){
			while(marked[++index]);
			k = index + 2;
		}
		// broadcast the next k
		MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}while(k * k <= n);
	count = 0;
	// count the primes in my portion
	for(i=0; i<size; i++)
		if(!marked[i]) count++;
	// do a global reduction on the local counts
	MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	elapsed_time += MPI_Wtime();
	// process 0 to print the results to stdout
	if(!id){
		printf("%d primes are less than or equal to %d\n", global_count, n);
		printf("Total elapsed time: %10.6f\n", elapsed_time);
	}

	MPI_Finalize();
	return 0;
}
