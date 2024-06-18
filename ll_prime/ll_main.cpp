/* File: ll_main.cpp
 * Purpose: Count the prime numbers in a linked list
 * Compile: g++ -g -Wall -fopenmp ll_main.cpp -o ll_main
 * Run: ./ll_main
 *
 */
#include <iostream>
#include <omp.h>
#include <math.h>
using namespace std;


#define SIZE 300000

struct Node {
        int data;
        Node* next;
};

bool isPrime(int n){
	int limit = sqrt(n);
	for(int i=2; i<=limit; i++)
		if(n%i == 0)
			return false;
	return true;
}

int seq_count_prime(struct Node *head);
int par_count_prime(struct Node *head);

#include "ll.hpp"

int main(int argc, char *argv[]) {

    struct Node *head = new Node;
    struct Node *currNode = head;
    int seq_cnt, par_cnt;
    double s_time, p_time; 
    
    // populate the linked list with random numbers
    srand(2024);
    head->data = 0;
    for (int i=1;i<SIZE;++i) {
        currNode->next = new Node;
        currNode = currNode->next;
        currNode->data = rand();
    }
    
    // call the serial function
    s_time = -omp_get_wtime();
    seq_cnt = seq_count_prime(head);
    s_time += omp_get_wtime(); 
    
    // call the parallel function
    p_time = -omp_get_wtime();
    par_cnt = par_count_prime(head);
    p_time += omp_get_wtime();
    
    // check if the results from serial and parallel 
    // runs are the same
    if(seq_cnt != par_cnt){
      cout << "ERROR: " << seq_cnt << "!=" << par_cnt << endl;
    }
    else{
      cout << "CORRECT. " << "seq_cnt = " << seq_cnt << " par_cnt = "<< par_cnt << endl;
      cout << "Serial time: \t\t" << s_time << endl;
      cout << "Parallel time: \t\t" << p_time << endl;
      cout << "Speedup: \t\t" << s_time/p_time << endl;
    }
    return 0;
}
