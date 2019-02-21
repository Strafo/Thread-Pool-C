#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <assert.h>
#include"threadpool.h"
#define NJOBS 1000000
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* add(void* counter) {
    int *value=(int*)malloc(sizeof(int));
    if(value==NULL){
        perror("ERRORE");
		return NULL;
    }
	pthread_mutex_lock(&mutex);
	    *value=*((int *) counter);
        *((int *) counter)=*((int *) counter)+1;
        //printf("thread id:%lu\n",pthread_self());fflush(stdout);
	pthread_mutex_unlock(&mutex);
	return value ;
}

int thread_pool_tester_thread_safe(){
	int counter=0;
	char comando;
	int nthread=5;
	int* result=NULL;
	future_t* res[NJOBS]={NULL};
	thread_pool_t* tp=NULL;

	tp=create_fixed_size_thread_pool(nthread,NULL);
	if(tp==NULL)return 1;
	start_thread_pool(tp);

	for(int i=0;i<NJOBS;i++) {
		res[i] = add_job_head(tp, add, &counter);
	}
	for(int i=0;i<NJOBS;i++){
		result=(int*)future_get(res[i]);
		if(*result%1000==0&&*result>=1000){//serve per non stampare tutti gli output
		    printf("%d\n",*result);
		}
		destroy_future(res[i]);
		free(result);
		if(i==5000){
			printf("Stopping tp...\n");
			pause_thread_pool(tp);
			assert(get_thread_pool_state(tp)==THREAD_POOL_PAUSED);
			sleep(1);
			printf("Vuoi ripartire?\n");
			scanf("%c",&comando);
		}
	}

	printf("FINAL result::%d\n",counter );
	printf("END\n");
    assert(counter==1000000);
    assert(shut_down_thread_pool(tp)==0);
    assert(get_thread_pool_state(tp)==THREAD_POOL_STOPPED);
	destroy_thread_pool(tp);
	return 0;
}


int main(){
	return thread_pool_tester_thread_safe();
}