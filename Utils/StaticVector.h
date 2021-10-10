//
// Created by Glodxy on 2021/10/10.
//

#ifndef OUR_GRAPHIC_UTILS_STATICVECTOR_H_
#define OUR_GRAPHIC_UTILS_STATICVECTOR_H_
#include <array>
#include <cstddef>
#include <type_traits>
#include <cassert>

template<class T, size_t N>
class static_vector { //NOLINT
  typename std::aligned_storage<sizeof(T), alignof(T)>::type mData[N];
  uint32_t mSize = 0;
 public:
  static_vector() = default; //NOLINT

  ~static_vector() noexcept {
    for (auto& elem : *this) {
      elem.~T();
    }
  }

  explicit static_vector(size_t count) noexcept : mSize(count) {
    assert(count < N);
    std::uninitialized_fill_n(begin(), count, T{});
  }

  static_vector(static_vector const& rhs) noexcept : mSize(rhs.mSize) {
    std::uninitialized_copy(rhs.begin(), rhs.end(), begin());
  }

  size_t size() const noexcept { return mSize; }

  T* data() noexcept { return reinterpret_cast<T*>(&mData[0]); }

  T const* data() const noexcept { return reinterpret_cast<T const*>(&mData[0]); }

  static_vector& operator=(static_vector const& rhs) noexcept {
    if (this != &rhs) {
      const size_t n = std::min(mSize, rhs.mSize);
      std::copy_n(rhs.begin(), n, begin());
      for (size_t pos = n, c = mSize; pos < c; ++pos) {
        data()[pos].~T();
      }
      std::uninitialized_copy(rhs.begin() + n, rhs.end(), begin() + n);
      mSize = rhs.mSize;
    }
    return *this;
  }

  const T& operator[](size_t pos) const noexcept {
    assert(pos < mSize);
    return data()[pos];
  }

  T& operator[](size_t pos) noexcept {
    assert(pos < mSize);
    return data()[pos];
  }

  T* begin() { return data(); }
  T* end() { return data() + mSize; }
  T const* begin() const { return data(); }
  T const* end() const { return data() + mSize; }
};

#endif //OUR_GRAPHIC_UTILS_STATICVECTOR_H_
