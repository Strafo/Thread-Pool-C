//
// Created by strafo on 14/01/19.
//
#include <stdio.h>
#include <assert.h>




int create_fixed_size_thread_pool_test(){


}


int start_thread_pool_test(){

}

int pause_thread_pool_test(){

}

int shut_down_now_thread_pool_test(){

}

int shut_down_thread_pool_test(){


}

int destroy_thread_pool_test(){

}


int destroy_future_test(){

}

int is_ready_test(){

}

int future_get_test(){

}


int add_job_head_test(){

}

int add_job_tail_test(){

}



int main(){

    int passed_test=0;
    const int tot_test=11;


    if(!create_fixed_size_thread_pool_test()){passed_test++;}
    if(!start_thread_pool_test()){passed_test++;}
    if(!pause_thread_pool_test()){passed_test++;}
    if(!shut_down_now_thread_pool_test()){passed_test++;}
    if(!shut_down_thread_pool_test()){passed_test++;}
    if(!destroy_thread_pool_test()){passed_test++;}
    if(!destroy_future_test()){passed_test++;}
    if(!is_ready_test()){passed_test++;}
    if(!future_get_test()){passed_test++;}
    if(!add_job_head_test()){passed_test++;}
    if(!add_job_tail_test()){passed_test++;}



    if(passed_test!=tot_test) {
        printf( "[%f] tests passed!\n",(float)(passed_test/tot_test));
    }

}


