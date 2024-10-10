#pragma once

#include <type_traits>
#include <string>
#include <cmath>

#include "GblDefines.hpp"
#include "Types.hpp"

namespace sp
{
  template<typename T>
  concept Integral = std::is_integral<T>::value;

  template<typename T>
  concept BoundedArray = std::is_bounded_array<T>::value;

  template<typename T>
  concept UnBoundedArray = std::is_unbounded_array<T>::value;

  template<Integral T>
  size_t SimpleHash(T&& p_key, size_t table_size)
  {
    return (size_t)p_key % table_size;
  };

  template<BoundedArray T>
  size_t SimpleHash(T&& p_key, size_t table_size)
  {
    size_t total = 0;
    for(const auto ch: p_key)
    {
      total += static_cast<size_t>(ch);
    }
    return total % table_size;
  };

  template<BoundedArray T>
  size_t MultiplicationHash(T&& p_key, size_t table_size)
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

  template<UnBoundedArray T>
  size_t MultiplicationHash(T&& p_key, size_t table_size)
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
  
  size_t MultiplicationHash(std::string_view p_key, size_t table_size)
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