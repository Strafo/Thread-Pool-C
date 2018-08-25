#include"threadpool.h"

//gestione thread pool
const int THREAD_POOL_RUNNING=1;
const int THREAD_POOL_STOPPED=0;
const int THREAD_POOL_PAUSED=2;


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
	pthread_cond_t job_is_empty;
	pthread_t* thread_list;
	linked_list_t* jobs_list;
	int n_thread;
	int state;
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


void shut_down_now(thread_pool_t* thread_pool);
void shut_down(thread_pool_t* thread_pool);
struct _job* init_job(const pthread_attr_t *attr,void *(*start_routine)(void*),void *arg);


void destroy_thread_pool(thread_pool_t* thread_pool);
void destroy_future(future_t* future );
void destroy_job(struct _job *job );
void destroy_job_future(struct _job *job );


int is_ready(future_t* future);
void* thread_wrapper(void* tp);
void* future_get(future_t* future);




thread_pool_t* create_fixed_size_thread_pool(int size,const pthread_attr_t *attr){//todo add pthread_setcancelstate

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

	if(pthread_cond_init(&(tp->job_is_empty),NULL)!=0){//todo la domanda è.. Dato he in linklist.h se un lock fallisce fa l'abort è veramnte necessario gestire gli errori?
		abort();
	}
	tp->n_thread=size;
	tp->state=THREAD_POOL_PAUSED;
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
	MUTEX_INIT(&(future->mutex);
	if(pthread_cond_init(&(future->ready),NULL)!=0){abort();}//todo fare una cosa simile alle macro di atomic_defs.h

	future->is_ready=FUTURE_UNREADY;
	return future;
}

job_t* create_job(void){
	return (struct _job*)malloc(sizeof(struct _job));
}





void destroy_job(job_t* job ) {
	//todo devo fare la free  su start_routine? (non di certo su future! guarda thread wrapper)
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
	list_destroy(threadPool ->jobs_list);
    free(thread_pool ->thread_list);
    free(thread_pool);
    thread_pool=NULL;
}


void shut_down_now(thread_pool_t* tp){
    if(!tp)return;
    tp->state=THREAD_POOL_STOPPED;
    for(int i=0;i<tp->nThread;i++){
        pthread_cancel(tp->threadList[i]);
    }

}


void shut_down(thread_pool_t* tp){
    if(!tp)return;
    tp->active=THREAD_POOL_STOPPED;
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

int start_thread_pool(thread_pool_t* tp){
	if(!tp)return -1;
	tp->state=THREAD_POOL_RUNNING;
	return 0;
}


int pause_thread_pool(thread_pool_t* tp){
	if(!tp)return -1;
	tp->state=THREAD_POOL_PAUSED;
	return 0;
}

int stop_thread_pool(thread_pool_t* tp){
    if(!tp)return -1;
    tp->state=THREAD_POOL_STOPPED;
    return 0;
}


void* thread_wrapper(void* arg){
    thread_pool_t* tp=(thread_pool_t*)arg;
	struct job_t* my_job;
	void* result;
	void* (*foo)(void*);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);//todo check

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
                MUTEX_UNLOCK(&(my_job->future->mutex));
                pthread_cond_broadcast(&(my_job->future->ready));//broadcast -->lost wakeup problem
                destroy_job(my_job);
            break;
            case THREAD_POOL_STOPPED:
                break;
            case THREAD_POOL_STOPPED:
                //todo add cond for stopped!
                break;
        }
	}
	return NULL;
}
