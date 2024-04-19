#include "cache.h"

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
