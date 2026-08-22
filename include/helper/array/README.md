# MSL Array Module

The `Array<T>` template class in MSL provides a dynamic, memory-safe, generic container with rich utility methods and Godot-inspired ergonomic syntax.

---

## Key Features

- **Generic & Dynamic**: Templated container backed by `std::vector<T>`.
- **Memory Safety**: Bounds-checked element access (`operator[]` and `get()`) throwing `ArrayIndexOutOfRange` (defined in `exceptions.hpp`).
- **Functional Algorithms**: Built-in `filter()`, `sort()`, `reverse()`, and `find()`.
- **Random Access & Utilities**: `pick_random()`, `pop()`, `join()`, `reserve()`.
- **Stream Friendly**: Direct printing via `std::cout << array`.

---

## Quick Tutorial & Examples

### 1. Initialization & Appending

```cpp
#include "array/Array.hpp"
#include <iostream>

int main() {
    // Initializer list constructor
    Array<int> numbers = {10, 20, 30};

    // Appending elements (copy and move)
    numbers.append(40);
    numbers.append(50);

    std::cout << "Array: " << numbers << "\n"; // Output: {10, 20, 30, 40, 50}
    std::cout << "Size: " << numbers.size() << "\n";
    return 0;
}
```

### 2. Searching, Filtering, and Sorting

```cpp
#include "array/Array.hpp"
#include <iostream>

int main() {
    Array<int> nums = {5, 2, 8, 1, 9, 4};

    // Sort in ascending order
    nums.sort();
    std::cout << "Sorted: " << nums << "\n"; // {1, 2, 4, 5, 8, 9}

    // Find element index
    int idx = nums.find(8);
    std::cout << "Index of 8: " << idx << "\n"; // 4

    // Filter elements (even numbers only)
    Array<int> evens = nums.filter([](int n) { return n % 2 == 0; });
    std::cout << "Even numbers: " << evens << "\n"; // {2, 4, 8}

    // Reverse array
    nums.reverse();
    std::cout << "Reversed: " << nums << "\n"; // {9, 8, 5, 4, 2, 1}
    return 0;
}
```

### 3. Joining and Random Element Selection

```cpp
#include "array/Array.hpp"
#include <iostream>

int main() {
    Array<std::string> fruits = {"Apple", "Banana"};
    Array<std::string> more_fruits = {"Orange", "Mango"};

    // Join arrays
    fruits.join(more_fruits);
    std::cout << "Fruits: " << fruits << "\n"; // {Apple, Banana, Orange, Mango}

    // Pick a random element
    std::cout << "Random fruit: " << fruits.pick_random() << "\n";

    // Pop the last element
    std::string last = fruits.pop();
    std::cout << "Popped: " << last << "\n";
    return 0;
}
```

### 4. Exception Handling

```cpp
#include "array/Array.hpp"
#include <iostream>

int main() {
    Array<int> arr = {1, 2, 3};
    try {
        int val = arr[5]; // Index out of bounds
    } catch (const ArrayIndexOutOfRange &e) {
        std::cerr << "Caught error: " << e.what() << "\n";
    }
    return 0;
}
```

---

## API Summary

| Method | Description |
| :--- | :--- |
| `size()` | Returns the number of elements in the array. |
| `is_empty()` | Returns `true` if array has no elements. |
| `reserve(capacity)` | Pre-allocates buffer memory. |
| `get(index)` / `operator[](index)` | Bounds-checked element access. |
| `append(data)` | Appends an item to the back (copy or move). |
| `join(other)` | Appends all items from another `Array`. |
| `pop()` | Removes and returns the last element. |
| `sort()` | Sorts items in ascending order. |
| `reverse()` | Reverses elements in-place. |
| `find(data)` | Returns index of item or `-1` if not found. |
| `filter(predicate)` | Returns a new `Array` with matching elements. |
| `pick_random()` | Returns a random element using `<random>`. |
| `begin()` / `end()` | Standard C++ iterator support (range-based for loops). |
