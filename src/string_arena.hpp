#ifndef STRING_ARENA_HPP
#define STRING_ARENA_HPP

#include <string.h>
#include <cassert>
#include <cstddef>
#include <cstdint>

struct String_Arena {
  size_t size;
  size_t capacity;
  char* pool;

  inline String_Arena(char* pool, size_t capacity)
      : size{0}, capacity{capacity}, pool{pool} {}

  inline void append(char c) {
    assert(size + 1 < capacity);
    pool[size++] = c;
  }

  inline void append(const char* str) {
    size_t n = strlen(str);
    assert(size + n < capacity);
    memcpy(pool + size, str, n);
    size += n;
  }

  inline void appendu(uint32_t u) {
    if (u == 0u) {
      append('0');
      return;
    }

    char buf[10];
    size_t len = 0;

    while (u > 0) {
      buf[len++] = '0' + (u % 10);
      u /= 10;
    }

    for (size_t i = 0; i < len; ++i) {
      append(buf[len - i - 1]);
    }
  }

  inline const char *str() {
    return pool;
  }
};

#endif  // STRING_ARENA_HPP
