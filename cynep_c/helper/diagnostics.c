#pragma once

#include <time.h>



int64 timestamp() {
    struct timeval tv;
    mingw_gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}