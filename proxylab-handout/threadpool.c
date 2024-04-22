#include "threadpool.h"
#include <unistd.h>
static void * ThreadDo(void * tmp_threadpool) {
  Threadpool_t * threadpool = (Threadpool_t *)(tmp_threadpool);
  while(1) {
    sem_wait(&threadpool->queuestat_);
    pthread_mutex_lock(&threadpool->queuelock_);
    struct Tasklist_t * tmp = threadpool->tasklist_front;
    int fd = tmp->fd;
    threadpool->tasklist_front = threadpool->tasklist_front->nxt;
    pthread_mutex_unlock(&threadpool->queuelock_);
    free(tmp);
    threadpool->task(&fd);
  }
  return NULL;
}

void ThreadpoolInit(Threadpool_t * threadpool, void * (*task)(void *)) {
  sem_init(&threadpool->queuestat_, 0, 0);
  pthread_t tpid;
  pthread_mutex_init(&threadpool->queuelock_, NULL);
  threadpool->task = task;
  threadpool->tasklist_front = NULL;
  for(int i = 0; i < MAX_THREAD_NUM; i ++) {
    if(pthread_create(&tpid, NULL, ThreadDo, threadpool) == -1) {
      fprintf(stderr, "Pthread %d dose not create\n", i);
      continue;
    } else printf("Pthread %d success create\n", i);
  }
}
void ThreadpoolAppend(Threadpool_t * threadpool, int fd) {
  pthread_mutex_lock(&threadpool->queuelock_);
  sem_post(&threadpool->queuestat_);
  if(threadpool->tasklist_front == NULL) {
    threadpool->tasklist_front = (struct Tasklist_t *)malloc(sizeof(struct Tasklist_t));
    threadpool->tasklist_back = threadpool->tasklist_front;
    *threadpool->tasklist_front = (struct Tasklist_t){NULL, fd}; 
  } else {
    threadpool->tasklist_back = 
    threadpool->tasklist_back->nxt = 
      (struct Tasklist_t *)malloc(sizeof(struct Tasklist_t));
    *threadpool->tasklist_back = (struct Tasklist_t){NULL, fd};  
  }
  pthread_mutex_unlock(&threadpool->queuelock_);
}