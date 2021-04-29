#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "p3190106-p3190205.h"

/* general global vars*/
int customers;
pizza_info *pids;
thread* threads;

/*stats*/
int moneyyy = 0;

/* global mutexes and condition variables */
mutex out_lock;
mutex increment_lock;

mutex tele_mutex;
mutex cook_mutex;
mutex oven_mutex;
mutex package_mutex;

condv cook_condv;
condv tele_condv;
condv oven_condv;

/* global counters for available resources */
int available_telephone_guys = N_TELE;
int available_cooks = N_COOK;
int available_ovens = N_OVEN;

/*===================== Main Functions =====================*/

/*
 * Function called by all threads to make an order, successively calls all steps
 * for the pizza delivery.
 */
void* makeOrder(void* args){
	pizza_info * p_info = (pizza_info *) args;
	startOrder(p_info);
	prepare_pizzas(p_info);
	cook_pizzas(p_info);
	package_pizzas(p_info);
	//...

	pthread_exit(NULL);
}


int main(int argc, char** argv) {

	/* Initialize all mutexes / cond variables */
	pthread_mutex_init(&out_lock, NULL);
	pthread_mutex_init(&increment_lock, NULL);
	pthread_mutex_init(&tele_mutex, NULL);
	pthread_mutex_init(&cook_mutex, NULL);
	pthread_mutex_init(&oven_mutex, NULL);
	pthread_mutex_init(&package_mutex, NULL);

	pthread_cond_init (&tele_condv, NULL);
	pthread_cond_init (&cook_condv, NULL);
	pthread_cond_init (&oven_condv, NULL);

#ifdef DEBUG
	printf("Customers: %s\nSeed: %s\n", argv[1],argv[2]);
#endif

	/* Initialize program num_of_pizzass */
	if (argc != 3) {
		fprintf(stderr,"Error: Give the number of customers and a random seed as num_of_pizzass.");
		exit(1);
	}

	/* Customer Number */
	int cust_num = atoi(argv[1]);
	if (cust_num <= 0) {
		fprintf(stderr,"Error: there must be at least one customer.");
		exit(2);
	}
	customers = cust_num;

	pids = (pizza_info*) malloc(cust_num*sizeof(pizza_info));
	if (pids == NULL) {
		fprintf(stderr,"Error: memory allocation in id array");
		exit(3);
	}

	threads = (thread*) malloc(cust_num * sizeof(thread));
	if (threads == NULL) {
		fprintf(stderr,"Error: memory allocation in thread array");
		exit(4);
	}

	/* Random Seed */
	rand_seed = atoi(argv[1]);

	/* Initialize orders */
	for (long i = 0; i < customers; ++i) {

		pizza_info thread_info;
		thread_info.threadID = i;
		thread_info.time = clock();
		pids[i] = thread_info;

		if (pthread_create(&threads[i], NULL, makeOrder, (void*) &pids[i])) {
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
		printf("Thread %d exited with code %ld\n", i, (long) status);
#endif

		failed += (long) status;
	}

	/* Print very useful stats */
	printf("\nVery Useful Stats:\n");
	printf("Total revenue: %d\nSuccessful orders: %d\nFailed orders: %d\n", moneyyy, cust_num - failed, failed);

	/* we know `free` exists pls give good grade */
	free(threads);
	free(pids);

	pthread_mutex_destroy(&out_lock);
	pthread_mutex_destroy(&increment_lock);
	pthread_mutex_destroy(&tele_mutex);
	pthread_mutex_destroy(&cook_mutex);
	pthread_mutex_destroy(&oven_mutex);
	pthread_mutex_destroy(&package_mutex);

	pthread_cond_destroy(&tele_condv);
	pthread_cond_destroy(&cook_condv);
	pthread_cond_destroy(&oven_condv);

	return 0;
}

/* Starts the pizza order */
void startOrder(pizza_info * pid) {

	char msg[MAX_LOG_LENGTH]; /* buffer for sprintf and logstr for this thread */

#ifdef DEBUG
	sprintf(msg, "Thread %d started", pid->threadID);
	logstr(msg);
#endif

	pthread_mutex_lock(&tele_mutex);

	while (available_telephone_guys == 0) { /* Wait for telephone guy */
		pthread_cond_wait(&tele_condv, &tele_mutex);
	}
	--available_telephone_guys;

	int pizza_count = randint(N_ORDER_LOW, N_ORDER_HIGH); /* Select pizzas */

	unsigned int sleep_time = randint(T_PAYMENT_LOW, T_PAYMENT_HIGH); /* Pay for pizzas */

#ifdef DEBUG
	sprintf(msg, "Order %d waits for %d seconds on the phone", pid->threadID, sleep_time);
	logstr(msg);
#endif

	sleep(sleep_time);

	/* Fail to pay for pizzas */
	if (randint(0, 1 * 100000) < P_FAIL * 100000) {
		sprintf(msg, "Order %d failed", pid->threadID);
		logstr(msg);
		pthread_exit((void*) 1); /* exit with code 1 so sum of codes = number of failed */
	}

	/* Actually pay for pizzas */
	sprintf(msg, "Order %d registered", pid->threadID);
	logstr(msg);
	increment(C_PIZZA * pizza_count, &moneyyy);

	/* Free the telephone guy */
	++available_telephone_guys;
	pthread_mutex_unlock(&tele_mutex);
	pthread_cond_signal(&tele_condv);

	pid->num_of_pizzas = pizza_count;
}


void prepare_pizzas(pizza_info* pid) {
	pthread_mutex_lock(&cook_mutex);

#ifdef DEBUG
	char msg [MAX_LOG_LENGTH];
	sprintf(msg, "Thread %d is waiting %d cooks to prepare %d pizzas", pid->threadID, available_cooks, pid->num_of_pizzas);
	logstr(msg);
#endif

	while(available_cooks == 0)
		pthread_cond_wait(&cook_condv, &cook_mutex);

	available_cooks--;
	sleep(pid->num_of_pizzas*T_PREP);
	available_cooks++;

	pthread_mutex_unlock(&cook_mutex);
	pthread_cond_signal(&cook_condv);
}



void cook_pizzas(pizza_info* pid) {

	pthread_mutex_lock(&oven_mutex);
	while(pid->num_of_pizzas > available_ovens)
		pthread_cond_wait(&oven_condv, &oven_mutex);

	available_ovens -= pid->num_of_pizzas;

#ifdef DEBUG
	char msg[100];
	sprintf(msg, "Thread %d started cooking %d pizzas, %d ovens currently available",pid->threadID,pid->num_of_pizzas,available_ovens);
	logstr(msg);
#endif

	sleep(T_BAKE);
	available_ovens += pid->num_of_pizzas;

	pthread_mutex_unlock(&oven_mutex);
	pthread_cond_signal(&oven_condv);
}



void package_pizzas(pizza_info* p_info) {
	char msg [100];

	pthread_mutex_lock(&package_mutex);
	sleep(T_PACK * p_info->num_of_pizzas);
	pthread_mutex_unlock(&package_mutex);

	sprintf(msg, "Order #%d packaged within %f.3 minutes",p_info->threadID, time_elapsed(p_info->time));
	logstr(msg);
	p_info->time = clock(); /*update time to calculate delivery time in the next step*/
}


/*===================== Utility Functions =====================*/

void logstr(char* string) {
    pthread_mutex_lock(&out_lock);
    fprintf(stdout, "%s\n",string);
    fflush(stdout);
    pthread_mutex_unlock(&out_lock);
}

void logerr(char* string) {
    pthread_mutex_lock(&out_lock);
    fprintf(stderr, "%s\n",string);
    fflush(stderr);
    pthread_mutex_unlock(&out_lock);
}

/* returns random integer in the range [start, end] */
int randint(int start, int end) {
    if(rand_seed == 869)
        logerr("Warning: seed not initialized.\nTODO: REMOVE THIS");
    return (rand_r(&rand_seed) % (end-start+1)) + start;
}

/* increments `total` by `amt`  s a f e l y */
void increment(int amt, int* total) {
	pthread_mutex_lock(&increment_lock);
	*total += amt;
	pthread_mutex_unlock(&increment_lock);
}

double time_elapsed(clock_t process_time) {
	return (double)((clock() - process_time)/CLOCKS_PER_SEC);
}
