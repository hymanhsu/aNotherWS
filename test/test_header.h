//
// Created by xuhuahai on 2017/4/28.
//
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "ws/common.h"

#ifndef UVTEST_TEST_HEADER_H
#define UVTEST_TEST_HEADER_H



void printx(const void* _data, uint len) {
    uchar * data = (uchar *)_data;
    uint i;
    printf("\n-----------------------------------------------\n");
    for (i = 0; i < len; i++) {
        if (i > 0 && i % 16 == 0)
            printf("\n");
        printf("%02x ", *data);
        data++;
    }
    printf("\n-----------------------------------------------\n");
}



#endif //UVTEST_TEST_HEADER_H
