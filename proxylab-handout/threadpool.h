#ifndef THREADPOOL_H 
#define THREADPOOL_H

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_THREAD_NUM 8
struct Tasklist_t{
  struct Tasklist_t * nxt;
  int fd;
};
typedef struct {
  pthread_mutex_t queuelock_;
  sem_t queuestat_;
  void * (*task)(void *);
  struct Tasklist_t * tasklist_front;
  struct Tasklist_t * tasklist_back;
} Threadpool_t;

void ThreadpoolInit(Threadpool_t * threadpool, void * (*task)(void *));
void ThreadpoolAppend(Threadpool_t * threadpool, int fd);
#endif //THREADPOOL_H