#ifndef ENUM_BIT_OPS_H
#define ENUM_BIT_OPS_H

// Defines all bitwise operators for a scoped enum type backed by an integral.
// Usage: DEFINE_ENUM_BIT_OPS(MyEnum)
#define DEFINE_ENUM_BIT_OPS(EnumType)                                                              \
  [[nodiscard]] constexpr EnumType operator^(EnumType lhs, EnumType rhs) noexcept {                \
    return static_cast<EnumType>(std::to_underlying(lhs) ^ std::to_underlying(rhs));               \
  }                                                                                                \
  [[nodiscard]] constexpr EnumType operator|(EnumType lhs, EnumType rhs) noexcept {                \
    return static_cast<EnumType>(std::to_underlying(lhs) | std::to_underlying(rhs));               \
  }                                                                                                \
  [[nodiscard]] constexpr EnumType operator&(EnumType lhs, EnumType rhs) noexcept {                \
    return static_cast<EnumType>(std::to_underlying(lhs) & std::to_underlying(rhs));               \
  }                                                                                                \
  [[nodiscard]] constexpr EnumType operator~(EnumType v) noexcept {                                \
    return static_cast<EnumType>(~std::to_underlying(v));                                          \
  }                                                                                                \
  constexpr EnumType &operator^=(EnumType &lhs, EnumType rhs) noexcept { return lhs = lhs ^ rhs; } \
  constexpr EnumType &operator|=(EnumType &lhs, EnumType rhs) noexcept { return lhs = lhs | rhs; } \
  constexpr EnumType &operator&=(EnumType &lhs, EnumType rhs) noexcept { return lhs = lhs & rhs; }

#endif // ENUM_BIT_OPS_H
