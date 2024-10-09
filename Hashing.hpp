#pragma once

#include <type_traits>
#include <string>

#include "GblDefines.hpp"
#include "Types.hpp"

namespace sp
{
  /*template <class T>
  typename std::enable_if<std::is_array<T>::value>::type
  size_t Hash(T p_key, size_t table_size)
  {
    size_t total = 0;
    for(const auto ch: p_key)
    {
      total += static_cast<size_t>(ch);
    }

    return total % table_size;
  };*/

  size_t HashString(std::string p_key, size_t table_size)
  {
    size_t total = 0;
    for(const auto ch: p_key)
    {
      total += static_cast<size_t>(ch);
    }

    return total % table_size;
  };
}