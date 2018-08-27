#include"../threadpool.h"
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>



void* add(void* counter) {
    int *value=(int*)malloc(sizeof(int));
    if(value==NULL){
        perror("ERRORE");
    }

    *value=*((int *) counter);
    *((int *) counter)=*((int *) counter)+1;
    return value ;
}

int main(){
    int counter=0;
    int nthread=4;
    int njobs=1000000;
    int* result;
    future_t* res[njobs];

    thread_pool_t* tp=create_fixed_size_thread_pool(nthread,NULL);
    start_thread_pool(tp);

    for(int i=0;i<njobs;i++)
        //res[i]=add_job_tail(tp,add,&counter);
        res[i]=add_job_head(tp,add,&counter);

    for(int i=0;i<njobs;i=i+10000){
        result=(int*)future_get(res[i]);
        printf("%d\n",*result);
        destroy_future(res[i]);
        free(result);
    }


    shut_down_thread_pool(tp);
    destroy_thread_pool(tp);
    printf("final result::%d\n",counter );
    printf("END\n");
}
