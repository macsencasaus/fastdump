#ifndef UTIL_HPP
#define UTIL_HPP

// helper consteval string functions

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>

template <size_t M, size_t N>
static consteval std::array<char, M> padded_string(const char (&str)[N]) {
  static_assert(M >= N, "target array too small");

  std::array<char, M> out{};
  std::copy_n(str, N, out.begin());
  return out;
}

template <size_t M, size_t N>
static consteval void slice_and_replace(std::array<char, M>& dest,
                                        size_t i,
                                        size_t n,
                                        const std::array<char, N>& src) {
  const size_t src_n = std::char_traits<char>::length(src.begin());
  const auto src_end = src.begin() + src_n;
  const auto it = dest.begin() + i;

  if (n == src_n) {
    std::copy(src.begin(), src_end, it);
  } else if (n > src_n) {
    std::copy(src.begin(), src_end, it);
    std::copy(it + n, dest.end(), it + src_n);
  } else if (n < src_n) {
    std::copy_backward(it + n, dest.end(), it + src_n);
    std::copy(src.begin(), src_end, it);
  }
}

#endif
