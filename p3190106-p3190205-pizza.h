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

#define MAX_LOG_LENGTH 100

typedef pthread_t thread;
typedef pthread_mutex_t mutex;
typedef pthread_cond_t condv;

unsigned int rand_seed;

/* struct with information about an order.
 * used to facilitate organisation of code into functions. */
typedef struct {
	long threadID;
	int num_of_pizzas;
	struct timespec order_start_time;
	struct timespec order_baked_time;
} pizza_info;

/* Function called by all threads to make an order.
 * Successively executes all steps for the pizza delivery. */
void* make_order(void*);

/* one function for each part of the order, for each resource.
 * they all share the same information about an order. */
int order_pizzas(pizza_info*);
void prepare_pizzas(pizza_info*);
void cook_pizzas(pizza_info*);
void package_pizzas(pizza_info*);
void deliver_pizzas(pizza_info*);

/* ==================== Utility Functions ==================== */

/*
 * logstr and logerr both append a newline in the end.
 * use locks so that the lines are not scrambled.
 * spinlocks are fine because printing is very quick.
 */
void logstr(char*);
void logerr(char*);

/* returns random integer in the range [start, end] */
int randint(int start, int end);

/* increments `total` by `amt` */
void increment(int amt, int* total);

/* sets `max` to the maximum of `val` and `max` */
void max(int val, int* max);

/* returns the time elapsed from start to now */
int time_elapsed(struct timespec*);

#endif
