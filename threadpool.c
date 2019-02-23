#include"threadpool.h"

//todo aggiungere codice per settare campo state del thread
enum _thread_state{
    FREE_SLOT,
    ACTIVE,//for cached  tp
    INACTIVE
};

struct _thread_slot{
    enum _thread_state thread_state;
    pthread_t thread_id;
};


/**
 * This structure describes the status and value of the work result passed to the thread pool.
 */
struct _future
{
	void* result;
	enum future_state is_ready;
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
    struct _thread_slot* thread_list;
    int n_thread;

    /**Jobs info**/
    linked_list_t* jobs_list;
	pthread_cond_t job_is_empty;

    /**Mutex and Conds**/
    pthread_mutex_t mutex;
    pthread_cond_t thread_pool_paused;

    /**Thread pool current state**/
    enum thread_pool_state state;
};


/****FUTURE****/
static inline future_t* create_future(void);
static inline void set_future_result_and_state(job_t* job,void* result);

/****JOB*******/
static inline struct _job *create_job(void );
static inline void destroy_job(struct _job *job );
static inline void destroy_job_and_future(struct _job *job );
static inline struct _job* init_job(void *(*start_routine)(void*),void *arg);

/****THREADPOOL STATE*****/
static inline int change_thread_pool_state(enum thread_pool_state state,thread_pool_t* tp);

/*******THREADPOOL LOGIC*******/
static inline void* thread_wrapper(void* arg);
static inline void thread_pool_running_logic(thread_pool_t* tp);
static inline void thread_pool_paused_logic(thread_pool_t* tp);

/*****AUX FOO****///todo check restrict meaning
static inline void tp_cond_init(pthread_cond_t*  cond);
static inline void tp_cond_destroy(pthread_cond_t*  cond);






/*****AUX FOO****/
static inline void tp_cond_init(pthread_cond_t*  cond){
    if(pthread_cond_init(cond,NULL)!=0){abort();}
}
static inline void tp_cond_destroy(pthread_cond_t*  cond){
    if(pthread_cond_destroy(cond)!=0){abort();}
}

static inline void tp_cond_broadcast(pthread_cond_t* cond){
    if(pthread_cond_broadcast(cond)!=0){abort();}
}



/**********************************FUTURE*********************************/

future_t* create_future(void ){
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


enum future_state get_future_state(future_t* future){
    enum future_state ir;
    if(!future)
        return FUTURE_ERROR;
    MUTEX_LOCK(future->mutex);
        ir=future->is_ready;
    MUTEX_UNLOCK(future->mutex);
    return ir;
}



void* future_get(future_t* future){
    void* res;
    if(!future)return NULL;
    MUTEX_LOCK(future->mutex);
        while((future->is_ready)==FUTURE_UNREADY){
            pthread_cond_wait(&(future->ready),&(future->mutex));
        }
        res=future->result;
    MUTEX_UNLOCK(future->mutex);
    return  res;
}

void set_future_result_and_state(job_t* job,void* result){
    if(!job)return;
    MUTEX_LOCK(job->future->mutex);
        job->future->is_ready =FUTURE_READY;
        job->future->result = result;
    MUTEX_UNLOCK(job->future->mutex);
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



job_t* init_job(void *(*start_routine)(void*),void *arg){
    if(!start_routine)return NULL;

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
    shut_down_thread_pool(tp);////todo non va bene la chiamata a shut_down è bloccante (nuova utilizzo con detached).
    for(int i=0;i<tp->n_thread;i++){
        pthread_cancel(tp->thread_list[i].thread_id);
    }
    return 0;
}


int shut_down_thread_pool(thread_pool_t* tp){
    void* ret;
    if(change_thread_pool_state(THREAD_POOL_STOPPED,tp)<0){
        return -1;
    }
    for(int i=0;i<tp->n_thread;i++){
        if(tp->thread_list[i].thread_state!=FREE_SLOT) {
            pthread_join(tp->thread_list[i].thread_id, &ret);
        }
    }
    return 0;
}



int change_thread_pool_state(enum thread_pool_state state,thread_pool_t* tp){
    if(!tp){
        return -1;
    }
    MUTEX_LOCK(tp->mutex);
        tp->state=state;
        tp_cond_broadcast(&(tp->thread_pool_paused));
    MUTEX_UNLOCK(tp->mutex);
    list_lock(tp->jobs_list);
        tp_cond_broadcast(&(tp->job_is_empty));
    list_unlock(tp->jobs_list);
    return 0;
}

int start_thread_pool(thread_pool_t* tp){
    return change_thread_pool_state(THREAD_POOL_RUNNING,tp);
}


int pause_thread_pool(thread_pool_t* tp){
    return change_thread_pool_state(THREAD_POOL_PAUSED,tp);
}

enum thread_pool_state get_thread_pool_state(thread_pool_t* tp){
    enum thread_pool_state state;
    if(!tp){
        state=THREAD_POOL_ERROR;
    }else {
        MUTEX_LOCK(tp->mutex);
        state = tp->state;
        MUTEX_UNLOCK(tp->mutex);
    }
    return state;
}


/****THREADPOOL JOBS*****/


future_t* add_job_tail(thread_pool_t* tp,void *(*start_routine)(void*),void *arg){
    if(!tp||!start_routine){
        return NULL;
    }
    job_t* job=init_job(start_routine,arg);
    if(!job){
        return NULL;
    }
    if(list_push_value(tp->jobs_list,job)<0){
        destroy_job_and_future(job);
        return NULL;
    }
    list_lock(tp->jobs_list);
        tp_cond_broadcast(&(tp->job_is_empty));//broadcast -->lost wakeup problem
    list_unlock(tp->jobs_list);
    return job->future;
}



future_t* add_job_head(thread_pool_t* tp,void *(*start_routine)(void*),void *arg){
    if(!tp||!start_routine){
        return NULL;
    }
    job_t* job=init_job(start_routine,arg);
    if(!job){
        return NULL;
    }
    if(list_insert_value(tp->jobs_list,job,0)<0){
        destroy_job_and_future(job);
        return NULL;
    }
    list_lock(tp->jobs_list);
        tp_cond_broadcast(&(tp->job_is_empty));
    list_unlock(tp->jobs_list);
    return job->future;
}


/****THREADPOOL CREATION/DESCTRUCT*****/

thread_pool_t* create_fixed_size_thread_pool(int size,const pthread_attr_t *attr){
    thread_pool_t* tp=NULL;
    if(size<=0)return NULL;
    if(!(tp=(thread_pool_t*)malloc(sizeof(struct _thread_pool)))){
		return NULL;
	}
	if(!(tp->thread_list=(struct _thread_slot*)malloc(sizeof(struct _thread_slot)*size))){
        free(tp);
        return NULL;
    }
	if(!(tp->jobs_list=list_create())){
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
		tp->thread_list[i].thread_state=FREE_SLOT;
	}

	for(int i=0;i<tp->n_thread;i++){
		if(pthread_create(&(tp->thread_list[i].thread_id),attr,thread_wrapper,(void*)tp)!=0){
		    shut_down_thread_pool(tp);
            destroy_thread_pool(tp);
            break;
		}else{
		    tp->thread_list[i].thread_state=INACTIVE;
		}
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


void thread_pool_paused_logic(thread_pool_t* tp){
    MUTEX_LOCK(tp->mutex);
        while (tp->state == THREAD_POOL_PAUSED ) {
            pthread_cond_wait(&(tp->thread_pool_paused), &(tp->mutex));
        }
    MUTEX_UNLOCK(tp->mutex);
}



void thread_pool_running_logic(thread_pool_t* tp) {
    job_t* my_job=NULL;
    void* result=NULL;
    void* (*foo)(void*);

    list_lock(tp->jobs_list);
    while (get_thread_pool_state(tp)==THREAD_POOL_RUNNING && (my_job = (job_t *) list_fetch_value(tp->jobs_list, 0)) == NULL) {
        pthread_cond_wait(&(tp->job_is_empty), get_lock_reference(tp->jobs_list));
    }
    list_unlock(tp->jobs_list);
    if(my_job!=NULL) {
        foo = my_job->start_routine;
        result = foo(my_job->arg);//foo can not be NULL because init_job does not allow it
        set_future_result_and_state(my_job, result);
        tp_cond_broadcast(&(my_job->future->ready));
        destroy_job(my_job);
    }
}

void* thread_wrapper(void* arg){
    thread_pool_t* tp=(thread_pool_t*)arg;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	while(tp->state!=THREAD_POOL_STOPPED)
	{
        switch(tp->state){
            case THREAD_POOL_RUNNING:
                thread_pool_running_logic(tp);
                break;
            case THREAD_POOL_ERROR:
	        case THREAD_POOL_STOPPED:
	            break;
            case THREAD_POOL_PAUSED:
                thread_pool_paused_logic(tp);
                break;
	    }
	}
    pthread_exit(NULL);//todo si potrebbe aggiungere un enumeratore con lo stato di uscita del thread
}




