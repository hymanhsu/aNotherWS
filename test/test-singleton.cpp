//
// Created by xuhuahai on 2017/5/3.
//

#include "ws/CSingleton.h"
#include <stdio.h>



    class Test1
    {
    DEFINE_SINGLETON(Test1)
    public:
        void doit(){
            printf("test1\n");
        }
    };

    Test1::Test1(){
        printf("test1 construct\n");
    }

    Test1::~Test1() {}

    class Test2
    {
    DEFINE_SINGLETON(Test2)
    public:
        void doit(){
            printf("test2\n");
        }
    };

    Test2::Test2(){
        printf("test2 construct\n");
    }

    Test2::~Test2() {}





int main(int argc, char** argv) {

    Test1::instance().doit();
    printf("test1 addr : %x\n",&Test1::instance());

    Test2::instance().doit();
    printf("test2 addr : %x\n",&Test2::instance());

    printf("===================\n");

    Test1::instance().doit();
    printf("test1 addr : %x\n",&Test1::instance());

    Test2::instance().doit();
    printf("test2 addr : %x\n",&Test2::instance());

}

