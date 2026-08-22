# MSL String Module

The `String` class provides a high-level, UTF-8 aware, immutable-friendly string type designed for ease of use and full compatibility with the MSL library.

---

## Key Features

- **Rich Text Operations**: `strip_edges()`, `to_lower()`, `to_upper()`, `replace()`, `substr()`.
- **Search & Inspection**: `contains()`, `begins_with()`, `ends_with()`, `find()`, `rfind()`.
- **Parsing & Conversions**: `to_int()`, `to_float()`, `to_utf8_buffer()`, `chr()`.
- **Array Integration**: `split()`, `join()`, and placeholder formatting `format({values})`.
- **UTF-8 Unicode Support**: Decodes UTF-8 codepoints via `unicode_at()`.

---

## Quick Tutorial & Examples

### 1. Construction & Inspections

```cpp
#include "string/String.hpp"
#include <iostream>

int main() {
    String greeting = "Hello, World!";

    std::cout << "Length: " << greeting.length() << "\n";
    std::cout << "Contains 'World': " << std::boolalpha << greeting.contains("World") << "\n";
    std::cout << "Begins with 'Hello': " << greeting.begins_with("Hello") << "\n";
    std::cout << "Ends with '!': " << greeting.ends_with("!") << "\n";
    return 0;
}
```

### 2. Transformations & Formatting

```cpp
#include "string/String.hpp"
#include <iostream>

int main() {
    String text = "  Antigravity Engine  ";

    // Strip whitespace
    String clean = text.strip_edges();
    std::cout << "Clean: '" << clean << "'\n"; // 'Antigravity Engine'

    // Case conversions
    std::cout << "Upper: " << clean.to_upper() << "\n";
    std::cout << "Lower: " << clean.to_lower() << "\n";

    // Replacement
    String replaced = clean.replace("Engine", "MSL");
    std::cout << "Replaced: " << replaced << "\n"; // 'Antigravity MSL'

    // Placeholder formatting
    String template_str = "Player {0} scored {1} points!";
    Array<String> args = {"Alice", "99"};
    std::cout << "Formatted: " << template_str.format(args) << "\n";
    return 0;
}
```

### 3. Splitting and Joining

```cpp
#include "string/String.hpp"
#include <iostream>

int main() {
    String csv = "apple,banana,cherry,orange";

    // Split string into Array<String>
    Array<String> fruits = csv.split(",");
    std::cout << "Fruits array: " << fruits << "\n";

    // Join Array<String> with a separator
    String joined = String(" | ").join(fruits);
    std::cout << "Joined: " << joined << "\n"; // "apple | banana | cherry | orange"
    return 0;
}
```

### 4. Unicode & Type Conversions

```cpp
#include "string/String.hpp"
#include <iostream>

int main() {
    String num_str = "12345";
    int64_t val = num_str.to_int();
    std::cout << "Parsed integer: " << val + 5 << "\n"; // 12350

    // Unicode character generation from codepoint (0x03C0 is Greek letter pi)
    String pi_symbol = String::chr(0x03C0);
    std::cout << "Pi symbol: " << pi_symbol << "\n";
    return 0;
}
```

---

## API Summary

| Method | Description |
| :--- | :--- |
| `length()` | Returns number of bytes in string. |
| `is_empty()` | Returns `true` if string is empty. |
| `contains(what)` | Checks if substring exists. |
| `begins_with(prefix)` / `ends_with(suffix)` | Checks prefix/suffix. |
| `find(what, from)` / `rfind(what, from)` | Searches for substring index. |
| `substr(from, len)` | Extracts substring safely. |
| `strip_edges()` | Trims leading and trailing whitespace. |
| `replace(what, forwhat)` | Replaces occurrences of substring. |
| `to_lower()` / `to_upper()` | Case conversions. |
| `format(Array<String>)` | Replaces `{0}`, `{1}`, etc. with values. |
| `split(delimiter)` | Splits string into `Array<String>`. |
| `join(Array<String>)` | Joins array of strings using current string as separator. |
| `to_int()` / `to_float()` | Numeric parsers. |
| `unicode_at(index)` | Decodes UTF-8 character codepoint. |
| `String::chr(codepoint)` | Creates string from Unicode codepoint. |
| `to_utf8_buffer()` | Converts to `Array<uint8_t>`. |
