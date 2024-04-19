#ifndef __CACHE_H__
#define __CACHE_H__

#include <string.h>
#include <pthread.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define	MAXLINE	 8192 
#define MAX_CACHE_NUM 10

typedef char string[MAXLINE];
typedef struct {
  string url;
  char content[MAX_OBJECT_SIZE];
  int content_size;
  int timestamp;
}CacheBlock_t;
typedef struct{
  int usednum;
  pthread_rwlock_t rwlock;
  CacheBlock_t cache_block[MAX_CACHE_NUM];
}Cache_t;

void CacheInit(Cache_t * cache);
void CacheDestroy(Cache_t * cache);
int IdInCache(Cache_t * cache, string url);


#endif  //__CACHE_H__