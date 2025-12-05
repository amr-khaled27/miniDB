#pragma once
#include <cstdint>
#include <array>
#include <cstddef>

constexpr size_t PAGE_SIZE = 4096;

struct Page {
  std::array<char, PAGE_SIZE> data;
  size_t used_bytes = 0;

  bool hasSpace(size_t n) const {
    return (used_bytes + n) <= PAGE_SIZE;
  }

  char* getWritePtr() {
    return data.data() + used_bytes;
  }

  const char* getReadPtr(size_t offset = 0) const {
    return data.data() + offset;
  }

  void clear() {
    used_bytes = 0;
  }
};