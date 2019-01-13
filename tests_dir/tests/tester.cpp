//
// Created by strafo on 04/12/18.
//
#include"gtest/gtest.h"
#include<threadpool.h>



class ThreadPoolTest: public ::testing::Test{
    thread_pool_t* tp;
    protected:
    public:
        ThreadPoolTest(int _tp_size,pthread_attr_t *_attr): Test(){
            tp=create_fixed_size_thread_pool(_tp_size,_attr);
        }
        virtual ~ThreadPoolTest(){
            destroy_thread_pool(tp);
            delete tp;
        }

};

//https://meekrosoft.wordpress.com/2009/11/09/unit-testing-c-code-with-the-googletest-framework/
//https://github.com/anastasiak2512/Calendar/blob/master/calendars_tests/basic_tests/calendar_check.cpp



