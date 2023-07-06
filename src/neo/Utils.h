// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2023 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* Utils.h                                         (C) 2000-2023             */
/*                                                                           */
/* Neo utils                                                                 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#ifndef NEO_UTILS_H
#define NEO_UTILS_H

#include <iostream>
#include <vector>
#include <cassert>
#include <cstdint>
#include <string>
#include <chrono>
#include <iterator>

#ifdef NDEBUG
static constexpr bool ndebug = true;
static constexpr bool _debug = false;
#ifdef _MSC_VER
#define _MS_REL_
#endif
#else
static constexpr bool ndebug = false;
static constexpr bool _debug = true;
#endif

//----------------------------------------------------------------------------/
//----------------------------------------------------------------------------/

namespace Neo
{
struct NullOstream : public std::ostream
{};
struct NeoOutputStream
{
  mutable NullOstream m_null_ostream;
};
inline NeoOutputStream print() {
  return NeoOutputStream{};
}

//----------------------------------------------------------------------------/

namespace utils
{
  using Int64 = std::int64_t;
  using Int32 = std::int32_t;
  using Real = double;
  static constexpr utils::Int32 NULL_ITEM_LID = -1;

  //----------------------------------------------------------------------------/

  struct Real3
  {
    double x, y, z;
    // Utilities
    friend inline std::ostream& operator<<(std::ostream& oss, Neo::utils::Real3 const& real3) {
      oss << "{" << real3.x << "," << real3.y << "," << real3.z << "}";
      return oss;
    }
    friend inline bool operator==(Neo::utils::Real3 const& a, Neo::utils::Real3 const& b) {
      return a.x == b.x && a.y == b.y && a.z == b.z;
    }
    friend inline bool operator!=(Neo::utils::Real3 const& a, Neo::utils::Real3 const& b) {
      return !(a == b);
    }
    friend inline Real3 operator+(Neo::utils::Real3 const& a, Neo::utils::Real3 const& b) {
      return Real3{ a.x + b.x, a.y + b.y, a.z + b.z };
    }
    friend inline Real3 operator-(Neo::utils::Real3 const& a, Neo::utils::Real3 const& b) {
      return Real3{ a.x - b.x, a.y - b.y, a.z - b.z };
    }
    friend inline Real3 operator*(Neo::utils::Real3 const& a, Neo::utils::Real3 const& b) {
      return Real3{ a.x * b.x, a.y * b.y, a.z * b.z };
    }
    friend inline Real3 operator/(Neo::utils::Real3 const& a, Neo::utils::Real3 const& b) {
      return Real3{ a.x / b.x, a.y / b.y, a.z / b.z };
    }
    friend inline Real3& operator+=(Neo::utils::Real3& a, Neo::utils::Real3 const& b) {
      a = a + b;
      return a;
    }
    friend inline Real3& operator-=(Neo::utils::Real3& a, Neo::utils::Real3 const& b) {
      a = a - b;
      return a;
    }
    friend inline Real3& operator*=(Neo::utils::Real3& a, Neo::utils::Real3 const& b) {
      a = a * b;
      return a;
    }
    friend inline Real3& operator/=(Neo::utils::Real3& a, Neo::utils::Real3 const& b) {
      a = a / b;
      return a;
    }
  };

  //----------------------------------------------------------------------------/

  /*!
     * \brief View of a data array
     * @tparam T view data type
     * Will be replaced by std::span when moving to C++20
    */
  // todo use std::span instead when moving to C++20
  template <typename T>
  struct Span
  {
    using value_type = T;
    using non_const_value_type = typename std::remove_const<T>::type;
    using size_type = int;
    using vector_size_type = typename std::vector<non_const_value_type>::size_type;

    size_type m_size = 0;
    T* m_ptr = nullptr;

    Span(size_type size, T* data)
    : m_size(size)
    , m_ptr(data) {}
    Span(vector_size_type size, T* data)
    : m_size(size)
    , m_ptr(data) {}
    Span() = default;

    T& operator[](int i) {
      assert(i < m_size);
      return *(m_ptr + i);
    }

    T const& operator[](int i) const {
      assert(i < m_size);
      return *(m_ptr + i);
    }

    T* begin() { return m_ptr; }
    T* end() { return m_ptr + m_size; }

    int size() const { return m_size; }
    std::vector<non_const_value_type> copy() {
      std::vector<non_const_value_type> vec(m_size);
      std::copy(this->begin(), this->end(), vec.begin());
      return vec;
    }
  };

  //----------------------------------------------------------------------------/

  template <typename T>
  struct ConstSpan
  {
    using value_type = T;
    using non_const_value_type = typename std::remove_const<T>::type;
    using size_type = int;
    using vector_size_type = typename std::vector<non_const_value_type>::size_type;

    int m_size = 0;
    const T* m_ptr = nullptr;

    ConstSpan(size_type size, const T* data)
    : m_size(size)
    , m_ptr(data) {}
    ConstSpan(vector_size_type size, const T* data)
    : m_size(size)
    , m_ptr(data) {}
    ConstSpan() = default;

    const T& operator[](int i) const {
      assert(i < m_size);
      return *(m_ptr + i);
    }

    const T* begin() const { return m_ptr; }
    const T* end() const { return m_ptr + m_size; }

    int size() const { return m_size; }

    std::vector<non_const_value_type> copy() {
      std::vector<non_const_value_type> vec(m_size);
      std::copy(this->begin(), this->end(), vec.begin());
      return vec;
    }
  };

  //----------------------------------------------------------------------------/

  /*!
   * \brief 2-Dimensional view of a contiguous data chunk
   * The second dimension varies first {(i,j),(i,j+1),(i+1,j),(i+1,j+1)}...
   * \fn operator[i] returns a view of size \refitem Array2View.m_dim2_size
   * @tparam T view data type
   * Will be replaced by std::mdspan when moving to C++23
   */
  // todo use std::mdspan instead when moving to C++23
  template <typename T>
  struct Array2View {
    using value_type = T;
    using size_type = int;

    size_type m_dim1_size = 0;
    size_type m_dim2_size = 0;
    T* m_ptr = nullptr;

    Span<T> operator[](int i) {
      assert(i < m_dim1_size);
      return { m_dim2_size, m_ptr + i * m_dim2_size };
    }

    T* begin() {return m_ptr;}
    T* end() { return m_ptr + (m_dim1_size * m_dim2_size); }

    size_type dim1Size() const { return m_dim1_size; }
    size_type dim2Size() const { return m_dim2_size; }

    std::vector<T> copy() {
      std::vector<T> vec(m_dim1_size * m_dim2_size);
      std::copy(this->begin(), this->end(), vec.begin());
      return vec;
    }
  };

  //----------------------------------------------------------------------------/

  /*!
   * 2-Dimensional const view. cf. \refitem Array2View
   * @tparam T view data type
   */
  template <typename T>
  struct ConstArray2View
  {
    using value_type = T;
    using size_type = int;

    size_type m_dim1_size = 0;
    size_type m_dim2_size = 0;
    T* m_ptr = nullptr;

    ConstSpan<T> operator[](int i) const {
      assert(i < m_dim1_size);
      return { m_dim2_size, m_ptr + i * m_dim2_size };
    }

    const T* begin() {return m_ptr;}
    const T* end()   {return m_ptr+(m_dim1_size*m_dim2_size);}

    size_type dim1Size() const { return m_dim1_size; }
    size_type dim2Size() const { return m_dim2_size; }

    std::vector<T> copy() { std::vector<T> vec(m_dim1_size*m_dim2_size);
      std::copy(this->begin(), this->end(), vec.begin());
      return vec;
    }
  };

  //----------------------------------------------------------------------------/

  template <typename Container>
  std::ostream&  _printContainer(Container&& container, std::ostream& oss){
    std::copy(container.begin(), container.end(), std::ostream_iterator<typename std::remove_reference_t<Container>::value_type>(oss, " "));
    return oss;
  }

  template <typename Container>
  void printContainer(Container&& container, std::string const& name = "Container") {
    std::cout << name << " , size : " << container.size() << std::endl;
    _printContainer(container, std::cout);
    std::cout << std::endl;
  }

  //----------------------------------------------------------------------------/

  using namespace std::chrono_literals;

  struct Profiler
  {

    std::string m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_end;
    void start() { m_start = std::chrono::high_resolution_clock::now(); }
    void stop() { m_end = std::chrono::high_resolution_clock::now(); }
    void start(std::string name) {
      m_name = name;
      start();
    }
    void stop_and_print(std::string message) {
      stop();
      print(message);
    }
    void stop_and_print() {
      stop();
      print("");
    }
    void print(std::string message) {
      std::cout << "=== cpu time " << message << m_name << " = " << (m_end - m_start) / 1us << " microseconds ===" << std::endl;
    }
  };

  //----------------------------------------------------------------------------/

  // Some useful using
  using Int32Span = Span<Int32>;
  using Int64Span = Span<Int64>;
  using Int32ConstSpan = ConstSpan<Int32>;
  using Int64ConstSpan = ConstSpan<Int64>;
  using Int32Span2D = Array2View<Int32>; // todo check name see stl MDSpan et Arcane ?
  using Int64Span2D = Array2View<Int64>;
  using Int32ConstSpan2D = ConstArray2View<Int32>; // todo check name see stl MDSpan et Arcane ?
  using Int64SConstpan2D = ConstArray2View<Int64>;

} // end namespace utils

//----------------------------------------------------------------------------/

} // end namespace Neo

//----------------------------------------------------------------------------/

// Array utilities
template <typename T>
std::ostream& operator<<(std::ostream& oss, std::vector<T> const& container) {
  return Neo::utils::_printContainer(container, oss);
}

template <typename T>
std::ostream& operator<<(std::ostream& oss, Neo::utils::Span<T> const& container) {
  return Neo::utils::_printContainer(container, oss);
}

template <typename T>
std::ostream& operator<<(std::ostream& oss, Neo::utils::ConstSpan<T> const& container) {
  return Neo::utils::_printContainer(container, oss);
}

template <typename T>
std::ostream& operator<<(Neo::NeoOutputStream const& oss, T const& printable) {
  if constexpr (ndebug)
    return oss.m_null_ostream;
  else
    return std::cout << printable;
}

//----------------------------------------------------------------------------/
//----------------------------------------------------------------------------/

#endif // NEO_UTILS_H
