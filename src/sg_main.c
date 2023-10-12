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

#define MSG_HEADER_SIZE 1024
#define MSG_BODY_SIZE 1024
#define MSG_DEFAULT_HEADER "MsgHeader"
#define MSG_DEFAULT_BODY "MsgBody"

typedef struct message_t {
  char header[MSG_HEADER_SIZE];
  char body[MSG_BODY_SIZE];
} message_t;

void SendSerializaedTestMsg(int fd) {
  message_t msg = {.header = MSG_DEFAULT_HEADER, .body = MSG_DEFAULT_BODY};
  char buf[MSG_HEADER_SIZE + MSG_BODY_SIZE];
  bzero(buf, MSG_HEADER_SIZE + MSG_BODY_SIZE);
  size_t offset = 0;
  strcpy(buf + offset, msg.header);
  offset += strlen(msg.header);
  strcpy(buf + offset, msg.body);
  offset += strlen(msg.body);

  int bytes = write(fd, buf, strlen(buf));
  printf("Header: \"%s\"\n", msg.header);
  printf("Body: \"%s\"\n", msg.body);
  printf("Buf: \"%s\"\n", buf);
  printf("Wrote: Size: %d bufSize: %ld\n", bytes, offset);
  if (bytes <= 0) {
    perror("writev");
    exit(EXIT_FAILURE);
  }
}

void SendTestMsg(int fd) {
  message_t msg = {.header = MSG_DEFAULT_HEADER, .body = MSG_DEFAULT_BODY};
  struct iovec iov[2];
  iov[0].iov_base = msg.header;
  iov[0].iov_len = strlen(msg.header);
  iov[1].iov_base = msg.body;
  iov[1].iov_len = strlen(msg.body);
  int bytes = writev(fd, iov, 2);
  printf("Header: \"%s\"\n", msg.header);
  printf("Body: \"%s\"\n", msg.body);
  printf("Wrote: Size: %d \n", bytes);
  if (bytes <= 0) {
    perror("writev");
    exit(EXIT_FAILURE);
  }
}

void ReceiveTestMsg(int fd, message_t *msg) {
  struct iovec iov[2];
  iov[0].iov_base = msg->header;
  // iov[0].iov_len = MSG_HEADER_SIZE;
  iov[0].iov_len = strlen(MSG_DEFAULT_HEADER);
  iov[1].iov_base = msg->body;
  // iov[1].iov_len = MSG_BODY_SIZE;
  iov[1].iov_len = strlen(MSG_DEFAULT_BODY);

  // int bytes = readv(fd, iov, 2);
  int bytes = read(fd, msg->header, MSG_BODY_SIZE);

  printf("Header: \"%s\"\n", msg->header);
  printf("Body: \"%s\"\n", msg->body);
  printf("Read: Size: %d \n", bytes);
  if (bytes <= 0) {
    perror("readv");
    exit(EXIT_FAILURE);
  }
}

size_t Receive(int fd) {
  size_t max_size = MSG_HEADER_SIZE + MSG_BODY_SIZE;
  char buf[max_size];
  size_t offset = 0, remaining_size = max_size;
  while (remaining_size) {
    ssize_t bytes = read(fd, buf + offset, remaining_size);
    printf("ChunkRead:: Size: %ld Buf: \"%s\"\n", bytes, buf + offset);

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
  printf("TotalRead:: Size: %ld Buf: \"%s\"\n", offset, buf);
  return offset;
}

void test_file() {
  int fd = open("msg.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);

  if (fd == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }
  SendTestMsg(fd);
  close(fd);

  fd = open("msg.txt", O_RDONLY);
  message_t msg;
  ReceiveTestMsg(fd, &msg);
  assert(strcmp(msg.header, MSG_DEFAULT_HEADER) == 0);
  assert(strcmp(msg.body, MSG_DEFAULT_BODY) == 0);
  close(fd);
}

void Server(char *interface) {
  printf("=====Starting Server=====\n");
  SetUpSignalHandler();
  CreateTCPServer(interface, PORT, 3);

  printf("Server status: %d\n", is_running);
  while (is_running) {
    int new_conn_fd = GetNewConnFd();
    // SendTestMsg(new_conn_fd);
    Receive(new_conn_fd);
    close(new_conn_fd);
  }
  close(server_fd);
}

void Client(char *interface, char *server_addr) {
  int client_fd = TCPClient(interface, server_addr, PORT);
  // Receive(client_fd);
  SendTestMsg(client_fd);
  // SendSerializaedTestMsg(client_fd);
  close(client_fd);
}

int main(int argc, char *argv[]) {
  int option;
  int is_server = 0;
  char *server_addr = "";
  char *interface = "";
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
    Client(interface, server_addr);
  }
  return 0;
}
