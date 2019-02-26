//
// Created by strafo on 14/01/19.
//
#include<errno.h>
#include "minunit.h"
#include "threadpool.h"

thread_pool_t* threadPool;
int res,i,size;
enum thread_pool_state expected_state;
future_t* future;
int expected_null;
void *(*start_routine)(void*);
void *arg;


int counter;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
void* add_fun(void* counter) {
    pthread_mutex_lock(&counter_mutex);
    *((int *) counter)=*((int *) counter)+1;
    pthread_mutex_unlock(&counter_mutex);
    return NULL;
}


void init_basic_test_suite(){
    errno=0;
    counter=0;
    start_routine=add_fun;
    expected_null=1;
    arg=&counter;
    size=2;
    threadPool=create_fixed_size_thread_pool(size);
    if(size<=0) {
        mu_check(threadPool == NULL);
    }
    if(errno!=0) {
        mu_fail("init_basic_test_suite malloc fail");
    }

}

void teardown_basic_test_suite(){
    destroy_thread_pool(threadPool);

}

MU_TEST(test_start_thread_pool){
    res=start_thread_pool(threadPool);
    mu_check((res==0&&threadPool!=NULL)||(res<0&&threadPool==NULL));
}

MU_TEST(test_pause_thread_pool){
    res=pause_thread_pool(threadPool);
    mu_check((res==0&&threadPool!=NULL)||(res<0&&threadPool==NULL));
}

MU_TEST(test_shut_down_thread_pool){
    res=shut_down_thread_pool(threadPool);
    mu_check((res==0&&threadPool!=NULL)||(res<0&&threadPool==NULL));
}

MU_TEST(test_shut_down_now_thread_pool){
    res=shut_down_now_thread_pool(threadPool);
    mu_check((res==0&&threadPool!=NULL)||(res<0&&threadPool==NULL));
}

MU_TEST(test_get_thread_pool_state){
    enum thread_pool_state state=get_thread_pool_state(threadPool);
    mu_check((state==THREAD_POOL_ERROR&&threadPool==NULL)||(threadPool!=NULL&&state!=THREAD_POOL_ERROR));
    mu_assert_int_eq(expected_state,state);
}

MU_TEST(test_destroy_future){
    destroy_future(future);
}

MU_TEST(test_get_future){//expected_null==1 if future result should be null
    void* result=get_future(future);
    mu_check((future==NULL&&result==NULL)||(expected_null&&result==NULL&&future!=NULL)||(!expected_null&&result!=NULL&&future!=NULL));

}

MU_TEST(test_get_future_state){
    enum future_state future_state=get_future_state(future);
    mu_check((future_state==FUTURE_ERROR&&future==NULL)||(future_state!=FUTURE_ERROR&&future!=NULL));
}

MU_TEST(test_add_job_head){
    errno=0;
    future_t *res=add_job_head(threadPool,start_routine,arg);
    if(!errno)
        mu_check((res==NULL&&(!threadPool||!start_routine))||(res!=NULL&&(threadPool!=NULL||start_routine!=NULL)));
    else
        mu_fail("test_add_job_head malloc fail");
}

MU_TEST(test_add_job_tail){
    errno=0;
    future_t *res=add_job_tail(threadPool,start_routine,arg);
    if(!errno)
        mu_check((res==NULL&&(!threadPool||!start_routine))||(res!=NULL&&(threadPool!=NULL||start_routine!=NULL)));
    else
        mu_fail("test_add_job_tail malloc fail");

}

MU_TEST(check_i_value) {
    mu_assert_int_eq(i,counter);
}

MU_TEST_SUITE(thread_pool_basic_test_suite) {
    i=0;
    init_basic_test_suite();

    expected_state=THREAD_POOL_PAUSED;MU_RUN_TEST(test_get_thread_pool_state);

    MU_RUN_TEST(test_start_thread_pool);
    for(i=0;i<2000;i++){
        MU_RUN_TEST(test_add_job_head);
    }

    MU_RUN_TEST(test_start_thread_pool);
    for(;i<40000;i++){
        expected_state=THREAD_POOL_RUNNING;MU_RUN_TEST(test_get_thread_pool_state);
        MU_RUN_TEST(test_add_job_head);
    }

    MU_RUN_TEST(test_pause_thread_pool);expected_state=THREAD_POOL_PAUSED;MU_RUN_TEST(test_get_thread_pool_state);

    MU_RUN_TEST(test_start_thread_pool);expected_state=THREAD_POOL_RUNNING;MU_RUN_TEST(test_get_thread_pool_state);

    future=add_job_tail(threadPool,start_routine,arg);i++;
    MU_RUN_TEST(test_get_future);

    MU_RUN_TEST(test_shut_down_thread_pool);expected_state=THREAD_POOL_STOPPED;MU_RUN_TEST(test_get_thread_pool_state);

    MU_RUN_TEST(check_i_value);

    MU_RUN_TEST(test_destroy_future);

    teardown_basic_test_suite();
}

int main(){
    MU_RUN_SUITE(thread_pool_basic_test_suite);
    MU_REPORT();
    return minunit_status;

}

