#ifndef __PIZZA_H__
#define __PIZZA_H__

#define N_TELE 3
#define N_COOK 3
#define N_OVEN 10
#define N_DELIVERER    7
#define T_ORDER_LOW    1
#define T_ORDER_HIGH   5
#define N_ORDER_LOW    1
#define N_ORDER_HIGH   5
#define T_PAYMENT_LOW  1
#define T_PAYMENT_HIGH 2
#define C_PIZZA 10
#define P_FAIL  0.05
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

typedef struct{
	int threadID;
	int num_of_pizzas;
	clock_t time;
} pizza_info;

unsigned int rand_seed = 869;

void startOrder(pizza_info * pid);
void prepare_pizzas(pizza_info* pid);
void cook_pizzas(pizza_info* pid);
void package_pizzas(pizza_info* p_info);

/*
 * logstr and logerr both append a newline in the end.
 * use locks so that the lines are not scrambled.
 * spinlocks are fine because printing is very quick.
 */

void logstr(char* string);

void logerr(char* string);

/* returns random integer in the range [start, end] */
int randint(int start, int end);

/* increments `total` by `amt`  s a f e l y */
void increment(int amt, int* total);

/* returns the time elapsed from "process_time" to now */
double time_elapsed(clock_t process_time);
#endif
