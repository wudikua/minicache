//定义MINI_CACHE_H 防止重复引用头文件 
#ifndef MINI_CACHE_H
#define MINI_CACHE_H

//KEY的最大长度 
#define KEY_MAX_LEN 32  
#define USER_ALLOC	malloc
#define USER_FREE	free

//value结构体 
typedef struct {
	//value的长度 
	unsigned int len;
	//value对应的数据地址;
	char *data; 
} mini_kv_val;

//key结构体
typedef struct {
	//key的hash值 
	unsigned long hash;
	//key所对应value的crc校验码 
	unsigned long crc;
	//key的长度 
	unsigned int len;
	//key的过期时间
	unsigned int ttl;  
	//key对应的value
	mini_kv_val *val; 
	//key所对应的key字符串
	unsigned char key[KEY_MAX_LEN]; 
} mini_kv_key;

//用于存储value的一块内存区域 
typedef struct {
	//pos位置之后的内存区域都还没分配 
	unsigned int pos;
	//这块内存区域的大小 
	unsigned int size;
	//这块内存的首地址
	void *p;
	//这块内存的共享内存shmid
	int shm_id;		
} mini_cache_segment; 

//全局对象，管理了key和value对应的存储区域 
typedef struct {
	//mini_kv_key的数组slots,存储了所有的key 
	mini_kv_key *slots;
	//mask用于映射到slots数组 
	unsigned int slots_mask;
	//slots的数量
	unsigned int slots_num;
	//内存块数组 
	mini_cache_segment **segments;	
	//内存块的数量 
	unsigned int segments_num;
	//mask用于映射到segments数组 
	unsigned int segments_mask;
} mini_cache_globals;

//声明一个全局变量mini_cache 
extern mini_cache_globals *mini_cache;

//定义一个操作mini_cache的宏 
#define MINI(element) (mini_cache->element);

//初始化cache 
int mini_cache_startup(unsigned long size, char	**err);

//添加一个key 
int mini_cache_add(char *key, unsigned int len, char *data, unsigned int size, int ttl);

//更新一个key
int mini_cache_update(char *key, unsigned int len, char *data, unsigned int size, int ttl);

//查找一个key 
int mini_cache_find(char *key, unsigned int len, char **data);

//删除一个key
int mini_cache_del(char *key, unsigned int len); 

#endif
