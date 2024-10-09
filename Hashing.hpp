#pragma once

#include <type_traits>
#include <string>
#include <cmath>

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

  size_t MultiplicationHash(std::string p_key, size_t table_size)
  {
    size_t total = 0; //static_cast<size_t>(p_key[0]);
    for(const auto ch: p_key)
    {
      total += static_cast<size_t>(ch);
    }
    
    constexpr double A = 0.6180339887; //constexpr double A = (std::sqrt(5) - 1) / 2;
    const auto prod  = A * total;
    const auto frac = prod - std::floor(prod);

    return static_cast<size_t>(table_size * frac);
  };
}