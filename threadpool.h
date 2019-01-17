/**
 * @file threadpool.h
 * @author Andrea Straforini
 * @date 16/06/2018
 * @brief Simple Thread Pool
 * @note
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H
#define THREAD_SAFE   //todo è già dichiarato in linklist.h rimuovere?
#ifdef __cplusplus
extern "C" {
#endif

#include<stdlib.h>
#include"linklist.h"
#include<errno.h>
#include<pthread.h>
#include"atomic_defs.h"

struct _thread_pool;
struct _future;
struct _job;
typedef struct _thread_pool thread_pool_t;
typedef struct _future future_t;



/************************************************************************
 *  API
 ************************************************************************/




/***************************************
 * FUTURE
 * ************************************/

/**
 * Possible states in which the future can be.
 * To get the status of the future, simply call the function: get_future_state
 */
enum future_state{
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
 * @return future_state
 */
enum future_state get_future_state(future_t* future);


/**
 * This function allows to obtain the encapsulated content in the future.
 * Return the pointer to the payload of the future.
 * the operation BLOCKS the execution of the current thread until
 * the state of the object turns out to be of type: FUTURE_READY
 * @return the pointer to the payload.
 */
void* future_get(future_t* future);




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
 * @return the  thread_pool's pointer
 * @return NUll if size<=0
 */
thread_pool_t* create_fixed_size_thread_pool(int size,const pthread_attr_t *attr);


//todo thread_pool_t *create_cached_size_thread_pool(int initial_size)


/**
 *
 * @param thread_pool
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
 *
 * @param tp
 * @return 0 if successful, -1 if tp is a null reference
 */
int start_thread_pool(thread_pool_t* tp);


/**
 *
 * @param tp
 * @return 0 if successful, -1 if tp is a null reference
 */
int pause_thread_pool(thread_pool_t* tp);


/**
 *
 * @param thread_pool
 */
int shut_down_now_thread_pool(thread_pool_t* thread_pool);//todo


/**
 *
 * @param thread_pool
 * @return 0 if successful, -1 if tp is a null reference
 */
int shut_down_thread_pool(thread_pool_t* thread_pool);

/**
 * @param thread_pool
 * @return the threadpool state {THREAD_POOL_STOPPED=0,THREAD_POOL_RUNNING=1,THREAD_POOL_PAUSED=2} if successful
 * @return THREAD_POOL_ERROR if tp is a null reference
 */
enum thread_pool_state get_thread_pool_state(thread_pool_t* tp);






/***************************************
 * THREAD POOL JOBS
 * ************************************/

/**
 *
 * @param tp
 * @param start_routine
 * @param arg
 * @return
 */
future_t* add_job_head(thread_pool_t* tp,void *(*start_routine)(void*),void *arg);

/**
 *
 * @param tp
 * @param start_routine
 * @param arg
 * @return
 */
future_t* add_job_tail(thread_pool_t* tp,void *(*start_routine)(void*),void *arg);




#ifdef _cplusplus
}
#endif

#endif