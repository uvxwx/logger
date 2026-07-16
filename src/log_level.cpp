#include "logger/log_level.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>

namespace logger {
namespace {

constexpr std::array<std::string_view, 3> kLevelNames = {
    "DEBUG",
    "INFO",
    "ERROR",
};

// Переводит строку в нижний регистр для разбора уровня.
std::string ToLowerCopy(std::string_view value) {
  std::string lowered(value);
  std::transform(
      lowered.begin(), lowered.end(), lowered.begin(),
      [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
  return lowered;
}

}  // namespace

// Возвращает каноническое имя уровня.
std::string_view ToString(const LogLevel level) noexcept {
  return kLevelNames[static_cast<std::size_t>(level)];
}

// Разбирает строковое имя уровня логирования.
std::optional<LogLevel> ParseLogLevel(const std::string_view value) noexcept {
  const std::string lowered = ToLowerCopy(value);
  if (lowered == "debug") {
    return LogLevel::kDebug;
  }
  if (lowered == "info") {
    return LogLevel::kInfo;
  }
  if (lowered == "error") {
    return LogLevel::kError;
  }

  return std::nullopt;
}

}  // namespace logger
