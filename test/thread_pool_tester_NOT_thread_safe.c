#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include "../src/threadpool.h"


void* add(void* counter) {
    int *value=(int*)malloc(sizeof(int));
    if(value==NULL){
        perror("ERRORE");
    }

    *value=*((int *) counter);
    *((int *) counter)=*((int *) counter)+1;
    return value ;
}

int thread_pool_tester_NOT_thread_safe(){
    int counter=0;
    int nthread=4;
    int njobs=1000000;
    int* result;
    future_t* res[njobs];

    thread_pool_t* tp=create_fixed_size_thread_pool(nthread);
    start_thread_pool(tp);

    for(int i=0;i<njobs;i++)
        //res[i]=add_job_tail(tp,add,&counter);
        res[i]=add_job_head(tp,add,&counter);

    for(int i=0;i<njobs;i++){
        result=(int*) get_future(res[i]);
        printf("%d\n",*result);
        destroy_future(res[i]);
        free(result);
    }

    printf("final result::%d\n",counter );
    printf("END\n");
    shut_down_thread_pool(tp);
    destroy_thread_pool(tp);
    return 0;
}

int main(){
    return thread_pool_tester_NOT_thread_safe();
}