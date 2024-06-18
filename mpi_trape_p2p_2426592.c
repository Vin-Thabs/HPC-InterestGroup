#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void DisplayUsage(char prog_name[]);
double CalculateFunction(double x);
double ComputeTrapezoid(double left, double right, int num_trapezoids, double base_length);
void CollectInput(int rank, int size, double* a_p, double* b_p, int* n_p);

int main(int argc, char* argv[]) {
    int rank, size, num_trapezoids, local_trapezoids;
    double start_point, end_point, step, local_start, local_end;
    double local_result, total_result;
    double start_time, end_time;
    int source;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (rank == 0) {
        if (argc < 4) {
            DisplayUsage(argv[0]);
        } else {
            start_point = atof(argv[1]);
            end_point = atof(argv[2]);
            num_trapezoids = atoi(argv[3]);
        }
    }
    
    start_time = MPI_Wtime();
    CollectInput(rank, size, &start_point, &end_point, &num_trapezoids);
    
    step = (end_point - start_point) / num_trapezoids;
    local_trapezoids = num_trapezoids / size;
    
    local_start = start_point + rank * local_trapezoids * step;
    local_end = local_start + local_trapezoids * step;
    
    local_result = ComputeTrapezoid(local_start, local_end, local_trapezoids, step);
    
    if (rank != 0) {
        MPI_Send(&local_result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        total_result = local_result;
        for (source = 1; source < size; source++) {
            MPI_Recv(&local_result, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_result += local_result;
        }
    }
    
    end_time = MPI_Wtime();
    if (rank == 0) {
        printf("having  n = %d trapezoids... the estimate of the integral from %f to %f = %.15e in %.6fs\n", 
               num_trapezoids, start_point, end_point, total_result, end_time - start_time);
    }
    
    MPI_Finalize();
    return 0;
}

double CalculateFunction(double x) {
    return 4 / (1 + x * x);
}

double ComputeTrapezoid(double left, double right, int num_trapezoids, double base_length) {
    double estimate, x;
    int i;
    estimate = (CalculateFunction(left) + CalculateFunction(right)) / 2.0;

    for (i = 1; i <= num_trapezoids - 1; i++) {
        x = left + i * base_length;
        estimate += CalculateFunction(x);
    }
    estimate = estimate * base_length;
    return estimate;
}

void CollectInput(int rank, int size, double* a_p, double* b_p, int* n_p) {
    int dest;
    if (rank == 0) {
        for (dest = 1; dest < size; dest++) {
            MPI_Send(a_p, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
            MPI_Send(b_p, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
            MPI_Send(n_p, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(a_p, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(b_p, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(n_p, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void DisplayUsage(char prog_name[]) {
    fprintf(stderr, "Usage: %s <start point> <end point> <number of trapezoids>\n", prog_name);
    exit(0);
}
