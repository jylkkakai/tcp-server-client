#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <arpa/inet.h>
// #include <fcntl.h>    //fcntl() function and defines
// #include <sys/time.h> //Needed for timeval struct
#include <unistd.h>

int main(int argc, char *argv[]) {

  const std::string PORT = "5555";
  const int BACKLOG = 5;

  int listener_fd;
  struct addrinfo hints;
  struct addrinfo *server_info_list;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC;     // Supports IPv4 and IPv6
  hints.ai_socktype = SOCK_STREAM; // Reliable Stream (TCP)
  hints.ai_flags = AI_PASSIVE;     // Assign local host address to socket

  // Get address information
  int err;
  err = getaddrinfo(NULL, PORT.c_str(), &hints, &server_info_list);
  if (err != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(err) << std::endl;
    return 1;
  }

  // Go over list and try to create socket and bind
  addrinfo *p;
  for (p = server_info_list; p != NULL; p = p->ai_next) {

    // Create the socket - system call that returns the file descriptor of the
    // socket
    listener_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener_fd == -1) {
      continue; // try next
    }

    // Make sure the port is not in use. Allows reuse of local address (and
    // port)
    int yes = 1;
    if (setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
        -1) {
      perror("setsockopt");
      return 1;
    }

    // Bind socket to specific port (p->ai_addr holds the address and port
    // information)
    if (bind(listener_fd, p->ai_addr, p->ai_addrlen) == -1) {
      close(listener_fd);
      continue; // try next
    }

    break; // success
  }

  // No one from the list succeeded - failed to bind
  if (p == NULL) {
    std::cerr << "failed to bind" << std::endl;
    return 1;
  }

  freeaddrinfo(server_info_list);

  if (listen(listener_fd, BACKLOG) == -1) {
    perror("listen");
    return 1;
  }
  int client_fd;
  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;

  // Accept the incoming connection, save the socket descriptor (client_fd)
  client_fd = accept(listener_fd, (struct sockaddr *)&their_addr, &addr_size);
  if (client_fd == -1) {
    perror("accept");
  } else { // If connection accepted

    // Print incoming connection
    if (their_addr.ss_family == AF_INET) {
      // IPv4
      char ip_as_string[INET_ADDRSTRLEN];
      inet_ntop(their_addr.ss_family,
                &((struct sockaddr_in *)&their_addr)->sin_addr, ip_as_string,
                INET_ADDRSTRLEN);
      std::cout << "New connection from " << ip_as_string << " on socket "
                << client_fd << std::endl;
    } else if (their_addr.ss_family == AF_INET6) {
      // IPv6
      char ip_as_string[INET6_ADDRSTRLEN];
      inet_ntop(their_addr.ss_family,
                &((struct sockaddr_in6 *)&their_addr)->sin6_addr, ip_as_string,
                INET6_ADDRSTRLEN);
      std::cout << "New connection from " << ip_as_string << " on socket "
                << client_fd << std::endl;
    }
  }
  const int BUFFER_SIZE = 20;
  char buff[BUFFER_SIZE];
  int n;

  // infinite loop for chat
  while (true) {
    bzero(buff, BUFFER_SIZE);

    // read the message from client and copy it in buffer
    recv(client_fd, buff, sizeof(buff), 0);
    // print buffer which contains the client contents
    std::cout << "From client: " << buff << std::endl;
    // bzero(buff, BUFFER_SIZE);
    n = 0;
    // copy server message in the buffer
    // while ((buff[n++] = getchar()) != '\n')
    //   ;

    std::string reply;
    if (strncmp("exit", buff, 4) == 0) {
      std::cout << "Server Exit..." << std::endl;
      reply = "exit";
      send(client_fd, reply.c_str(), reply.length(), 0);
      break;
    }
    // and send that buffer to client
    reply = "Ok!";
    write(client_fd, reply.c_str(), reply.length());
  }
  return 0;
}
