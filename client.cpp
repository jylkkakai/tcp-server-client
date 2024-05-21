#include <arpa/inet.h> // inet_addr()
#include <cstdint>
#include <iostream>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()

int main() {
  uint16_t PORT = 5555;
  const int BACKLOG = 5;
  int sockfd, connfd;
  struct sockaddr_in servaddr, cli;

  // socket create and verification
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    std::cout << "Socket creation failed..." << std::endl;
    exit(0);
  } else
    std::cout << "Socket successfully created.." << std::endl;

  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = htons(PORT);

  // connect the client socket to server socket
  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
    std::cout << "connection with the server failed..." << std::endl;
    exit(0);
  } else
    std::cout << "connected to the server.." << std::endl;

  // function for chat
  const int BUFFER_SIZE = 20;
  char buff[BUFFER_SIZE];
  int n;
  while (true) {
    bzero(buff, sizeof(buff));
    std::cout << "Enter the string : ";
    n = 0;
    while ((buff[n++] = getchar()) != '\n')
      ;
    send(sockfd, buff, sizeof(buff), 0);
    bzero(buff, sizeof(buff));
    recv(sockfd, buff, sizeof(buff), 0);
    std::cout << "From Server : " << buff << std::endl;
    if ((strncmp(buff, "exit", 4)) == 0) {
      std::cout << "Client Exit...\n" << std::endl;
      break;
    }
  }
  // close the socket
  close(sockfd);
}
