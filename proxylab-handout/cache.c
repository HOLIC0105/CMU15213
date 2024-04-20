#include "cache.h"

void CacheInit(Cache_t * cache) {
  pthread_rwlock_init(&cache->rwlock, NULL);
  pthread_mutex_init(&cache->timestamp_mutex, NULL);
  for(int i = 0; i < MAX_CACHE_NUM; i ++)
    pthread_mutex_init(&cache->cache_block[i].timestamp_mutex, NULL);
}

void CacheDestroy(Cache_t * cache) {
  pthread_rwlock_destroy(&cache->rwlock);
  pthread_mutex_destroy(&cache->timestamp_mutex);
  for(int i = 0; i < MAX_CACHE_NUM; i ++)
    pthread_mutex_destroy(&cache->cache_block[i].timestamp_mutex);
}

static void SendContent(CacheBlock_t * src, int client_fd) {
  if(rio_writen(client_fd, src->content, src->content_size) != src->content_size) {
    fprintf(stderr, "Send response to client error\n");
  }
}

int UrlInCache(Cache_t * cache, string url, int client_fd) {
  pthread_rwlock_rdlock(&cache->rwlock);
  int i = MAX_CACHE_NUM - 1;
  for(; i >= 0; i --) {
    if(!strcasecmp(cache->cache_block[i].url, url)) {
      SendContent(&cache->cache_block[i], client_fd);
      pthread_mutex_lock(&cache->cache_block[i].timestamp_mutex);
      pthread_mutex_lock(&cache->timestamp_mutex);
      cache->cache_block[i].timestamp = ++ cache->timestamp;
      pthread_mutex_unlock(&cache->timestamp_mutex);
      pthread_mutex_unlock(&cache->cache_block[i].timestamp_mutex);
      break;
    }
  }
  pthread_rwlock_unlock(&cache->rwlock);
  return i;
}

void InsertCache(Cache_t * cache, string url, char * src) {
  pthread_rwlock_wrlock(&cache->rwlock);
  int id = 0;
  for(int i = 1; i < MAX_CACHE_NUM; i ++) {
    if(cache->cache_block[i].timestamp < cache->cache_block[id].timestamp) {
      id = i;
    }
  }
  memcpy(cache->cache_block[id].content, src,  
        cache->cache_block[id].content_size = strlen(src));
  memcpy(cache->cache_block[id].url, url, strlen(url));
  cache->cache_block[id].timestamp = ++ cache->timestamp;
  pthread_rwlock_unlock(&cache->rwlock);
}
