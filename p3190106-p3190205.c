#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "p3190106-p3190205.h"

#define DEBUG

int customers;
long* ids;
thread* threads;
int moneyyy = 0;

condv tele_condv;
mutex tele_mutex;
int available_telephone_guys = N_TELE;

/* Initializes stuff of the .h file*/
void init_helper_vars();

/*Prints a string in a thread-safe way to stdout*/
void logstr(char* string);

/*Prints a string in a thread-safe way to stderr*/
void logerr(char* string);

/*Returns a random integer between start and end (inclusive)*/
int randint(int start, int end);

/* increases sum by amt  s a f e l y */
void pay(int amt, int* sum);

/*Starts the pizza order*/
void* startOrder(void* args);


void* startOrder(void* args) {
	// buffer for sprintf and logstr
	char msg[100];

	sprintf(msg, "Thread %ld started", (long) args);
	logstr(msg);

	/* Wait for telephone guy */
	while (available_telephone_guys == 0)
		pthread_cond_wait(&tele_condv, &tele_mutex);

	--available_telephone_guys;
	
	/* Order pizzas */
	int pizza_count = randint(N_ORDER_LOW, N_ORDER_HIGH);

	/* Pay for pizzas */
	unsigned int s = randint(T_PAYMENT_LOW, T_PAYMENT_HIGH);
	sprintf(msg, "thread %ld will sleep for %d\n", (long) args, s);
	logstr(msg);
	sleep(s);	

	/* Fail to pay for pizzas*/
	if (randint(0, 1 * 100000) < P_FAIL * 100000) {
		sprintf(msg, "Order %ld failed", (long) args);
		logstr(msg);
		// exit with code 1 so sum of codes = number of failed
		pthread_exit((void*) 1);
	}		
	sprintf(msg, "Order %ld kataxwrh8hke lmao", (long) args);
	logstr(msg);

	/* Actually pay for pizzas */
	pay(C_PIZZA * pizza_count, &moneyyy);

	/* Free the telephone guy */
	pthread_mutex_unlock(&tele_mutex);
	pthread_cond_signal(&tele_condv);
	++available_telephone_guys;

	/* ... */
	pthread_exit((void*) 0);
}


int main(int argc, char** argv) {

#ifdef DEBUG
	printf("Customers: %s\nSeed: %s\n", argv[1],argv[2]);
#endif

	/*Initialize program arguments*/
	if(argc != 3){
		fprintf(stderr,"Error: Give the number of customers and a random seed as arguments.");
		exit(-1);
	}

	/* Customer Number */
	int cust_num = atoi(argv[1]);
	if (cust_num <= 0) {
		logerr("Error: Invalid number of customers.");
		exit(-1);
	}
	customers = cust_num;

	ids = (long*) calloc(cust_num, sizeof(long));
	if (ids == NULL) {
		logerr("Error: memory allocation in id array");
		exit(-1);
	}

	threads = (thread*) malloc(cust_num * sizeof(thread));
	if (threads == NULL) {
		logerr("Error: memory allocation in thread array");
		exit(-1);
	}

	/* Random Seed */
	rand_seed = atoi(argv[1]);

	/*Initialize all mutexes / cond variables*/
	init_helper_vars();
	pthread_mutex_init(&tele_mutex, NULL);

	/*Initialize threads*/
	for (long i = 0; i < customers; i++) {
		ids[i] = i;
		if (pthread_create(&threads[i], NULL, startOrder, (void*) ids[i])) {
			logerr("Error: Thread could not be created");
		}
		sleep(randint(T_ORDER_LOW, T_ORDER_HIGH));
	}

	/*Wait for threads to finish*/
	int failed = 0;
        void* status;
	for (int i = 0; i < customers; i++) {
		pthread_join(threads[i], &status);
		char msg[100];
		sprintf(msg, "thread %d exited with code %ld\n", i, (long)
				status);
		logstr(msg);
		failed += (long) status;
	}

	/* Print useful stats */
	printf("money: %d, succ: %d, fail: %d\n", moneyyy, cust_num - failed, failed);

	/* flex */
	free(threads);
	free(ids);
	return 0;
}

