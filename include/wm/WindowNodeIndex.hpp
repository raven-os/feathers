#pragma once

#include <cstdint>


namespace wm
{
  struct WindowNodeIndex
  {
    /// used for offsets (operators `+=`, `+`, `-=`, `-`)
    using offset_type = uint16_t;
    /// the stored type
    using data_type = uint16_t;

      data_type data;

    constexpr WindowNodeIndex(WindowNodeIndex const &) noexcept = default;
    constexpr WindowNodeIndex &operator=(WindowNodeIndex const &) noexcept = default;

    /// Construtor is explicit, to avoid conversion
    explicit constexpr WindowNodeIndex(data_type data = 0u) noexcept
      : data(data)
    {}

#define WINDOW_NODE_INDEX_PREFIX_OP(OP)			\
    constexpr WindowNodeIndex &operator OP() noexcept	\
    {							\
      OP data;						\
      return *this;					\
    }

    WINDOW_NODE_INDEX_PREFIX_OP(++);
    WINDOW_NODE_INDEX_PREFIX_OP(--);

#undef WINDOW_NODE_INDEX_PREFIX_OP

#define WINDOW_NODE_INDEX_SUFFIX_OP(OP)			\
    constexpr WindowNodeIndex operator OP(int) noexcept \
    {							\
      auto copy(*this);					\
      OP *this;						\
      return copy;					\
    }

    WINDOW_NODE_INDEX_SUFFIX_OP(++);
    WINDOW_NODE_INDEX_SUFFIX_OP(--);

#undef WINDOW_NODE_INDEX_SUFFIX_OP

#define WINDOW_NODE_INDEX_COMPARE(OP)					\
    constexpr bool operator OP(WindowNodeIndex const &other) const noexcept \
    {                                                                   \
      return data OP other.data;                                        \
    }

    WINDOW_NODE_INDEX_COMPARE(==);
    WINDOW_NODE_INDEX_COMPARE(!=);
    WINDOW_NODE_INDEX_COMPARE(<=);
    WINDOW_NODE_INDEX_COMPARE(>=);
    WINDOW_NODE_INDEX_COMPARE(<);
    WINDOW_NODE_INDEX_COMPARE(>);

#undef WINDOW_NODE_INDEX_COMPARE

#define WINDOW_NODE_INDEX_BINARY_OP(OP)					\
    constexpr auto operator OP(offset_type const &other) const noexcept \
    {									\
      return WindowNodeIndex(static_cast<data_type>(data OP other));	\
    }

    WINDOW_NODE_INDEX_BINARY_OP(+);
    WINDOW_NODE_INDEX_BINARY_OP(-);

#undef WINDOW_NODE_INDEX_BINARY_OP
  };

  static constexpr WindowNodeIndex nullNode{uint16_t(-1u)};
}
