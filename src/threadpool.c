#include"threadpool.h"

/* const variables for thread pool management*/
typedef int thread_pool_state;
const thread_pool_state THREAD_POOL_RUNNING=1;
const thread_pool_state THREAD_POOL_STOPPED=0;
const thread_pool_state THREAD_POOL_PAUSED=2;


/* const variables for future management*/
const int FUTURE_READY=1;
const int FUTURE_UNREADY=0;

/* const variables for thread list management*/
const int THREAD_FREE_SLOT=-1;


/**
 * This structure describes the status and value of the work result passed to the thread pool.
 */
struct _future
{
	void* result;
	int is_ready;
	pthread_mutex_t mutex;
	pthread_cond_t ready;
};

/**
 * structure that represents the work that the thread must perform.
 */
typedef struct _job
{
    void *arg;
    future_t* future;
    void* (*start_routine)(void*);

}job_t;
/**
 * structure that describes the threadPool's properties.
 */
struct _thread_pool
{
    /**Threads info**/
    pthread_t* thread_list;
    int n_thread;

    /**Jobs info**/
    linked_list_t* jobs_list;
	pthread_cond_t job_is_empty;

    /**Mutex and Conds**/
    pthread_mutex_t mutex;
    pthread_cond_t thread_pool_paused;

    /**Thread pool current state**/
    thread_pool_state state;
};


/****FUTURE****/
inline future_t* create_future(void);
void destroy_future(future_t* future );
int is_ready(future_t* future);
void* future_get(future_t* future);

/****JOB*******/
inline struct _job *create_job(void );
void destroy_job(struct _job *job );
void destroy_job_and_future(struct _job *job );
struct _job* init_job(void *(*start_routine)(void*),void *arg);

/****THREADPOOL STATE*****/
int start_thread_pool(thread_pool_t* tp);
int pause_thread_pool(thread_pool_t* tp);
int change_thread_pool_state(thread_pool_state state,thread_pool_t* tp);
int shut_down_now_thread_pool(thread_pool_t* thread_pool);
int shut_down_thread_pool(thread_pool_t* thread_pool);

/****THREADPOOL JOBS*****/
future_t* add_job_head(thread_pool_t* tp,void *(*start_routine)(void*),void *arg);
future_t* add_job_tail(thread_pool_t* tp,void *(*start_routine)(void*),void *arg);

/****THREADPOOL CREATION/DESCTRUCT*****/
thread_pool_t* create_fixed_size_thread_pool(int size,const pthread_attr_t *attr);
void destroy_thread_pool(thread_pool_t* thread_pool);

/*******THREADPOOL LOGIC*******/
void* thread_wrapper(void* tp);

/*****AUX FOO****///todo check restrict meaning
inline void tp_cond_init(pthread_cond_t* restrict cond){
    if(pthread_cond_init(cond,NULL)!=0){abort();}
}
inline void tp_cond_destroy(pthread_cond_t*  cond){
    if(pthread_cond_destroy(cond)!=0){abort();}
}




/**********************************FUTURE*********************************/

inline future_t* create_future(void ){
    future_t* future=(future_t*)malloc(sizeof(struct _future));
    if(!future){
        return NULL;
    }
    MUTEX_INIT(future->mutex);
    tp_cond_init(&(future->ready));
    future->is_ready=FUTURE_UNREADY;
    return future;
}

//la free al puntatore del payload di future deve essere fatta dall'utente in quanto non sappiamo come è la struttura o tipo ecc
void destroy_future(future_t* future ) {
    if (!future)return;
    MUTEX_DESTROY(future->mutex);
    tp_cond_destroy(&(future->ready));
    free(future);
}


int is_ready(future_t* future){
    int ir;
    MUTEX_LOCK(future->mutex);
    ir=future->is_ready;
    MUTEX_UNLOCK(future->mutex);
    return ir;
}



void* future_get(future_t* future){
    int ir;
    void* res;
    MUTEX_LOCK(future->mutex);
    while((ir=future->is_ready)==FUTURE_UNREADY){
        pthread_cond_wait(&(future->ready),&(future->mutex));
    }
    MUTEX_UNLOCK(future->mutex);
    res=future->result;
    return  res;
}



/********************************JOB************************************/

job_t* create_job(void){
    return (struct _job*)malloc(sizeof(struct _job));
}

void destroy_job_and_future(job_t* job ){
    if(!job)return;
    destroy_future(job->future);
    free(job);
}

void destroy_job(job_t* job ) {
    free(job);
}

//todo la domanda è.. Dato he in linklist.h se un lock fallisce fa l'abort è veramnte necessario gestire gli errori?


//todo check cosa fà se start_routine==Null?
job_t* init_job(void *(*start_routine)(void*),void *arg){
    job_t* job=create_job();
    if(!job){
        return NULL;
    }

    job->future=create_future();
    if(!job->future) {
        destroy_job(job);
        return NULL;
    }
    //INIT _JOB
    job->arg=arg;
    job->start_routine=start_routine;
    return job;
}





/***************************THREADPOOL**************************************/


/****THREADPOOL STATE*****/


int shut_down_now_thread_pool(thread_pool_t* tp){
    if(!tp)return -1;
    shut_down_thread_pool(tp);
    for(int i=0;i<tp->n_thread;i++){
        pthread_cancel(tp->thread_list[i]);
    }
    return 0;
}


int shut_down_thread_pool(thread_pool_t* tp){
    return change_thread_pool_state(THREAD_POOL_STOPPED,tp);
}



int change_thread_pool_state(thread_pool_state state,thread_pool_t* tp){
    if(!tp){
        return -1;
    }
    MUTEX_LOCK(tp->mutex);
    tp->state=state;
    pthread_cond_broadcast(&(tp->thread_pool_paused));
    MUTEX_UNLOCK(tp->mutex);
    return 0;
}

int start_thread_pool(thread_pool_t* tp){
    return change_thread_pool_state(THREAD_POOL_RUNNING,tp);
}


int pause_thread_pool(thread_pool_t* tp){
    return change_thread_pool_state(THREAD_POOL_PAUSED,tp);
}



/****THREADPOOL JOBS*****/

future_t* add_job_tail(thread_pool_t* tp,void *(*start_routine)(void*),void *arg){
    job_t* job=init_job(start_routine,arg);
    if(!job){
        return NULL;
    }
    if(list_push_value(tp->jobs_list,job)<0){
        destroy_job_and_future(job);
        return NULL;
    }
    pthread_cond_broadcast(&(tp->job_is_empty));//broadcast -->lost wakeup problem//todo check return value

    return job->future;
}



future_t* add_job_head(thread_pool_t* tp,void *(*start_routine)(void*),void *arg){
    job_t* job=init_job(start_routine,arg);
    if(!job){
        return NULL;
    }
    if(list_insert_value(tp->jobs_list,job,0)<0){
        destroy_job_and_future(job);
        return NULL;
    }
    pthread_cond_broadcast(&(tp->job_is_empty));//lost wakeup problem//todo check return value
    return job->future;;
}


/****THREADPOOL CREATION/DESCTRUCT*****/

thread_pool_t* create_fixed_size_thread_pool(int size,const pthread_attr_t *attr){
    int error=0;//todo is really necessary?

    //checking input values
    if(size<=0)return NULL;


	thread_pool_t* tp=(thread_pool_t*)malloc(sizeof(struct _thread_pool));
	if(!tp){
		return NULL;
	}

	tp->thread_list=(pthread_t*)malloc(sizeof( pthread_t)*size);
	if(!tp->thread_list){
		free(tp);
	    return NULL;
	}

	tp->jobs_list=list_create();
	if(!tp->jobs_list){
	    free(tp->thread_list);
	    free(tp);
		return NULL;
	}
    tp_cond_init(&(tp->job_is_empty));
    MUTEX_INIT(tp->mutex);
    tp_cond_init(&(tp->thread_pool_paused));
	tp->n_thread=size;
	tp->state=THREAD_POOL_PAUSED;
	//INIT THREADS
	for(int i=0;i<tp->n_thread;i++){
		tp->thread_list[i]=THREAD_FREE_SLOT;
	}

	for(int i=0;i<tp->n_thread;i++){
		if(pthread_create(&(tp->thread_list[i]),attr,thread_wrapper,(void*)tp)!=0){
			error=1;
		}
	}
	if(error){
		destroy_thread_pool(tp);
	}
	return tp;
}

void destroy_thread_pool(thread_pool_t* thread_pool){
    if(!thread_pool)return;
    tp_cond_destroy(&(thread_pool->job_is_empty));
    tp_cond_destroy(&(thread_pool->thread_pool_paused));
    MUTEX_DESTROY(thread_pool->mutex);
	list_destroy(thread_pool ->jobs_list);
    free(thread_pool ->thread_list);
    free(thread_pool);
    thread_pool=NULL;
}


/*******THREADPOOL LOGIC*******/

void* thread_wrapper(void* arg){
    thread_pool_t* tp=(thread_pool_t*)arg;
    job_t* my_job;
	void* result;
	void* (*foo)(void*);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	while(tp->state!=THREAD_POOL_STOPPED)
	{
	    if(tp->state==THREAD_POOL_RUNNING) {
	        list_lock(tp->jobs_list);
	        while ((my_job = (job_t *) list_fetch_value(tp->jobs_list, 0)) == NULL) {
	            pthread_cond_wait(&(tp->job_is_empty), get_lock_reference(tp->jobs_list));
	        }
	        list_unlock(tp->jobs_list);
	        foo = my_job->start_routine;
	        result = foo(my_job->arg);
	        MUTEX_LOCK(my_job->future->mutex);
	            my_job->future->is_ready =FUTURE_READY;
	            my_job->future->result = result;
	        MUTEX_UNLOCK(my_job->future->mutex);
            pthread_cond_broadcast(&(my_job->future->ready));
	        destroy_job(my_job);

	    }else if(tp->state==THREAD_POOL_STOPPED) {
            break;
        }else if(tp->state==THREAD_POOL_PAUSED) {
            MUTEX_LOCK(tp->mutex);
            while (tp->state == THREAD_POOL_PAUSED && tp->state != THREAD_POOL_STOPPED) {
                pthread_cond_wait(&(tp->thread_pool_paused), &(tp->mutex));
            }
            MUTEX_UNLOCK(tp->mutex);
        }

	}
	return NULL;
}


/*todo list:
 * 1)Dato che i thread wrapper sono stati settati con PTHREAD_CANCEL_ASYNCHRONOUS
 *      -se l'utente chiama shutdownnow possibili memori leaks deadlock ecc
 *      -se invece l'utente chiama shutdown questi mml e ddlck non possono avvenire
 *      *si potrebbe implementare un qualcosa per risolver il problema della shutdown_now?
 *
 * 2)usare valgrind per memcheck
 *
 *
 *
 *
 *
 *      */