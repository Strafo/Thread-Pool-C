#include"threadPool.h"
#define RUNNING 0
#define STOPPING -1

struct _future
{
	void* result;
	bool isReady;
	pthread_mutex_t mutex;
};
struct _threadPool
{
	pthread_t* threadList;
	linked_list_t* jobsList;
	int nThread;
	int active;
};



typedef struct _job
{
	const pthread_attr_t *attr;
	void *arg;
	future_t future;

	void* (*start_routine)(void*);
}* job_t;

void destroy_thread_pool(thread_pool_t threadPool);
void shut_down(thread_pool_t tp);
void shut_down_now(thread_pool_t tp);
void destroy_thread_pool(thread_pool_t threadPool);
inline future_t create_future(void);
inline struct _job *create_job(void );
void destroy_future(future_t future );
void destroy_job_future(struct _job *job );
void destroy_job(struct _job *job );
inline void mutex_lock(pthread_mutex_t *mutex);
inline void mutex_unlock(pthread_mutex_t *mutex);
void* thread_wrapper(void* tp);
bool is_ready(future_t future);
void* future_get(future_t future);
job_t init_job(const pthread_attr_t *attr,void *(*start_routine)(void*),void *arg);


thread_pool_t create_thread_pool(int numberOfExecutors){
	thread_pool_t tp=(thread_pool_t)malloc(sizeof(struct _threadPool));
	if(!tp){
		debug(stdin,"ERROR:Unable to create threadPool\n");
		return NULL;
	}
	tp->threadList=(pthread_t*)malloc(sizeof( pthread_t)*numberOfExecutors);
	if(!tp){
		debug(stdin,"ERROR:Unable to create threadList\n");
		free(tp);
		return NULL;
	}
	tp->jobsList=list_create();
	if(!tp->jobsList){
		debug(stdin,"ERROR:Unable to create jobsList\n");
		free(tp->threadList);
		free(tp);
		return NULL;
	}
	tp->nThread=numberOfExecutors;
	tp->active=RUNNING;
return tp;
}



future_t add_job_tail(thread_pool_t tp,const pthread_attr_t *attr,void *(*start_routine)(void*),void *arg){
	future_t future;
	struct _job* job=init_job(attr,start_routine,arg);
	if(!job){
		return NULL;
	}

	future=job->future;
	//PUSH VALUE
	if(list_push_value(tp->jobsList,job)<0){
		debug(stderr,"ERROR pushing value to jobs list \n");
		destroy_job_future(job);
		return NULL;
	}
return future;
}


future_t add_job_head(thread_pool_t tp,const pthread_attr_t *attr,void *(*start_routine)(void*),void *arg){
	future_t future;
	struct _job* job=init_job(attr,start_routine,arg);
	if(!job){
		return NULL;
	}

	future=job->future;
	//INSERT VALUE
	if(list_insert_value(tp->jobsList,job,0)<0){
		debug(stderr,"ERROR pushing value to jobs list \n");
		destroy_job_future(job);
		return NULL;
	}
return future;
}

job_t init_job(const pthread_attr_t *attr,void *(*start_routine)(void*),void *arg){
	future_t future=create_future();
	if(!future){return NULL;}

	struct _job* job=create_job();
	if(!job){
		destroy_future(future);
		return NULL;
	}
	//INIT _JOB
	job->future=future;
	job->arg=arg;
	job->attr=attr;
	job->start_routine=start_routine;
return job;
}





future_t create_future(void ){
	future_t future=(future_t)malloc(sizeof(struct _future));
	if(!future){
		debug(stderr,"ERROR: creating future\n");
		return NULL;
	}
	if(pthread_mutex_init(&(future->mutex),NULL)!=0){
		debug(stderr,"ERROR: creating future(init mutex)\n");
		free(future);
		return NULL;
	}
	future->isReady=false;
return future;
}

struct _job* create_job(void ){
	struct _job* job=(struct _job*)malloc(sizeof(struct _job));
	if(!job){
		debug(stderr,"ERROR: creating _job\n");
		return NULL;
	}else{
		return job;
	}
}

void destroy_future(future_t future ){
	if(!future)return;
	pthread_mutex_destroy(&(future->mutex));
	free(future);
}

void destroy_job_future(struct _job* job ){
	if(!job)return;
	destroy_future(job->future);
	free(job);
}

void destroy_job(struct _job* job ){
	free(job);
}


void start_thread_pool(thread_pool_t tp){
	if(!tp){debug(stderr,"Illegal argument tp\n");return;}
	for(int i=0;i<tp->nThread;i++){
		if(pthread_create(&(tp->threadList[i]),NULL,thread_wrapper,(void*)tp)!=0){
			debug(stderr,"ERROR: creating thread\n");
			return;
		}
	}
return;
}


//TODO USE VAR CONDITION TO AVOID SPINLOCK

void* thread_wrapper(void* arg){
	thread_pool_t tp=(thread_pool_t)arg;
	struct _job* my_job;
	void* result;
	void* (*foo)(void*);
	debug(stderr,"thread wrapper is running\n");
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  while(tp->active==RUNNING)
  {
	while((my_job=(struct _job*)list_fetch_value(tp->jobsList,0))==NULL){
		if(tp->active==STOPPING)return NULL;
	}
	foo=my_job->start_routine;

	result=foo(my_job->arg);

	mutex_lock(&(my_job->future->mutex) );
	my_job->future->isReady=true;
	my_job->future->result=result;
	mutex_unlock(&(my_job->future->mutex) );
	destroy_job(my_job);
	
  }
  return NULL;
}


void mutex_lock(pthread_mutex_t *mutex){
	int res;
	if((res=pthread_mutex_lock(mutex))!=0)
	{
		switch(res)
		{
			case EAGAIN:
				debug(stderr,"ERROR acquiring mutex EAGAIN\n");
				break;
			case EINVAL:
				debug(stderr,"ERROR acquiring mutex EINVAL\n");
				break;
			case ENOTRECOVERABLE:
				debug(stderr,"ERROR acquiring mutex ENOTRECOVERABLE\n");
				break;
			case EOWNERDEAD:
				debug(stderr,"ERROR acquiring mutex EOWNERDEAD\n");
				break;
			case EDEADLK:
				debug(stderr,"ERROR acquiring mutex EDEADLK\n");
				break;
			default:
				debug(stderr,"ERROR acquiring mutex \n");
				break;
		}
	}
	return;
}

void mutex_unlock(pthread_mutex_t *mutex){
	int res;
	if((res=pthread_mutex_unlock(mutex))!=0)
	{
		switch(res)
		{
			case EPERM:
				debug(stderr,"ERROR releasing mutex EAGAIN\n");
				break;
			
			default:
				debug(stderr,"ERROR releasing mutex \n");
				break;
		}
	}
	return;
}

bool is_ready(future_t future){
	bool ir;
	mutex_lock(&(future->mutex));
	ir=future->isReady;
	mutex_unlock(&(future->mutex));
	return ir;
}

//TODO AVOID SPINLOCK-->pthreadJOIN
void* future_get(future_t future){
	bool ir=false;
	void* res;
	while(!ir){
		mutex_lock(&(future->mutex));
		ir=future->isReady;
		mutex_unlock(&(future->mutex));
	}
	res=future->result;
	destroy_future(future);
	return  res;
}


void destroy_now_thread_pool(thread_pool_t threadPool){
	if(!threadPool)return;
	shut_down_now(threadPool);
	list_destroy(threadPool ->jobsList);
	free(threadPool ->threadList);
	free(threadPool);
}


void shut_down(thread_pool_t tp){
	if(!tp)return;
	tp->active=STOPPING;
}

void shut_down_now(thread_pool_t tp){

	if(!tp)return;
	tp->active=STOPPING;
	for(int i=0;i<tp->nThread;i++){
		pthread_cancel(tp->threadList[i]);
	}

}


void destroy_thread_pool(thread_pool_t tp){
	if(!tp)return;
	shut_down(tp);
	for(int i=0;i<tp->nThread;i++){
		pthread_join(tp->threadList[i],NULL);
	}
	list_destroy(tp ->jobsList);
	free(tp ->threadList);
	free(tp);
}
