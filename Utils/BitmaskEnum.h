//
// Created by Glodxy on 2021/10/7.
// copy from filament
//

#ifndef OUR_GRAPHIC_UTILS_BITMASKENUM_H_
#define OUR_GRAPHIC_UTILS_BITMASKENUM_H_

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace our_graph::utils {
template<typename Enum>
struct EnableBitMaskOperators : public std::false_type { };
}  // namespace our_graph::utils

namespace our_graph {
template<typename Enum, typename std::enable_if_t<
    std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr bool operator!(Enum rhs) noexcept {
  using underlying = std::underlying_type_t<Enum>;
  return underlying(rhs) == 0;
}

template<typename Enum, typename std::enable_if_t<
    std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator~(Enum rhs) noexcept {
  using underlying = std::underlying_type_t<Enum>;
  return Enum(~underlying(rhs));
}

template<typename Enum, typename std::enable_if_t<
    std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator|(Enum lhs, Enum rhs) noexcept {
  using underlying = std::underlying_type_t<Enum>;
  return Enum(underlying(lhs) | underlying(rhs));
}

template<typename Enum, typename std::enable_if_t<
    std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator&(Enum lhs, Enum rhs) noexcept {
  using underlying = std::underlying_type_t<Enum>;
  return Enum(underlying(lhs) & underlying(rhs));
}

template<typename Enum, typename std::enable_if_t<
    std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator^(Enum lhs, Enum rhs) noexcept {
  using underlying = std::underlying_type_t<Enum>;
  return Enum(underlying(lhs) ^ underlying(rhs));
}

template<typename Enum, typename std::enable_if_t<
    std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator|=(Enum &lhs, Enum rhs) noexcept {
  return lhs = lhs | rhs;
}

template<typename Enum, typename std::enable_if_t<
    std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator&=(Enum &lhs, Enum rhs) noexcept {
  return lhs = lhs & rhs;
}

template<typename Enum, typename std::enable_if_t<
    std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator^=(Enum &lhs, Enum rhs) noexcept {
  return lhs = lhs ^ rhs;
}

template<typename Enum, typename std::enable_if_t<
    std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr bool none(Enum lhs) noexcept {
  return !lhs;
}

template<typename Enum, typename std::enable_if_t<
    std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr bool any(Enum lhs) noexcept {
  return !none(lhs);
}
}  // namespace our_graph
#endif //OUR_GRAPHIC_UTILS_BITMASKENUM_H_
