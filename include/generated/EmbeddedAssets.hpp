#pragma once

#include <cstddef>
#include <string>

namespace EmbeddedAssets {
  const unsigned char* get(const std::string& path, size_t& outSize);
  bool has(const std::string& path);

  const unsigned char* getDefaultFontData(size_t& outSize);
  const unsigned char* getDefaultLogoData(size_t& outSize);
}
