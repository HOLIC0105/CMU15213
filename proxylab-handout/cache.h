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

void CacheInit(Cache_t * cache) {
  pthread_rwlock_init(&cache->rwlock, NULL);
}

void CacheDestroy(Cache_t * cache) {
  pthread_rwlock_destroy(&cache->rwlock);
}

int IdInCache(Cache_t * cache, string url) {
  pthread_rwlock_rdlock(&cache->rwlock);
  int i = MAX_CACHE_NUM - 1;;
  for(; i >= 0; i --) {
    if(!strcasecmp(cache->cache_block[i].url, url)) break;
  }
  pthread_rwlock_unlock(&cache->rwlock);
  return i;
}



#endif  //__CACHE_H__