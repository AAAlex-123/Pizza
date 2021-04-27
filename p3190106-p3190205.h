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

typedef pthread_cond_t condv;
typedef pthread_mutex_t mutex;
typedef pthread_t thread;

mutex out_lock;
mutex pay_lock;

unsigned int rand_seed = 869;

void init_helper_vars() {
	pthread_mutex_init(&out_lock, NULL);
	pthread_mutex_init(&pay_lock, NULL);
}

/* logstr and logerr both append a newline in the end */
void logstr(char* string) {
	pthread_mutex_lock(&out_lock);
	printf("%s\n",string);
	fflush(stdout);
	pthread_mutex_unlock(&out_lock);
}

/* Printing both in stdout or stderr won't cause a crash but might print messages in the same line */
void logerr(char* string) { 
	pthread_mutex_lock(&out_lock);
	fprintf(stderr, "%s\n",string);
	fflush(stderr);
	pthread_mutex_unlock(&out_lock);
}

int randint(int start, int end) {
	if(rand_seed == 869)
		logerr("Warning: no seed initialized.\nTODO: REMOVE THIS");
	return (rand_r(&rand_seed) % (end-start+1)) + start;
}

/* Increase `sum` by `amt`  s a f e l y */
void pay(int amt, int* sum) {
	pthread_mutex_lock(&pay_lock);
	*sum += amt;
	pthread_mutex_unlock(&pay_lock);
}

#endif
