#include "mini_allocator.h"
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
/**
内存初始化申请size大小的内存
shmget  创建一个新的共享内存区段 
        取得一个共享内存区段的描述符 
shmctl  取得一个共享内存区段的信息 
        为一个共享内存区段设置特定的信息 
        移除一个共享内存区段 
shmat   挂接一个共享内存区段 
shmdt   于一个共享内存区段的分离 
**/
int mini_allocator_startup(unsigned long size) {
    unsigned int allocate_num,i;
    mini_cache_segment *segments;
    int shm_id,ret;  
    //求应该有多少块内存
    allocate_num = size / MINI_SEGMENT_SIZE;
    //在用户空间记录segments的信息，后面赋值给mini_cache_globals后再删除这个变量
    segments = (mini_cache_segment *)calloc(1, allocate_num * sizeof(mini_cache_segment));
    //为每块分配内存
    for (i=0; i<allocate_num; i++) {
        //创建共享内存
        shm_id = shmget(IPC_PRIVATE, MINI_SEGMENT_SIZE, 0666|IPC_CREAT );  
        //给每一块内存赋值
        segments[i].shm_id = shm_id;
        segments[i].pos = 0;
        segments[i].size = MINI_SEGMENT_SIZE;
        //映射到用户内存空间后的地址
        segments[i].p = shmat(shm_id, NULL, 0);
    }
    //把内存块的第一块作为全局对象的存储空间
    mini_cache = segments[0].p;
    //segments得首地址应该是从全局对象之后开始的
    mini_cache->segments = (mini_cache_segment **)((char *)mini_cache + sizeof(mini_cache_globals));
    //因为第一块让mini_cache_globals占了，所以是减一个
    mini_cache->segments_num = allocate_num - 1;
     //segments_mask
    mini_cache->segments_mask =  MINI(segments_num) - 1;
    //将在用户内存空间的segments管理对象复制到共享内存上
    char *p = (char *)MINI(segments);
    memcpy(p, (void *)segments,  sizeof(mini_cache_segment) * (mini_cache->segments_num));
    //给共享内存上的segments管理对象赋值
    for (i=0; i<mini_cache->segments_num; i++) {
        mini_cache->segments[i] = p;
        p += sizeof(mini_cache_segment);
    }
    //把分配完全局变量和segments到第一块区域结束的区域作为slots的存储空间
    mini_cache->slots = (mini_kv_key *)((char *)mini_cache->segments + (mini_cache->segments_num * sizeof(void *))
        + sizeof(mini_cache_segment) * (mini_cache->segments_num));
    //设置slots的数量
    mini_cache->slots_num = (MINI_SEGMENT_SIZE - ((char *)mini_cache->slots - (char *)mini_cache))/sizeof(mini_kv_key);
    //设置slots_mask
    mini_cache->slots_mask = mini_cache->slots_num - 1;
    //在共享区域分配好全局对象以后，释放用户空间申请的segments临时对象
    free(segments);
    return 0;
}

//返回一块共享内存区域
void *mini_allocator_alloc(unsigned long size, int hash) {
    mini_cache_segment *segment;
    unsigned int current,i;
    //计算segments的槽位
    current = hash & MINI(segments_mask);
    //取得映射到的segments
    segment = mini_cache->segments[current];
    //判断是否这块内存区域的剩余空间够分配
    if (segment->size - segment->pos >= size) {
        //增加pos的地址
        segment->pos += size;
        //返回分配的地址
        return (void *)(segment->p + segment->pos - size);
    } else {
        //当前映射到的segment剩余的空间不够size大小
        for (i=0; i<3; i++) {
            //换一块segment
            segment = mini_cache->segments[(current + i) & mini_cache->segments_mask];
            if (segment->size - segment->pos >= size) {
                //增加pos的地址
                segment->pos += size;
                //返回分配的地址
                return (void *)(segment->p + segment->pos - size);
            }
        }
    }
    //换了几块segment还是没找到够分配的块,就把之前映射的块pos=0,在这块上写
    segment = mini_cache->segments[current];
    segment->pos = size;
    return (void *)segment->p;
}