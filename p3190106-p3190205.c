#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "p3190106-p3190205.h"

/* general global vars*/
int customers;
pizza_info* pizza_infos;
thread* threads;

/*stats*/
int moneyyy = 0;
int successful = 0;
int failed = 0;
int total_wait = 0;
int max_wait = 0;
int total_delivery = 0;
int max_delivery = 0;
int total_cooling = 0;
int max_cooling = 0;

/* global mutexes and condition variables */
mutex out_lock;
mutex increment_lock;
mutex max_lock;

mutex tele_mutex;
mutex cook_mutex;
mutex oven_mutex;
mutex package_mutex;
mutex delivery_mutex;

condv cook_condv;
condv tele_condv;
condv oven_condv;
condv delivery_condv;

/* global counters for available resources */
int available_telephone_guys = N_TELE;
int available_cooks = N_COOK;
int available_ovens = N_OVEN;
int available_delivery_guys = N_DELIVERER;

/*===================== Main Functions =====================*/

/*
 * Function called by all threads to make an order, successively calls all steps
 * for the pizza delivery.
 */
void* makeOrder(void* args) {
	pizza_info* p_info = (pizza_info*) args;

	if (order_pizzas(p_info) == 1)
		pthread_exit((void*)1);
	// prepare_pizzas(p_info);
	// cook_pizzas(p_info);
	// package_pizzas(p_info);
	deliver_pizzas(p_info);

	pthread_exit(NULL);
}

int main(int argc, char** argv) {

	/* Initialize all mutexes / cond variables */
	pthread_mutex_init(&out_lock, NULL);
	pthread_mutex_init(&increment_lock, NULL);
	pthread_mutex_init(&max_lock, NULL);
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

	pizza_infos = (pizza_info*) malloc(cust_num*sizeof(pizza_info));
	if (pizza_infos == NULL) {
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
		thread_info.order_start_time = time(NULL);
		pizza_infos[i] = thread_info;

		if (pthread_create(&threads[i], NULL, makeOrder, (void*) &pizza_infos[i])) {
			fprintf(stderr, "Error: Thread %ld could not be created", i);
		}

		sleep(randint(T_ORDER_LOW, T_ORDER_HIGH));
	}
#ifdef DEBUG
	printf("done creating\n");
	fflush(stdout);
#endif

	/* Wait for orders to finish */
	int failed = 0;
        void* status;
	for (int i = 0; i < customers; i++) {
		pthread_join(threads[i], &status);
#ifdef DEBUG
		printf("Thread %d joined\n", i);
		fflush(stdout);
#endif
		failed += (long) status;
	}

	/* Print very useful stats */
	int successful = cust_num - failed;
	printf("\nVery Useful Stats:\n");
	printf("Total revenue: %d\nSuccessful orders: %d\nFailed orders: %d\n", moneyyy, successful, failed);
	printf("Average wait time: %.3f\nMax wait time: %d\n", (float) total_wait / cust_num, max_wait);
	printf("Average delivery time: %.3f\nMax delivery time: %d\n", (float) total_delivery / successful, max_delivery);
	printf("Average cooling time: %.3f\nMax cooling time: %d\n", (float) total_cooling / successful, max_cooling);

	/* we know `free` and `destroy` exist pls give good grade */
	free(threads);
	free(pizza_infos);

	pthread_mutex_destroy(&out_lock);
	pthread_mutex_destroy(&increment_lock);
	pthread_mutex_destroy(&max_lock);
	pthread_mutex_destroy(&tele_mutex);
	pthread_mutex_destroy(&cook_mutex);
	pthread_mutex_destroy(&oven_mutex);
	pthread_mutex_destroy(&package_mutex);
	pthread_mutex_destroy(&delivery_mutex);

	pthread_cond_destroy(&tele_condv);
	pthread_cond_destroy(&cook_condv);
	pthread_cond_destroy(&oven_condv);
	pthread_cond_destroy(&delivery_condv);

	return 0;
}

int order_pizzas(pizza_info* p_info) {

	/* buffer for sprintf and logstr for this thread */
	char msg[MAX_LOG_LENGTH]; 

#ifdef DEBUG
	sprintf(msg, "Thread %ld started", p_info->threadID);
	logstr(msg);
#endif

	/* Wait for telephone guy */
	pthread_mutex_lock(&tele_mutex);

	while (available_telephone_guys == 0)
		pthread_cond_wait(&tele_condv, &tele_mutex);
	
	--available_telephone_guys;

	pthread_mutex_unlock(&tele_mutex);

	/* Talk with telephone guy */
	int wait = time_elapsed(p_info->order_start_time);
	increment(wait, &total_wait);
	max(wait, &max_wait);

	/* Select pizzas */
	p_info->num_of_pizzas = randint(N_ORDER_LOW, N_ORDER_HIGH);

	/* Pay for pizzas */
	unsigned int sleep_time = randint(T_PAYMENT_LOW, T_PAYMENT_HIGH);

#ifdef DEBUG
	sprintf(msg, "Thread %ld waits for %d seconds on the phone", p_info->threadID, sleep_time);
	logstr(msg);
#endif
	sleep(sleep_time);

	int failed = randint(0, 1 * 100000) < P_FAIL * 100000;

	if (failed) {
		
		/* Fail to pay for pizzas */
		sprintf(msg, "Order %ld failed", p_info->threadID);
		logstr(msg);
	} else {
		
		/* Actually pay for pizzas */
		sprintf(msg, "Order %ld registered", p_info->threadID);
		logstr(msg);
		increment(p_info->num_of_pizzas * C_PIZZA, &moneyyy);
	}

	/* Always free the telephone guy */
	pthread_mutex_lock(&tele_mutex);

	++available_telephone_guys;
	pthread_cond_signal(&tele_condv);
	pthread_mutex_unlock(&tele_mutex);
	return failed ? 1 : 0;
}

void prepare_pizzas(pizza_info* p_info) {
	pthread_mutex_lock(&cook_mutex);

#ifdef DEBUG
	char msg [MAX_LOG_LENGTH];
	sprintf(msg, "Thread %ld is waiting %d cooks to prepare %d pizzas", p_info->threadID, available_cooks, p_info->num_of_pizzas);
	logstr(msg);
#endif

	while(available_cooks == 0)
		pthread_cond_wait(&cook_condv, &cook_mutex);

	available_cooks--;
	sleep(p_info->num_of_pizzas*T_PREP);
	available_cooks++;

	pthread_mutex_unlock(&cook_mutex);
	pthread_cond_signal(&cook_condv);
}

void cook_pizzas(pizza_info* p_info) {

	pthread_mutex_lock(&oven_mutex);
	while(p_info->num_of_pizzas > available_ovens)
		pthread_cond_wait(&oven_condv, &oven_mutex);

	available_ovens -= p_info->num_of_pizzas;

#ifdef DEBUG
	char msg[MAX_LOG_LENGTH];
	sprintf(msg, "Thread %ld started cooking %d pizzas, %d ovens currently available",p_info->threadID,p_info->num_of_pizzas,available_ovens);
	logstr(msg);
#endif

	sleep(T_BAKE);

	// I DONT KNOW IF THIS IS RIGHT PLACE TO PUT IT
	p_info->order_baked_time = time(NULL);

	available_ovens += p_info->num_of_pizzas;

	pthread_mutex_unlock(&oven_mutex);
	pthread_cond_signal(&oven_condv);
}

void package_pizzas(pizza_info* p_info) {
	char msg[MAX_LOG_LENGTH];

	pthread_mutex_lock(&package_mutex);
	sleep(T_PACK * p_info->num_of_pizzas);
	pthread_mutex_unlock(&package_mutex);

	sprintf(msg, "Order %ld prepared in %ld seconds", p_info->threadID, time_elapsed(p_info->order_start_time));
	logstr(msg);
}

void deliver_pizzas(pizza_info* p_info) {
	char msg[MAX_LOG_LENGTH];

#ifdef DEBUG
	sprintf(msg, "Thread %ld waiting to be delivered", p_info->threadID);
	logstr(msg);
#endif
	/* Wait for delivery guy */
	pthread_mutex_lock(&delivery_mutex);
	while (available_delivery_guys == 0)
		pthread_cond_wait(&delivery_condv, &delivery_mutex);

	--available_delivery_guys;
	pthread_mutex_unlock(&delivery_mutex);

	/* Deliver pizzas */
	int delivery_time = randint(T_DEL_LOW, T_DEL_HIGH);
	sleep(delivery_time);

	sprintf(msg, "Order %ld delivered in %ld seconds", p_info->threadID, time_elapsed(p_info->order_start_time));
	logstr(msg);

	/* Update global variables */
	int cooling_time = time_elapsed(p_info->order_baked_time);

	increment(delivery_time, &total_delivery);
	max(delivery_time, &max_delivery);
	increment(cooling_time, &total_cooling);
	max(cooling_time, &max_cooling);

	/* Return to pizza shop */
	sleep(delivery_time);

	/* Free delivery guy */
	pthread_mutex_lock(&delivery_mutex);

	++available_delivery_guys;
	pthread_cond_signal(&delivery_condv);
	pthread_mutex_unlock(&delivery_mutex);
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

void max(int val, int* max) {
	pthread_mutex_lock(&max_lock);
	*max = (val > *max) ? val : *max;
	pthread_mutex_unlock(&max_lock);
}

time_t time_elapsed(time_t start_time) {
	return (time(NULL) - start_time);
}
