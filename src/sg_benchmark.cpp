#include "benchmark.hpp"
#include "net.h"

#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>


int server_fd = -1;
int is_running = 0;
struct sockaddr_in server_address;

const port_t PORT = 8080;

size_t Receive(int fd, char *buf, size_t max_size) {
  size_t offset = 0, remaining_size = max_size;
  while (remaining_size) {
    ssize_t bytes = read(fd, buf + offset, remaining_size);
    //printf("ChunkRead:: Size: %ld Buf: \"%s\"\n", bytes, buf + offset);

    if (bytes > 0) {
      assert(bytes <= remaining_size);
      offset += bytes;
      remaining_size -= bytes;
    }
    if (bytes == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    }
    if (bytes == 0) {
      // connection is closed
      break;
    }
  }
  //printf("TotalRead:: Size: %ld Buf: \"%s\"\n", offset, buf);
  printf("TotalRead:: Size: %ld\n", offset);
  return offset;
}

void Server(char *interface) {
  printf("=====Starting Server=====\n");
  SetUpSignalHandler();
  size_t max_size = MESSAGE_SIZES[MESSAGE_SIZES_LEN - 1];
  char *buf = Helper::Allocator::alloc<char>(max_size);

  CreateTCPServer(interface, PORT, 3);

  printf("Server status: %d\n", is_running);
  while (is_running) {
    int new_conn_fd = GetNewConnFd();
    Receive(new_conn_fd, buf, max_size);
    close(new_conn_fd);
  }
  close(server_fd);
  free(buf);
}

void Client(char *interface, char *server_addr) {
  int client_fd = TCPClient(interface, server_addr, PORT);
  // SendTestMsg(client_fd);
  close(client_fd);
}

int main(int argc, char *argv[]) {
  int option;
  int is_server = 0;
  char *server_addr;
  char *interface;
  while ((option = getopt(argc, argv, "sc:i:h")) != -1) {
    switch (option) {
    case 's':
      is_server = 1;
      break;
    case 'c':
      is_server = 0;
      server_addr = optarg;
      break;
    case 'i':
      interface = optarg;
      break;
    case 'h':
    default:
      printf("Help/Usage Example\n");
      abort();
    }
  }

  // Print config
  printf("interface: \"%s\"\n", interface);
  printf("is_server: %d\n", is_server);
  printf("server_addr: \"%s\"\n", server_addr);
  if (is_server) {
    Server(interface);
  } else {
    assert(strlen(server_addr) > 0);
    // Client(interface, server_addr);
    Benchmark(interface, server_addr, PORT);
  }
  return 0;
}
