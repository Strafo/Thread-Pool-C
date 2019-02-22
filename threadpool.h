/**
 * @file threadpool.h
 * @author Andrea Straforini
 * @date 16/06/2018
 * @brief Simple Thread Pool
 * @note In case of failures reported from the pthread interface
 *         abort() will be called. Callers can catch SIGABRT if more
 *         actions need to be taken.
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H
#ifdef __cplusplus
extern "C" {
#endif

#include<stdlib.h>
#include"linklist.h"
#include<errno.h>
#include<pthread.h>
#include"atomic_defs.h"


/************************************************************************
 *  API
 ************************************************************************/
/**
 * The following API is inspired by the Java ExecutorService (ThreadPool) and Future interfaces.
 * Therefore, to get a general idea of the functioning you can use the Oracle documentation.
 * This version implements the fixed size version with some limitations.
 * e.g. If the threads are deleted they are not restored.
 */


/***************************************
 * FUTURE
 * ************************************/


/**
 * @brief Future
 * A Future represents the result of an asynchronous computation.
 * Functions are provided to check if the computation is complete , to wait for its completion, and to retrieve the result of the computation.
 * The result can only be retrieved using function get when the computation has completed, blocking if necessary until it is ready.
 */
typedef struct _future future_t;


/**
 * Possible states in which the future can be.
 * To get the status of the future, simply call the function: get_future_state
 */
enum future_state{
    FUTURE_ERROR=-1,
    FUTURE_UNREADY=0,
    FUTURE_READY=1
};


/**
 * This function destroys the object but not its contents.
 * The destruction of the future's content is entrusted to the user.
 * @param future the future to be destroyed
 * @note if the structure is in use the behavior is undefined
 *
 */
void destroy_future(future_t* future );


/**
 * Returns the status of the future.
 * The possible states of the object are expressed by the enumerator: future_state
 * @return future_state; FUTURE_ERROR if the future was a null reference.
 */
enum future_state get_future_state(future_t* future);


/**
 * This function allows to obtain the encapsulated content in the future.
 * Return the pointer to the payload of the future.
 * the operation BLOCKS the execution of the current thread until
 * the state of the object turns out to be of type: FUTURE_READY
 * @return the pointer to the payload.
 * @return  null pointer if the future past was invalid.
 * @note the null return value could be the exact payload value and not an error!
 */
void* future_get(future_t* future);




/***************************************
 * THREAD POOL
 * ************************************/


/**
 * @brief Thread Pool
 * This type of thread pool always has a specified number of threads running.
 * Jobs(Tasks) are submitted to the pool via an internal queue, which holds extra jobs whenever there are more active tasks than threads.
 * Threads consume tasks starting at the head of the list.
 */
typedef struct _thread_pool thread_pool_t;


/***************************************
 * THREAD POOL CREATION/DESTRUCTION
 * ************************************/


/**
 * This function creates and initializes a "fixed size" thread pool.
 * @param size  number of threads to be created
 * @param attr  The attr argument points to a pthread_attr_t structure  +
 * whose  contents are  used  at  thread creation time to determine attributes for the new thread;
 * this structure is initialized  using  pthread_attr_init(3)  and related  functions.
 * If  attr is NULL, then the thread is created with default attributes.
 * Initial thread pool status is THREAD_POOL_PAUSED.
 * @return the  thread_pool's pointer
 * @return NUll if size<=0
 */
thread_pool_t* create_fixed_size_thread_pool(int size,const pthread_attr_t *attr);


/**
 * This function frees the memory occupied by the threadpool.Use one of the shutdown functions before using "destroy_threadpool".
 * @param thread_pool (if null reference no action is performed)
 */
void destroy_thread_pool(thread_pool_t* thread_pool);



/***************************************
 * THREAD POOL STATE
 * ************************************/


/* enum for thread pool management*/
enum thread_pool_state{
    THREAD_POOL_ERROR=-1,
    THREAD_POOL_STOPPED=0,
    THREAD_POOL_RUNNING=1,
    THREAD_POOL_PAUSED=2
};


/**
 * Set the threadpool passed in the state: THREAD_POOL_RUNNING
 * @param tp
 * @return 0 if successful, -1 if tp is a null reference
 */
int start_thread_pool(thread_pool_t* tp);


/**
 * Set the threadpool passed in the state: THREAD_POOL_PAUSED
 * @param tp
 * @return 0 if successful, -1 if tp is a null reference
 */
int pause_thread_pool(thread_pool_t* tp);


/**
 *
 * @param thread_pool
 */
int shut_down_now_thread_pool(thread_pool_t* thread_pool);//todo tobe implemented


/**
 * Set the threadpool passed in the state: THREAD_POOL_STOPPED.
 * The function "gently" interrupts the threadpool by waiting for all threads to finish their tasks.
 * The termination of the function is not guaranteed if the threadpool threads remain blocked.
 * After this function call the threadpool is unusable (use destroy_thread_pool () to free the memory)
 * @param thread_pool
 * @return 0 if successful, -1 if tp is a null reference
 */
int shut_down_thread_pool(thread_pool_t* thread_pool);

/**
 * This function allows to obtain the current threadpool status.
 * @param thread_pool
 * @return the threadpool state {THREAD_POOL_STOPPED=0,THREAD_POOL_RUNNING=1,THREAD_POOL_PAUSED=2} if successful
 * @return THREAD_POOL_ERROR if tp is a null reference
 * @note Warning: the status reflects the threadpool situation during the function call.
 * It is not assured that after the call to the function the state remains in that particular situation.
 */
enum thread_pool_state get_thread_pool_state(thread_pool_t* tp);




/***************************************
 * THREAD POOL JOBS
 * ************************************/


/**
 * This function adds the task to the head of the jobs list.
 * @param tp the threadpool on which to add the task
 * @param start_routine the pointer to the function to be performed
 * @param arg the parameters to be passed to the function start_routine
 * @return returns a pointer to the future structure, which has as a payload a pointer to the return value of the function.
 * @return null if tp or start_routine are invalid pointers.
*/
future_t* add_job_head(thread_pool_t* tp,void *(*start_routine)(void*),void *arg);


/**
 * This function adds the task to the tail of the jobs list.
 * @param tp the threadpool on which to add the task
 * @param start_routine the pointer to the function to be performed
 * @param arg the parameters to be passed to the function start_routine
 * @return returns a pointer to the future structure, which has as a payload a pointer to the return value of the function.
 * @return null if tp or start_routine are invalid pointers.
*/
future_t* add_job_tail(thread_pool_t* tp,void *(*start_routine)(void*),void *arg);




#ifdef _cplusplus
}
#endif

#endif



/***************************************
 * USAGE EXAMPLE
 * ************************************/
/*
Small program where a counter is incremented 1000 times via an add function passed to the Thread_pool.

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"threadpool.h"
#define NJOBS 10000
#define NTHREAD 5
#define STR_LENGTH 100
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* add(void* counter) {
    pthread_mutex_lock(&mutex);
    *((int *) counter)=*((int *) counter)+1;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* create_string(void* arg){
    char* string=(char*)malloc(sizeof(char)*STR_LENGTH);
    return strncpy(string,"COUNTER VALUE IS:",STR_LENGTH);
}


int main(){
    int counter=0;
    char* string;
    future_t* add_res[NJOBS]={NULL};
    future_t* future;
    thread_pool_t* tp;

    if(!(tp=create_fixed_size_thread_pool(NTHREAD,NULL)))return EXIT_FAILURE;
    start_thread_pool(tp);
    //adding the tasks to the list
    for(int i=0;i<NJOBS;i++) {
        add_res[i]=add_job_head(tp, add, &counter);
    }
    future=add_job_tail(tp,create_string,NULL);
    //waiting for the termination of the tasks
    for(int i=0;i<NJOBS;i++){
        future_get(add_res[i]);
        destroy_future(add_res[i]);
    }
    string=(char*)future_get(future);
    assert(counter==NJOBS);
    assert(shut_down_thread_pool(tp)==0);
    assert(get_thread_pool_state(tp)==THREAD_POOL_STOPPED);
    destroy_thread_pool(tp);
    printf("%s%d",string,counter);
    destroy_future(future);
    free(string);
    return EXIT_SUCCESS;
}

 */





















