/**
 * @file threadpool.h
 * @author Andrea Straforini
 * @date 16/06/2018
 * @brief Simple Thread Pool
 * @note
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H
#define THREAD_SAFE
#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include<stdlib.h>
#include"libhl/linklist.h"
#include<errno.h>
#include<pthread.h>
#include"libhl/atomic_defs.h"

struct _thread_pool;
struct _future;
struct _job;
typedef struct _thread_pool thread_pool_t;
typedef struct _future future_t;



/************************************************************************
 *  API
 ************************************************************************/


/**
 *
 * @param size
 * @param attr
 * @return
 */
thread_pool_t* create_fixed_size_thread_pool(int size,const pthread_attr_t *attr);


//todo thread_pool_t *create_cached_size_thread_pool(int initial_size)


/**
 *
 * @param tp
 * @return
 */
int start_thread_pool(thread_pool_t* tp);


/**
 *
 * @param tp
 * @return
 */
int pause_thread_pool(thread_pool_t* tp);


/**
 *
 * @param thread_pool
 */
int shut_down_now_thread_pool(thread_pool_t* thread_pool);


/**
 *
 * @param thread_pool
 */
int shut_down_thread_pool(thread_pool_t* thread_pool);


/**
 *
 * @param thread_pool
 */
void destroy_thread_pool(thread_pool_t* thread_pool);


/**
 *
 * @param future
 */
void destroy_future(future_t* future );


/**
 *
 * @param future
 * @return
 */
int is_ready(future_t* future);


/**
 *
 * @param future
 * @return
 */
void* future_get(future_t* future);

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