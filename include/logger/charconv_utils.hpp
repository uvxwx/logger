#pragma once

#include <array>
#include <charconv>
#include <limits>
#include <string>
#include <string_view>

#include "logger/error.hpp"
#include "logger/expected.hpp"

namespace logger {

// Разбирает целое число из всей строки без лишних символов.
template <typename Integer>
Expected<Integer, Error> ParseInteger(std::string_view text,
                                      std::string_view field_name) {
  if (text.empty()) {
    return Error{
        LogError::kInvalidArgument,
        std::string{field_name} + " is empty",
    };
  }

  Integer value{};
  auto result = std::from_chars(text.data(), text.data() + text.size(), value);
  if (result.ec != std::errc() || result.ptr != text.data() + text.size()) {
    return Error{
        LogError::kInvalidArgument,
        "invalid " + std::string{field_name},
    };
  }

  return value;
}

// Преобразует целое число в строку
template <typename Integer>
std::string ToCharsString(Integer value) {
  std::array<char, std::numeric_limits<Integer>::digits10 + 3> buffer{};
  auto result =
      std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
  return std::string{buffer.data(),
                     static_cast<std::size_t>(result.ptr - buffer.data())};
}

}  // namespace logger
