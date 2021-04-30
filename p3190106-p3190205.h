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

typedef pthread_t thread;
typedef pthread_mutex_t mutex;
typedef pthread_cond_t condv;

/* mutexes for utility functions in this file */
mutex out_lock;
mutex increment_lock;

void init_helper_mutexes() {
    pthread_mutex_init(&out_lock, NULL);
    pthread_mutex_init(&increment_lock, NULL);
}

unsigned int rand_seed = 869;

/*
 * logstr and logerr both append a newline in the end.
 * use locks so that the lines are not scrambled.
 * spinlocks are fine because printing is very quick.
 */ 

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
#endif
