#pragma once

// Array.hpp : Include file for standard system include files,
// or project specific include files.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "array/exceptions.hpp"

template <typename T> class Array {
private:
  std::vector<T> arr;

public:
  Array() = default;

  Array(std::initializer_list<T> list) : arr(list) {}

  Array(std::size_t size) : arr(size) {}

  Array(const Array &) = default;
  Array(Array &&) noexcept = default;
  Array &operator=(const Array &) = default;
  Array &operator=(Array &&) noexcept = default;

  // Capacity
  size_t size() const noexcept;
  bool is_empty() const noexcept;
  void reserve(size_t capacity);

  // Element access
  T &get(int index);
  const T &get(int index) const;
  const T &pick_random() const;
  int find(const T &data) const;

  T &operator[](int index);
  const T &operator[](int index) const;

  // Modifiers
  void append(const T &data);
  void append(T &&data);

  void join(const Array<T> &array);
  void join(Array<T> &&array);

  T pop();

  // Algorithms
  void sort();
  Array<T> &reverse();

  template <typename Predicate> Array<T> filter(Predicate predicate) const;

  // Iterators
  auto begin() noexcept { return arr.begin(); }
  auto end() noexcept { return arr.end(); }

  auto begin() const noexcept { return arr.begin(); }
  auto end() const noexcept { return arr.end(); }

  auto cbegin() const noexcept { return arr.cbegin(); }
  auto cend() const noexcept { return arr.cend(); }

  friend std::ostream &operator<<(std::ostream &os, const Array<T> &array) {
    os << "{";

    for (size_t i = 0; i < array.arr.size(); ++i) {
      if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t> ||
                    std::is_same_v<T, unsigned char> ||
                    std::is_same_v<T, signed char>)
        os << static_cast<int>(array.arr[i]);
      else
        os << array.arr[i];

      if (i + 1 != array.arr.size())
        os << ", ";
    }

    os << "}";

    return os;
  }
};

template <typename T> int Array<T>::find(const T &data) const {
  auto it = std::find(arr.begin(), arr.end(), data);

  if (it == arr.end())
    return -1;

  return static_cast<int>(std::distance(arr.begin(), it));
}

template <typename T> Array<T> &Array<T>::reverse() {
  std::reverse(arr.begin(), arr.end());
  return *this;
}

template <typename T>
template <typename Predicate>
Array<T> Array<T>::filter(Predicate predicate) const {
  Array<T> result;

  for (const auto &item : arr) {
    if (predicate(item))
      result.append(item);
  }

  return result;
}

template <typename T> T &Array<T>::operator[](int index) {
  if (index < 0 || static_cast<size_t>(index) >= size())
    throw ArrayIndexOutOfRange();

  return arr[index];
}

template <typename T> const T &Array<T>::operator[](int index) const {
  if (index < 0 || static_cast<size_t>(index) >= size())
    throw ArrayIndexOutOfRange();

  return arr[index];
}

template <typename T> bool Array<T>::is_empty() const noexcept {
  return arr.empty();
}

template <typename T> void Array<T>::reserve(size_t capacity) {
  arr.reserve(capacity);
}

template <typename T> T Array<T>::pop() {
  if (arr.empty())
    throw std::runtime_error("Cannot pop from an empty Array.");

  T value = std::move(arr.back());
  arr.pop_back();

  return value;
}

template <typename T> void Array<T>::join(const Array<T> &array) {
  arr.insert(arr.end(), array.begin(), array.end());
}

template <typename T> void Array<T>::join(Array<T> &&array) {
  for (auto &item : array) {
    arr.push_back(std::move(item));
  }
}

template <typename T> T &Array<T>::get(int index) {
  if (index < 0 || static_cast<size_t>(index) >= size())
    throw ArrayIndexOutOfRange();

  return arr[index];
}

template <typename T> const T &Array<T>::get(int index) const {
  if (index < 0 || static_cast<size_t>(index) >= size())
    throw ArrayIndexOutOfRange();

  return arr[index];
}

template <typename T> void Array<T>::sort() {
  std::sort(arr.begin(), arr.end());
}

template <typename T> size_t Array<T>::size() const noexcept {
  return arr.size();
}

template <typename T> void Array<T>::append(const T &data) {
  arr.push_back(data);
}

template <typename T> void Array<T>::append(T &&data) {
  arr.push_back(std::move(data));
}

template <typename T> const T &Array<T>::pick_random() const {
  if (arr.empty())
    throw std::runtime_error("Array is empty.");

  static std::random_device rd;
  static std::mt19937 gen(rd());

  std::uniform_int_distribution<size_t> distrib(0, arr.size() - 1);

  return arr[distrib(gen)];
}