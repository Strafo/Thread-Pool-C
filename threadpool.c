#include"threadpool.h"

//gestione thread pool
typedef int thread_pool_state;
const thread_pool_state THREAD_POOL_RUNNING=1;
const thread_pool_state THREAD_POOL_STOPPED=0;
const thread_pool_state THREAD_POOL_PAUSED=2;


/*gestione future*/
const int FUTURE_READY=1;
const int FUTURE_UNREADY=0;

/*gestione lista threads*/

const int THREAD_FREE_SLOT=NULL;



struct _future
{
	void* result;
	int is_ready;
	pthread_mutex_t mutex;
	pthread_cond_t ready;
};
struct _thread_pool
{
    pthread_t* thread_list;
    int n_thread;


    linked_list_t* jobs_list;
	pthread_cond_t job_is_empty;

    pthread_mutex_t thread_pool_mutex;
    pthread_cond_t thread_pool_paused;
    thread_pool_state state;
};



typedef struct _job
{
	void *arg;
	future_t* future;
	void* (*start_routine)(void*);

} job_t;



thread_pool_t* create_fixed_size_thread_pool(int size);
inline future_t* create_future(void);
inline struct _job *create_job(void );

int start_thread_pool(thread_pool_t* tp);
int pause_thread_pool(thread_pool_t* tp);
void shut_down_now_thread_pool(thread_pool_t* thread_pool);
void shut_down_thread_pool(thread_pool_t* thread_pool);



void destroy_thread_pool(thread_pool_t* thread_pool);
void destroy_future(future_t* future );
void destroy_job(struct _job *job );
void destroy_job_future(struct _job *job );


int is_ready(future_t* future);
void* thread_wrapper(void* tp);
void* future_get(future_t* future);
struct _job* init_job(const pthread_attr_t *attr,void *(*start_routine)(void*),void *arg);
int change_thread_pool_state(thread_pool_state state,thread_pool_t* tp);



thread_pool_t* create_fixed_size_thread_pool(int size,const pthread_attr_t *attr){

	int error=0;
	thread_pool_t* tp=(thread_pool_t*)malloc(sizeof(struct _thread_pool));
	if(!tp){
		return NULL;
	}

	tp->threadList=(pthread_t*)malloc(sizeof( pthread_t)*size);
	if(!tp->thread_list){
		free(tp);
	    return NULL;
	}

	tp->jobs_list=list_create();
	if(!tp->jobsList){
	    free(tp->thread_list);
	    free(tp);
		return NULL;
	}

	if(pthread_cond_init(&(tp->job_is_empty),NULL)!=0){//todo la domanda è.. Dato he in linklist.h se un lock fallisce fa l'abort è veramnte necessario gestire gli errori?  ->guarda le note in cima a linklist.h SIGABRT ecc
		abort();
	}

    MUTEX_INIT(&(tp->mutex));
    if(pthread_cond_init(&(tp->thread_pool_paused),NULL)!=0){//todo la domanda è..
        abort();
    }

	tp->n_thread=size;
	tp->state=THREAD_POOL_PAUSED;

	//INIT THREADS
	for(int i=0;i<tp->n_thread;i++){
		tp->threadList[i]=THREAD_FREE_SLOT;
	}

	for(int i=0;i<tp->n_thread;i++){
		if(pthread_create(&(tp->threadList[i]),attr,thread_wrapper,(void*)tp)!=0){
			error=1;
		}
	}
	if(error){
		destroy_thread_pool(tp);
	}
	return tp;
}





future_t* create_future(void ){
	future_t* future=(future_t*)malloc(sizeof(struct _future));
	if(!future){
		return NULL;
	}
	MUTEX_INIT(&(future->mutex));
	if(pthread_cond_init(&(future->ready),NULL)!=0){abort();}//todo fare una cosa simile alle macro di atomic_defs.h

	future->is_ready=FUTURE_UNREADY;
	return future;
}

job_t* create_job(void){
	return (struct _job*)malloc(sizeof(struct _job));
}





void destroy_job(job_t* job ) {
	free(job);
}

//todo la domanda è.. Dato he in linklist.h se un lock fallisce fa l'abort è veramnte necessario gestire gli errori?

void destroy_future(future_t* future ) {//todo gli facciamo restituire l'errore?
	int result;
	if (!future)return;
	result = pthread_mutex_destroy(&(future->mutex));
	switch (result) {
		case EBUSY:
			//todo
			break;
		case EINVAL:
			//todo
			break;
	}
	result = pthread_cond_destroy(&(future->ready));
	switch (result) {
		case EBUSY:
			//todo
			break;
		case EINVAL:
			//todo
			break;
	}
	free(future);

}

void destroy_thread_pool(thread_pool_t* thread_pool){
    if(!threadPool)return;
    shut_down_now(thread_pool);
    pthread_cond_destroy(&(thread_pool->job_is_empty));//todo check
    pthread_cond_destroy(&(thread_pool->thread_pool_paused));//todo check
    MUTEX_DESTROY(&(thread_pool->mutex));
	list_destroy(threadPool ->jobs_list);
    free(thread_pool ->thread_list);
    free(thread_pool);
    thread_pool=NULL;
}


void shut_down_now_thread_pool(thread_pool_t* tp){
    if(!tp)return;
    shut_down_thread_pool(tp);
    for(int i=0;i<tp->nThread;i++){
        pthread_cancel(tp->threadList[i]);
    }
}


int shut_down_thread_pool(thread_pool_t* tp){
    return change_thread_pool_state(THREAD_POOL_STOPPED,tp);
}




//todo check cosa fà se start_routine==Null?
job_t* init_job(const pthread_attr_t *attr,void *(*start_routine)(void*),void *arg){
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
    job->attr=attr;
    job->start_routine=start_routine;
    return job;
}

void destroy_job_and_future(job_t* job ){
    if(!job)return;
    destroy_future(job->future);
    free(job);
}

int is_ready(future_t* future){
    int ir;
    MUTEX_LOCK(&(future->mutex));
    ir=future->isReady;
    MUTEX_UNLOCK(&(future->mutex));
    return ir;
}



future_t* add_job_tail(thread_pool_t* tp,const pthread_attr_t *attr,void *(*start_routine)(void*),void *arg){
	future_t* future;
	job_t* job=init_job(attr,start_routine,arg);
	if(!job){
		return NULL;
	}
	if(list_push_value(tp->jobsList,job)<0){
		destroy_job_future(job);
		return NULL;
	}
	pthread_cond_broadcast(&(tp->job_is_empty));//broadcast -->lost wakeup problem//todo check return value

	return job->future;
}



future_t* add_job_head(thread_pool_t* tp,const pthread_attr_t *attr,void *(*start_routine)(void*),void *arg){
	future_t* future;
	job_t* job=init_job(attr,start_routine,arg);
	if(!job){
		return NULL;
	}
	if(list_insert_value(tp->jobsList,job,0)<0){
		destroy_job_future(job);
		return NULL;
	}
	pthread_cond_broadcast(&(tp->job_is_empty));//lost wakeup problem//todo check return value
	return job->future;;
}



void* future_get(future_t* future){
	int ir;
	void* res;
	MUTEX_LOCK(&(future->mutex));
	while((ir=future->isReady)==FUTURE_UNREADY){
		pthread_cond_wait(&(future->ready),&(future->mutex));
	}
	MUTEX_UNLOCK(&(future->mutex));
	res=future->result;
	destroy_future(future);
	return  res;
}


int change_thread_pool_state(thread_pool_state state,thread_pool_t* tp){
    if(!tp){
        return -1;
    }
    MUTEX_LOCK(&(tp->mutex));
        tp->state=state;
        pthread_cond_broadcast(&(tp->thread_pool_paused));
    MUTEX_UNLOCK(&(tp->mutex));
    return 0;
}

int start_thread_pool(thread_pool_t* tp){
    return change_thread_pool_state(THREAD_POOL_RUNNING,tp);
}


int pause_thread_pool(thread_pool_t* tp){
    return change_thread_pool_state(THREAD_POOL_PAUSED,tp);
}






void* thread_wrapper(void* arg){
    thread_pool_t* tp=(thread_pool_t*)arg;
	struct job_t* my_job;
	void* result;
	void* (*foo)(void*);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	while(tp->active!=THREAD_POOL_STOPPED)
	{
        switch (tp->state) {

            case THREAD_POOL_RUNNING:
                list_lock(tp->jobsList);
                while ((my_job = (job_t *) list_fetch_value(tp->jobsList, 0)) == NULL) {
                    pthread_cond_wait(&(tp->job_is_empty), get_lock_reference(tp->jobsList));
                }
                list_unlock(tp->jobsList);

                foo = my_job->start_routine;
                result = foo(my_job->arg);

                MUTEX_LOCK(&(my_job->future->mutex));
                    my_job->future->isReady = 1;
                    my_job->future->result = result;
                    pthread_cond_broadcast(&(my_job->future->ready));
                MUTEX_UNLOCK(&(my_job->future->mutex));
                destroy_job(my_job);
            break;
            case THREAD_POOL_STOPPED:
                break;
            case THREAD_POOL_PAUSED:
                MUTEX_LOCK(&(tp->mutex));
                while (tp->state==THREAD_POOL_PAUSED&&tp->state!=THREAD_POOL_STOPPED) {
                    pthread_cond_wait(&(tp->thread_pool_paused), &(tp->mutex));
                }
                MUTEX_UNLOCK(&(tp->mutex));
                break;
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
 *
 *
 *
 *
 *
 *      */