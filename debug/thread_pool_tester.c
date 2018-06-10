#include"../threadPool.h"
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* add(void* counter) {
	int* ptr=counter;
	pthread_mutex_lock(&mutex);
	(*ptr)++;
	pthread_mutex_unlock(&mutex);
    return counter ;
}

int main(){
	int counter=0;
	int nthread=4;
	int njobs=1000000;
	int* result;
	future_t res[njobs];

	thread_pool_t tp=create_thread_pool(nthread);
	start_thread_pool(tp);

	for(int i=0;i<njobs;i++)
		//res[i]=add_job_tail(tp,NULL,add,&counter);
		res[i]=add_job_head(tp,NULL,add,&counter);

	for(int i=0;i<njobs;i=i+10000){
		result=(int*)future_get(res[i]);
		printf("%d\n",*result);
	}
	printf("result::%d\n",counter );
	

	for(int i=0;i<njobs;i++){
		destroy_future(res[i]);
	}
	destroy_thread_pool(tp);
	printf("END\n");
}
