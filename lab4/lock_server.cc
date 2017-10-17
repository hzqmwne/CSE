// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static std::map<lock_protocol::lockid_t, lock_protocol::rpc_numbers> locks;

lock_server::lock_server():
  nacquire (0)
{
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  r = nacquire;
  return ret;
}

lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
	// Your lab4 code goes here
  pthread_mutex_lock(&mtx);
  while(locks.find(lid) != locks.end() && locks[lid] == lock_protocol::acquire) {
    printf("==debug wait %llu\n",lid);
    pthread_cond_wait(&cond, &mtx); 
  }
  locks[lid] = lock_protocol::acquire;
  ++(this->nacquire);
  pthread_mutex_unlock(&mtx);
  printf("==acquire %llu\n", lid);
  return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
	// Your lab4 code goes here
  pthread_mutex_lock(&mtx);
  locks[lid] = lock_protocol::release;
  --(this->nacquire);
  printf("==debug wake up %llu\n",lid);
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mtx);
  printf("===debug release %llu\n", lid);
  return ret;
}
