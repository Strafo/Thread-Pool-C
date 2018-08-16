/**
 * @file threadpool.h
 * @author Andrea Straforini
 * @date 16/06/2018
 * @brief Simple Thread Pool
 * @note
 */



#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include"debug/debug.h"
#include"libhl/linklist.h"
#include<errno.h>
#include<math.h>

typedef struct _threadPool thread_pool_t;
typedef struct _future future_t;

thread_pool_t *create_fixed_size_thread_pool(int initial_size);
//todo thread_pool_t *create_cached_size_thread_pool(int initial_size)

void start_thread_pool(thread_pool_t *tp, const pthread_attr_t *attr);

future_t *add_job_head(thread_pool_t *tp, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

future_t *add_job_tail(thread_pool_t *tp, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

int is_ready(future_t *future);

void *future_get(future_t *future);

//Testing necessary
//void destroy_now_thread_pool(thread_pool_t* threadPool);


void destroy_thread_pool(thread_pool_t *threadPool);

void destroy_future(future_t *future);

#ifdef _cplusplus
}
#endif

#endif