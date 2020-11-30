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
#include <string>
#include <iostream>
using namespace std;

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
  
  if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    err_exit("connect");

  return sockfd;
}



int main(int argc, const char *argv[]){
  if(argc < 3){
    printf("usage:%s ip port\n", argv[0]);
    exit(0);
  }

  const char *ip = argv[1];
  const int port = atoi(argv[2]);

  int sock = create_socket(ip, port);
  printf("success create_socket %d \n", sock);

  while(1){
    string buff;
    getline(cin, buff);
    printf("cin buff = %s\n", buff.c_str());
    if(buff == "exit") break;
    printf("sock = %d, buff=%s\n", sock, buff.c_str());
    write(sock, buff.c_str(), buff.size());
    char end ='\n';
    write(sock, &end, 1);
    char recvBuff[1024];
    int recvLen = recv(sock, recvBuff, 1024, 0);
    if(recvLen < 0){
      printf("revn message fail\n");
    }else{
      printf("recv message is %s\n", recvBuff);
    }
  }
  close(sock);
  return 0;
}
