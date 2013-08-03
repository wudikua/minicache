#ifndef MINI_ALLOCATOR_H
#define MINI_ALLOCATOR_H
#include "mini_cache.h"

//每一快内存的大小
#define MINI_SEGMENT_SIZE (1024*1024*1)

//内存初始化申请size大小的内存
int mini_allocator_startup(unsigned long size);

//返回一块共享内存区域
void *mini_allocator_alloc(unsigned long size, int hash);

#endif