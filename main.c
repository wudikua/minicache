
#include <stdio.h>
#include <string.h>
#include "mini_cache.h"
int main(int argc, char *argv[])
{
	printf("Hello C-Free!%d\n",sizeof(mini_kv_key));
    mini_allocator_startup(1024*1024*8);

    // mini_cache = shmat(0, NULL, 0);
    mini_cache_stat();
    mini_cache_add("test",strlen("test"),"hello world",strlen("hello world"),60);
    char *data;
    mini_cache_find("test",strlen("test"),&data);
    printf("find value :%s\n", data);
	return 0;
}
