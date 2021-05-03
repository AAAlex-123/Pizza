#ifndef __PIZZA_H__
#define __PIZZA_H__

#define N_TELE 3
#define N_COOK 2
#define N_OVEN 10
#define N_DELIVERER    7
#define T_ORDER_LOW    1
#define T_ORDER_HIGH   5
#define N_ORDER_LOW    1
#define N_ORDER_HIGH   5
#define T_PAYMENT_LOW  1
#define T_PAYMENT_HIGH 2
#define C_PIZZA 10
#define P_FAIL 0.05
#define T_PREP  1
#define T_BAKE  10
#define T_PACK  2
#define T_DEL_LOW  5
#define T_DEL_HIGH 15

#define DEBUG
#define MAX_LOG_LENGTH 100

typedef pthread_t thread;
typedef pthread_mutex_t mutex;
typedef pthread_cond_t condv;

unsigned int rand_seed;

/* Struct containing information about each order.
 * Used to facilitate organization of code into separate functions. */
typedef struct {
	long threadID;
	int num_of_pizzas;
	struct timespec order_start_time;
	struct timespec order_baked_time;
} order_info;

/*
 * Function called by all threads to make an order. Takes an order_info pointer as argument.
 * Successively executes all steps for the pizza delivery.
 * Returns NULL if the order was completed successfully, 1 if the order was aborted.
 */
void* make_order(void*);

/*
 * Separate functions each executing a part of the order.
 * Called successively for each order (represented by the order_info pointer), 
 * but accessed by multiple threads at the same time.
 */
int order_pizzas(order_info*);
void prepare_pizzas(order_info*);
void cook_pizzas(order_info*);
void package_pizzas(order_info*);
void deliver_pizzas(order_info*);

/* ==================== Utility Functions ==================== */

/*
 * Functions to safely print messages in the stdout and stderr streams,
 * appending a new line character at the end.
 * Use of the same lock by both so that the lines are not scrambled in the console.
 * Blocking implemented by spinlocks because printing is very quick (1 syscall).
 */
void logstr(char*);
void logerr(char*);

/* Returns a random integer in the range [start, end] */
int randint(int start, int end);

/* Increments `total` by `amt` in a thread-safe way */
void increment(int amt, int* total);

/* Sets `max` to the maximum of `val` and `max` in a thread-safe way*/
void max(int val, int* max);

/* Returns the time elapsed from start to now */
int time_elapsed(struct timespec*);

#endif
