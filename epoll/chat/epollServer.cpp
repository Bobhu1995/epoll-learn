// File Name: epollServer.cpp
// Author: bob
// Created Time: Fri 13 Mar 2020 02:26:25 PM CST
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <vector>
using namespace std;

const int MAX_EPOLL_EVENTS = 1000;
const int MAX_MSG_LEN = 1024;

void setFdNonblock(int fd){
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void err_exit(const char *s){
  printf("error: %s\n", s);
  exit(0);
}

int create_socket(const char *ip, const int port_number){
  struct sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_number);
  if(inet_pton(server_addr.sin_family, ip, &server_addr.sin_addr) == -1){
    err_exit("inet_pton");
  }

  int sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if(sockfd == -1)
    err_exit("socket");
  int reuse = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    err_exit("setsockopt");
  
  if(bind(sockfd, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    err_exit("bind");
  if(listen(sockfd, 5) == -1)
    err_exit("listen");

  return sockfd;
}



int main(int argc, const char *argv[]){
  if(argc < 3){
    printf("usage:%s ip port\n", argv[0]);
    exit(0);
  }

  const char *ip = argv[1];
  const int port = atoi(argv[2]);

  int sockfd = create_socket(ip, port);
  printf("success create_socket fd = %d \n", sockfd);
  setFdNonblock(sockfd);

  int epollfd = epoll_create1(0);
  if(epollfd == -1) err_exit("epoll_create1");
  
  struct epoll_event ev;
  ev.data.fd = epollfd;
  ev.events = EPOLLIN|EPOLLET;
  if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1)
    err_exit("epoll_ctl");
  
  struct epoll_event events[MAX_EPOLL_EVENTS] = {};
  vector<int> users;
  while(1){
   printf("begin wait\n");
   int number = epoll_wait(epollfd, events, MAX_EPOLL_EVENTS, -1);
   printf("end wait, number = %d\n", number);
   //sleep(10);
   if(number > 0){
     for(int i = 0; i < number; i++){
       int eventfd = events[i].data.fd;

       if(eventfd == epollfd){
         printf("accept new client...\n");
         struct sockaddr_in client_addr;
         socklen_t client_addr_len = sizeof(client_addr);
         int connfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
         setFdNonblock(connfd);

         struct epoll_event ev;
         ev.data.fd = connfd;
         ev.events = EPOLLIN|EPOLLOUT|EPOLLET;
         if(epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) == -1){
           err_exit("epoll_ctl2");
         }
         users.push_back(connfd);
         printf("accept new client fd = %d users = %d end.\n", connfd, (int)users.size());
       }else{
         printf("read start... eventfd = %d \n", eventfd);
         while(1){
           if(events[i].events & (EPOLLIN | EPOLLOUT)){
             if(EPOLLIN){
                printf("epollin--------------- eventfd = %d\n", eventfd);
                char recvBuff[1024];
                int ret = read(eventfd, recvBuff, 1024);                          
                printf("read ret = %d\n", ret);
                if(ret > 0){
                   printf("read content = %s", recvBuff);
                   for(int j = 0; j < (int)users.size(); ++j){
                     if(users[j] != eventfd){
                        int sendLen = send(users[j], recvBuff, ret, 0);
                        if(sendLen > 0){
                          printf("send message from %d to %d, content = %s", eventfd, users[j], recvBuff);
                        }else{
                          printf("send massage fail");
                        }
                     }
                   }
                }
                else if(ret == 0){
                  printf("client close.\n");
                  close(eventfd);
                  epoll_ctl(epollfd, EPOLL_CTL_DEL, eventfd, NULL);
                  break;
                }
                else{
                  break;
                }
             }else if(EPOLLOUT){
                  printf("epollout--------------- eventfd = %d", eventfd);
                  char sendBuff[1024];
                  cout << "please cin your message:";
                  scanf("%s", sendBuff);
                  printf("your input is %s\n", sendBuff);
                  int sendLen = send(eventfd, sendBuff, strlen(sendBuff) + 1, 0);
                  if(sendLen < 0){
                    printf("send message fail\n");
                    break;
                  }
                  
             } 
           }
           /**
           char recvBuff[1024];
           char sendBuff[1024];
           int ret = read(eventfd, recvBuff, 1024);
           printf("read ret = %d\n", ret);
           if(ret > 0){
             printf("read content = %s", recvBuff);
             cout << "please cin your message:";
             //cin >> sendBuff;
             scanf("%s", sendBuff);
             printf("your input is %s\n", sendBuff);
             int sendLen = send(eventfd, sendBuff, strlen(sendBuff) + 1, 0);
             if(sendLen < 0){
                printf("send message fail\n");
                break;
             }
           }
           else if(ret == 0){
             printf("client close.\n");
             close(eventfd);
             epoll_ctl(epollfd, EPOLL_CTL_DEL, eventfd, NULL);
             break;
           }else if(ret < 0){
             break;
           }
           **/
         }
         
         printf("read end.\n");
       }
     }
   }
  }

  return 0;
}
