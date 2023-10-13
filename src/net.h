/*
 * Header for Network communication
 */

#ifndef NET_H
#define NET_H

#include <arpa/inet.h> // inet_pton
#include <assert.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef uint16_t port_t;

extern int server_fd;
extern int is_running;
extern struct sockaddr_in server_address;
int addrlen = sizeof(server_address);

int CreateTCPServer(char *interface, const port_t port, int backlog) {
  int opt = 1;
  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  if (strlen(interface) > 0 &&
      (setsockopt(server_fd, SOL_SOCKET, SO_BINDTODEVICE, interface,
                  strlen(interface)) == -1)) {
    perror("setsockopt: SO_BINDTODEVICE\n");
    exit(EXIT_FAILURE);
  }
  /*
  // Forcefully attaching socket to the port 8080
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  */
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&server_address,
           sizeof(server_address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  if (listen(server_fd, backlog) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  return is_running = 1;
}

int GetNewConnFd() {
  int new_socket = -1;
  if ((new_socket = accept(server_fd, (struct sockaddr *)&server_address,
                           (socklen_t *)&addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }
  return new_socket;
}

int TCPClient(char *interface, char *addr, port_t port) {
  int client_fd = -1;
  struct sockaddr_in serv_addr;
  if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket\n");
    exit(EXIT_FAILURE);
  }
  if (strlen(interface) > 0 &&
      (setsockopt(client_fd, SOL_SOCKET, SO_BINDTODEVICE, interface,
                  strlen(interface)) == -1)) {
    perror("setsockopt: SO_BINDTODEVICE\n");
    exit(EXIT_FAILURE);
  }
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, addr, &serv_addr.sin_addr) <= 0) {
    perror("inet_pton: Invalid address\n");
    exit(EXIT_FAILURE);
  }

  if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) ==
      -1) {
    perror("connect\n");
    exit(EXIT_FAILURE);
  }
  return client_fd;
}

void KillServer() { is_running = 0; }

void DefaultSignalHandler() {
  KillServer();
  close(server_fd);
  printf("Closed server_fd\n");
}

void SetUpSignalHandler() {
  // signal(SIGINT, DefaultSignalHandler);
}
#endif
