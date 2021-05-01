#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "p3190106-p3190205.h"

/* general global vars*/
int number_of_customers;
pizza_info* pizza_infos;
thread* threads;

/* global stats that each thread alters in a safe manner */
int revenue = 0;
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
condv package_condv;
condv delivery_condv;

/* global counters for available resources */
int available_telephone_guys = N_TELE;
int available_cooks = N_COOK;
int available_ovens = N_OVEN;
int available_package_guys = 1;
int available_delivery_guys = N_DELIVERER;

/* ==================== Main Functions ==================== */

void* make_order(void* args) {

	pizza_info* p_info = (pizza_info*) args;

	// return code 1 = failed order, 0 = registered order
	if (order_pizzas(p_info) == 1)
		pthread_exit((void*) 1);
	prepare_pizzas(p_info);
	cook_pizzas(p_info);
	package_pizzas(p_info);
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
	pthread_mutex_init(&delivery_mutex, NULL);

	pthread_cond_init(&tele_condv, NULL);
	pthread_cond_init(&cook_condv, NULL);
	pthread_cond_init(&oven_condv, NULL);
	pthread_cond_init(&package_condv, NULL);
	pthread_cond_init(&delivery_condv, NULL);

	/* Initialize program */
	if (argc != 3) {
		fprintf(stderr, "Usage: ./a.out [number of customers] [random seed]");
		exit(1);
	}

	/* Numer of Customers */
	number_of_customers = atoi(argv[1]);
	if (number_of_customers <= 0) {
		fprintf(stderr, "Error: Invalid number of customers!");
		exit(2);
	}

	pizza_infos = (pizza_info*) malloc(number_of_customers * sizeof(pizza_info));
	if (pizza_infos == NULL) {
		fprintf(stderr, "Error: Memory allocation in pizza_info array failed");
		exit(3);
	}

	threads = (thread*) malloc(number_of_customers * sizeof(thread));
	if (threads == NULL) {
		fprintf(stderr, "Error: Memory allocation in thread array failed");
		exit(4);
	}

	/* Random Seed */
	rand_seed = atoi(argv[1]);
	// if (random_seed == NULL) {
		

	/* Initialize orders */
	for (long i = 0L; i < number_of_customers; ++i) {

		pizza_info new_order_info;
		new_order_info.threadID = i + 1;
		clock_gettime(CLOCK_REALTIME, &new_order_info.order_start_time);
		pizza_infos[i] = new_order_info;

		if (pthread_create(&threads[i], NULL, make_order, (void*) &pizza_infos[i])) {
			char msg[MAX_LOG_LENGTH];
			sprintf(msg, "Error: Thread %ld could not be created", i);
			logerr(msg);
		}

		/* Wait for next customer */
		sleep(randint(T_ORDER_LOW, T_ORDER_HIGH));
	}

	/* Wait for orders to finish */
	int failed_orders = 0;
        void* return_code;
	for (int i = 0; i < number_of_customers; i++) {
		pthread_join(threads[i], &return_code);
		failed_orders += (long) return_code;
	}

	/* Print stats */
	int successful_orders = number_of_customers - failed_orders;

	printf("\nStats:\n");
	printf("Total revenue:         %d\nSuccessful orders:     %d\nFailed orders:         %d\n",
			revenue, successful_orders, failed_orders);

	printf("Average wait time:     %.2f minutes\nMax wait time:         %.2f minutes\n",
			(float) total_wait / number_of_customers / 60.f, max_wait / 60.f);

	printf("Average delivery time: %.2f minutes\nMax delivery time:     %.2f minutes\n",
			(float) total_delivery / successful_orders / 60.f, max_delivery / 60.f);

	printf("Average cooling time:  %.2f minutes\nMax cooling time:      %.2f minutes\n",
			(float) total_cooling / successful_orders / 60.f, max_cooling / 60.f);

	/* Release resourses (even though program terminates right after) */
	free(pizza_infos);
	free(threads);

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
	pthread_cond_destroy(&package_condv);
	pthread_cond_destroy(&delivery_condv);

	return 0;
}

int order_pizzas(pizza_info* p_info) {

	/*
	 * Buffer for sprintf and logstr for this thread.
	 * Each function has its own buffer, if necessary.
	 */
	char msg[MAX_LOG_LENGTH]; 

	/* Wait for an available telephone guy */
	pthread_mutex_lock(&tele_mutex); /* lock the resource to access and edit it */

	while (available_telephone_guys == 0) /* if unable to aquire it, block self and unlock it */
		pthread_cond_wait(&tele_condv, &tele_mutex);
	
	--available_telephone_guys; /* update the resource */

	pthread_mutex_unlock(&tele_mutex); /* unlock the resource */

	/* Talk with telephone guy */
	int wait = time_elapsed(&p_info->order_start_time);
	/* Update global variables */
	increment(wait, &total_wait);
	max(wait, &max_wait);

	/* Select pizzas */
	p_info->num_of_pizzas = randint(N_ORDER_LOW, N_ORDER_HIGH);

	/* Pay for pizzas */
	sleep(randint(T_PAYMENT_LOW, T_PAYMENT_HIGH));

	int order_failed = randint(0, 1 * 100000) < P_FAIL * 100000;

	if (order_failed) {
		/* Fail to pay for pizzas */
		sprintf(msg, "Order %ld failed", p_info->threadID);
		logstr(msg);
	} else {
		/* Actually pay for pizzas */
		sprintf(msg, "Order %ld registered", p_info->threadID);
		logstr(msg);
		/* Update global variable */
		increment(p_info->num_of_pizzas * C_PIZZA, &revenue);
	}

	/* Always free the telephone guy */
	pthread_mutex_lock(&tele_mutex); /* lock the resource to edit it */

	++available_telephone_guys; /* update the resource */
	pthread_cond_signal(&tele_condv); /* signal the other threads that the resource is available */
	pthread_mutex_unlock(&tele_mutex); /* unlock the resource */

	/* Return code indicating success or failure of payment */
	return order_failed ? 1 : 0;
}

void prepare_pizzas(pizza_info* p_info) {

	/* Wait for an available cook */
	pthread_mutex_lock(&cook_mutex);

	while(available_cooks == 0)
		pthread_cond_wait(&cook_condv, &cook_mutex);

	available_cooks--;
	pthread_mutex_unlock(&cook_mutex);

	/* Prepare the pizzas */
	sleep(p_info->num_of_pizzas * T_PREP);

	/* The cook is freed once the pizzas are in the ovens */
}

void cook_pizzas(pizza_info* p_info) {

	/* Wait for the correct number of available ovens */
	pthread_mutex_lock(&oven_mutex);
	while(p_info->num_of_pizzas > available_ovens)
		pthread_cond_wait(&oven_condv, &oven_mutex);

	available_ovens -= p_info->num_of_pizzas;
	pthread_mutex_unlock(&oven_mutex);

	/* Free the cook now that the pizzas are in the ovens */
	pthread_mutex_lock(&cook_mutex);

	available_cooks++;
	pthread_mutex_unlock(&cook_mutex);
	pthread_cond_signal(&cook_condv);

	/* Bake the pizzas */
	sleep(T_BAKE);

	/* Mark the time the pizzas are ready in order to calculate the time they are cooling */
	clock_gettime(CLOCK_REALTIME, &p_info->order_baked_time);

	/* The ovens are freed once the pizzas are all packed */
}

void package_pizzas(pizza_info* p_info) {

	/* Wait for an available package guy */
	pthread_mutex_lock(&package_mutex);
	while (available_package_guys == 0)
		pthread_cond_wait(&package_condv, &package_mutex);

	--available_package_guys;
	pthread_mutex_unlock(&package_mutex);

	/* Package the pizzas */
	sleep(T_PACK * p_info->num_of_pizzas);

	char msg[MAX_LOG_LENGTH];
	sprintf(msg, "Order %ld prepared in %d seconds", p_info->threadID,
			time_elapsed(&p_info->order_start_time));
	logstr(msg);

	/* Free the ovens now that the pizzas are packaged */
	pthread_mutex_lock(&cook_mutex);

	available_ovens += p_info->num_of_pizzas;
	pthread_mutex_unlock(&cook_mutex);

	/* Free the package guy */
	pthread_mutex_lock(&package_mutex);

	available_package_guys++;
	pthread_mutex_unlock(&package_mutex);
	pthread_cond_signal(&package_condv);
}

void deliver_pizzas(pizza_info* p_info) {

	/* Wait for delivery guy */
	pthread_mutex_lock(&delivery_mutex);
	while (available_delivery_guys == 0)
		pthread_cond_wait(&delivery_condv, &delivery_mutex);

	--available_delivery_guys;
	pthread_mutex_unlock(&delivery_mutex);

	/* Deliver pizzas */
	int delivery_duration = randint(T_DEL_LOW, T_DEL_HIGH);
	sleep(delivery_duration);

	char msg[MAX_LOG_LENGTH];
	sprintf(msg, "Order %ld delivered in %d seconds", p_info->threadID,
			time_elapsed(&p_info->order_start_time));
	logstr(msg);

	/* Update global variables */
	int delivery_time = time_elapsed(&p_info->order_start_time);
	int cooling_time = time_elapsed(&p_info->order_baked_time);

	increment(delivery_time, &total_delivery);
	max(delivery_time, &max_delivery);
	increment(cooling_time, &total_cooling);
	max(cooling_time, &max_cooling);

	/* Return to pizza shop */
	sleep(delivery_duration);

	/* Free delivery guy */
	pthread_mutex_lock(&delivery_mutex);

	++available_delivery_guys;
	pthread_cond_signal(&delivery_condv);
	pthread_mutex_unlock(&delivery_mutex);
}

/* ==================== Utility Functions ==================== */

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

int randint(int start, int end) {
    return (rand_r(&rand_seed) % (end - start + 1)) + start;
}

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

int time_elapsed(struct timespec *tp) {
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return now.tv_sec - tp->tv_sec;
//	return (time(NULL) - start_time);
}
