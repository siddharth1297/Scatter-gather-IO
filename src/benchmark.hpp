#ifndef __BENCHMARK_HPP__
#define __BENCHMARK_HPP__

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "net.h"
//#include "timer.hpp"
#include "helper.hpp"

// sg
#include <sys/uio.h>

#define KB *1024

// TODO: Compile in  O3

const size_t ROUNDS = 5;
// in KB
static size_t MESSAGE_SIZES[] = {10 KB, 100 KB, 1000 KB, 10000 KB};

const size_t MESSAGE_SIZES_LEN =
    sizeof(MESSAGE_SIZES) / sizeof(MESSAGE_SIZES[0]);

typedef struct msg_t {
  size_t *sizes_ = nullptr;
  char **mem_ = nullptr;
  size_t len_ = 0;

  msg_t(size_t len): len_(len) {}
  ~msg_t() {
	  for(size_t i=0; i<len_; i++)
		  free(mem_[i]);
  }

  void FillRandomeData() {
	  for(size_t i=0; i<len_; i++) {
	    mem_[i] = Helper::Allocator::alloc<char>(sizes_[i]);
    		Helper::Random::FillRandomReadableASCII(mem_[i], sizes_[i]);
	  }
  }
  void SerializeAndWrite(int fd) {
	  /*
    size_t offset = 0;
    size_t bytes = 0;
    char *buf = nullptr;
    Helper::time_unit_t complete_serialization_time;
    {
      Helper::Timer<Helper::microsecond_t> _(complete_serialization_time);
      // Allocate buffer
      buf = Helper::Allocator::alloc<char>();
      // serialize
      memcpy(buf + offset, mem_1, size_1);
      offset += size_1;
      bytes = write(fd, buf, size_1);
      // deallocate
      free(buf);
    }
    assert(offset == bytes);
    std::cout << __FUNCTION__ << ":: Bytes: " << bytes << " Timetaken: " << complete_serialization_time << std::endl;
    */
  }
} msg_t;

typedef struct msg_1_t {
  size_t size_1;
  char *mem_1;
  ~msg_1_t() {
    if (size_1)
      free(mem_1);
  }
  void FillRandomeData() {
    mem_1 = Helper::Allocator::alloc<char>(size_1);
    Helper::Random::FillRandomReadableASCII(mem_1, size_1);
  }
  void SerializeAndWrite(int fd) {
    size_t offset = 0;
    ssize_t bytes = 0;
    char *buf = nullptr;
    Helper::time_unit_t complete_serialization_time;
    {
      Helper::Timer<Helper::microsecond_t> _(complete_serialization_time);
      // Allocate buffer
      buf = Helper::Allocator::alloc<char>();
      // serialize
      memcpy(buf + offset, mem_1, size_1);
      offset += size_1;
      bytes = write(fd, buf, size_1);
      // deallocate
      free(buf);
    }
    assert(offset == bytes);
    std::cout << "write:: " << "Bytes: " << bytes << " Timetaken: " << complete_serialization_time << std::endl;
  }

  void SerializeAndWritev(int fd) {
    size_t offset = 0;
    ssize_t bytes = 0;
    int iov_len = 1;
    struct iovec iov[iov_len];
    iov[0].iov_base = mem_1;
    iov[0].iov_len = static_cast<int>(size_1);
    offset += size_1;
    Helper::time_unit_t complete_serialization_time;
    {
      Helper::Timer<Helper::microsecond_t> _(complete_serialization_time);
      bytes = writev(fd, iov, iov_len);
    }
    assert(offset == bytes);
    std::cout << "writev:: " << "Bytes: " << bytes << " Timetaken: " << complete_serialization_time << std::endl;
  }
} msg_1_t;

typedef struct msg_2_t {
  char *mem_1;
  char *mem_2;
  msg_2_t() { mem_1 = mem_2 = nullptr; }
} msg_2_t;

char *interface = NULL;
char *server_addr = NULL;
port_t port = 0;

int GetNewTCPClient() { return TCPClient(interface, server_addr, port); }

void Benchmark_type_1(size_t size) {
  msg_1_t *msg = Helper::Allocator::alloc<msg_1_t>();
  msg->size_1 = size;
  msg->FillRandomeData();
  int fd = GetNewTCPClient();
  msg->SerializeAndWrite(fd);
  close(fd);
  sleep(1);
  fd = GetNewTCPClient();
  msg->SerializeAndWritev(fd);
  close(fd);
}
// benchmark standard write system call
void Benchmark_write(size_t size) {
	for(size_t i=0; i<ROUNDS; i++) {
	Benchmark_type_1(size);
	sleep(1);
	}
}

void Benchmark_for_size(size_t size) { Benchmark_write(size); }

void Benchmark(char *device, char *server, port_t p) {
  interface = device;
  server_addr = server;
  port = p;
  for (size_t i = 0; i < MESSAGE_SIZES_LEN; i++) {
	  std::cout << "===============SIZE: " << MESSAGE_SIZES[i] << "===============" << std::endl;
    Benchmark_for_size(MESSAGE_SIZES[i]);
    //break;
    sleep(2);
	  std::cout << "==============================================================" << std::endl;
  }
}

#endif
