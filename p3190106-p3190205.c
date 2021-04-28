#include "p3190106-p3190205.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define DEBUG

typedef pthread_mutex_t mutex;
typedef pthread_t thread;

mutex out_lock;
unsigned int rand_seed = 869;
int customers;
int* ids;
thread* threads;

/*Prints a string in a thread-safe way to stdout*/
void logstr(char* string);

/*Prints a string in a thread-safe way to stderr*/
void logerr(char* string);

/*Returns a random integer between start and end (inclusive)*/
int randint(int start, int end);

/*Starts the pizza order*/
void* startOrder(void* args);

int main(int argc, char** argv) {

#ifdef DEBUG
	printf("Customers: %s\nSeed: %s\n", argv[1],argv[2]);
#endif

	/*Initialize program arguments*/
	if(argc != 3){
		fprintf(stderr,"Error: Give the number of customers and a random seed as arguments.");
		exit(-1);
	}

	int cust_num = atoi(argv[1]);
	if(cust_num <= 0){
		fprintf(stderr,"Error: Invalid number of customers.");
		exit(-1);
	}
	customers =	cust_num;

	ids = (int*) calloc(cust_num,sizeof(int));
	if(ids == NULL){
		fprintf(stderr, "Error: memory allocation in id array");
		exit(-1);
	}

	threads = (thread*) malloc(cust_num*sizeof(thread));
	if(threads == NULL){
		fprintf(stderr, "Error: memory allocation in thread array");
		exit(-1);
	}

	rand_seed = atoi(argv[1]);


	/*Initialize all mutexes / cond variables*/
	pthread_mutex_init(&out_lock,NULL);
	//...

	/*Initialize threads*/
	for(int i=0; i<= customers; i++){
		ids[i] = i;
		if(pthread_create(threads[i],NULL,startOrder,ids[i]) == 0){
			logerr("Error: Thread could not be created");
		}
	}

	/*Wait for threads to finish*/
	for(int i=0; i<= customers; i++){
		pthread_join(threads[i],NULL);
	}

	free(threads);
	free(ids);
	return 0;

}

/*Helper functions*/

void logstr(char* string) {
	pthread_mutex_lock(&out_lock);
	printf("%s",string);
	pthread_mutex_unlock(&out_lock);
}

void logerr(char* string) { /*Printing both in stdout or stderr won't cause a crash but might print messages in the same line*/
	pthread_mutex_lock(&out_lock);
	fprintf(stderr, "%s",string);
	pthread_mutex_unlock(&out_lock);
}

int randint(int start, int end){
	#ifdef DEBUG
	if(rand_seed == 869)
		logerr("Warning: no seed initialized.");
	#endif

	return rand_r(&rand_seed) % (end-start+1) + start;
}

void * startOrder(void * args){
	logstr("Thread executed");
	return NULL;
}
