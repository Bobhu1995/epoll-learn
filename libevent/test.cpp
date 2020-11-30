// File Name: test.cpp
// Author: bob
// Created Time: Fri 29 May 2020 11:06:40 AM CST

#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <cstdlib>
#include <event2/event-config.h>
#include <event.h>
#include <sys/types.h>

using namespace std;
struct event ev;
struct timeval tv;

void time_cb(int fd, short event, void *argc){
  printf("timer wakeup!\n");
  event_add(&ev, &tv);
}

int main(){
  struct event_base *base = event_init();
  tv.tv_sec = 10;
  tv.tv_usec = 0;
  evtimer_set(&ev, time_cb, NULL);
  event_base_set(base, &ev);
  event_add(&ev, &tv);
  event_base_dispatch(base);

  return 0;
}
