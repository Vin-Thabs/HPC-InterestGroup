#include<stdio.h>
#include<omp.h>


//parallelized using reduction construct and critical
int main()
{
	static long num_steps = 1000000;
	double steps;
	int i;
	double  pi, sum = 0.0, run_time, start_time;
	steps = 1.0/(double) num_steps;

	start_time = omp_get_wtime();
	#pragma omp parallel
	{
		double x;
		#pragma omp for reduction(+:sum)//operator is"+" on the sum variable
			for(i = 0; i < num_steps; i++)
			{
				x = (i+0.5)*steps;
				sum = sum + 4.0/(1.0 + x*x);
			}

		#pragma omp crtical
		{
			pi = steps*sum;
		}
	}

	run_time = omp_get_wtime() - start_time;
	
	printf("value of PI %f\n", pi);
	printf("runtime parallel is : %f\n",run_time );
}
