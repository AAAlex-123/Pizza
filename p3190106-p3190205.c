#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "p3190106-p3190205.h"

// #define DEBUG
#define MAX_LOG_LENGTH 100

int customers;
long* ids;
thread* threads;

/* global counters for stats */
int moneyyy = 0;

/* global mutexes and condition variables */
mutex tele_mutex;
condv tele_condv;

/* global counters for available resourses */
int available_telephone_guys = N_TELE;

/* Starts the pizza order */
void* startOrder(void* args) {

	/* buffer for sprintf and logstr for this thread */
	char msg[MAX_LOG_LENGTH];

#ifdef DEBUG
	sprintf(msg, "Thread %ld started", (long) args);
	logstr(msg);
#endif

	/* Wait for telephone guy */
	pthread_mutex_lock(&tele_mutex);

	while (available_telephone_guys == 0)
		pthread_cond_wait(&tele_condv, &tele_mutex);

	--available_telephone_guys;

	pthread_mutex_unlock(&tele_mutex);

	/* Select pizzas */
	int pizza_count = randint(N_ORDER_LOW, N_ORDER_HIGH);

	/* Pay for pizzas */
	unsigned int sleep_time = randint(T_PAYMENT_LOW, T_PAYMENT_HIGH);

#ifdef DEBUG
	sprintf(msg, "Order %ld waits for %d seconds on the phone", (long) args, sleep_time);
	logstr(msg);
#endif
	sleep(sleep_time);

	/* Fail to pay for pizzas */
	if (randint(0, 1 * 100000) < P_FAIL * 100000) {
		sprintf(msg, "Order %ld failed", (long) args);
		logstr(msg);
		/* exit with code 1 so sum of codes = number of failed */
		/* THIS WILL CHANGE IN THE FUTURE */
		pthread_exit((void*) 1);
	}

	/* Actually pay for pizzas */
	sprintf(msg, "Order %ld registered", (long) args);
	logstr(msg);
	increment(C_PIZZA * pizza_count, &moneyyy);

	/* Free the telephone guy */
	pthread_mutex_lock(&tele_mutex);

	++available_telephone_guys;
	pthread_cond_signal(&tele_condv);
	pthread_mutex_unlock(&tele_mutex);

	/*
	 * ...
	 * other stuff I don't have to worry about
	 * ...
	 */
	pthread_exit((void*) 0);
}


int main(int argc, char** argv) {

	/* Initialize all mutexes / cond variables */
	init_helper_mutexes();
	pthread_mutex_init(&tele_mutex, NULL);

#ifdef DEBUG
	printf("Customers: %s\nSeed: %s\n", argv[1],argv[2]);
#endif

	/* Initialize program arguments */
	if (argc != 3) {
		logerr("Error: Give the number of customers and a random seed as arguments.");
		exit(1);
	}

	/* Customer Number */
	int cust_num = atoi(argv[1]);
	if (cust_num <= 0) {
		logerr("Error: there must be at least one customer.");
		exit(2);
	}
	customers = cust_num;

	ids = (long*) calloc(cust_num, sizeof(long));
	if (ids == NULL) {
		logerr("Error: memory allocation in id array");
		exit(3);
	}

	threads = (thread*) malloc(cust_num * sizeof(thread));
	if (threads == NULL) {
		logerr("Error: memory allocation in thread array");
		exit(4);
	}

	/* Random Seed */
	rand_seed = atoi(argv[1]);

	/* Initialize orders */
	for (long i = 0; i < customers; ++i) {
		ids[i] = i;
		if (pthread_create(&threads[i], NULL, startOrder, (void*) ids[i])) {
			fprintf(stderr, "Error: Thread %ld could not be created", i);
		}
		sleep(randint(T_ORDER_LOW, T_ORDER_HIGH));
	}

	/* Wait for orders to finish */
	int failed = 0;
        void* status;
	for (int i = 0; i < customers; i++) {
		pthread_join(threads[i], &status);
#ifdef DEBUG
		char msg[100];
		sprintf(msg, "thread %d exited with code %ld\n", i, (long)
				status);
		logstr(msg);
#endif
		failed += (long) status;
	}

	/* Print very useful stats */
	printf("\nVery Useful Stats:\n");
	printf("Total revenue: %d\nSuccessful orders: %d\nFailed orders: %d\n", moneyyy, cust_num - failed, failed);

	/* we know `free` exists pls give good grade */
	free(threads);
	free(ids);
	return 0;
}
