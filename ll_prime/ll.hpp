// DO NOT MODIFY THE FOLLOWING SEQUENTIAL FUNCTION
int seq_count_prime(struct Node *head){
  struct Node *currNode = head;
  int count = 0;
  // traverse the linked list:
  // process data
  while (currNode){
    if (isPrime(currNode->data))
      count = count+1;
    currNode = currNode->next;
  }
  return count;
}

// TODO: COMPLETE THE PARALLEL PROCESSING FUNCTION BASED ON
// THE SERIAL PROCESSING GIVEN IN FUNCTION seq_count_prime
int par_count_prime(struct Node *head){
  struct Node *currNode = head;
  int count = 0;
  
  #pragma omp single
  {
    #pragma omp task shared(count)
    {
        while (currNode){
        if (isPrime(currNode->data))
          #pragma omp atomic
          count = count+1;
        currNode = currNode->next;
      }
    }
  }
  return count; 
}
