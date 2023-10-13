#ifndef __HELPER_HPP__
#define __HELPER_HPP__

#include <chrono>
#include <stdlib.h>

namespace Helper {
class Random {
public:
  static inline int GetRandrange(int min, int max) {
    return rand() % (max - min + 1) + min;
  }
  static void FillRandomReadableASCII(char *mem, size_t size) {
    for (size_t i = 0; i < size; i++) {
      // choose between 3 section 0: [a-z] 1: [A-Z] 2: [0-9]
      int section = GetRandrange(0, 2);
      int low, high;
      switch (section) {
      case 0:
        low = 'a';
        high = 'z';
        break;
      case 1:
        low = 'A';
        high = 'Z';
        break;
      case 2:
        low = '0';
        high = '9';
        break;
      default:
        std::cerr << __FILE__ << ":" << __LINE__ << " :: Invalid section"
                  << std::endl;
        abort();
      }
      // generate a random byte
      int x = GetRandrange(low, high);
      mem[i] = x;
    }
  }
};

using time_unit_t = long long;
using microsecond_t = std::chrono::microseconds;

template <typename T> class Timer {
public:
  Timer() = delete;
  inline Timer(time_unit_t &st) : store_at(st) {
    t1 = std::chrono::high_resolution_clock::now();
  }
  inline ~Timer() {
    t2 = std::chrono::high_resolution_clock::now();
    auto elapsed = t2 - t1;
    // store_at =
    // std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    store_at = std::chrono::duration_cast<T>(elapsed).count();
  }

private:
  time_unit_t &store_at;
  std::chrono::high_resolution_clock::time_point t1, t2;
};

class Allocator {
public:
  template <typename T> inline static T *alloc(size_t size = 1) {
    return static_cast<T *>(malloc(sizeof(T) * size));
    // TODO(Maam): bzero??
  }
};
} // namespace Helper
#endif
