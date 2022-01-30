//
// Created by 77205 on 2022/1/26.
//

#ifndef OUR_GRAPHIC_UTILS_HASHUTILS_H_
#define OUR_GRAPHIC_UTILS_HASHUTILS_H_
#include <type_traits>
#include <array>

namespace our_graph::utils {
template<class T>
inline void CombineHash(size_t& seed, const T& value) {
  std::hash<T> hasher;
  seed ^= (hasher(value) << 1u);
}
}
#endif //OUR_GRAPHIC_UTILS_HASHUTILS_H_
