#pragma once

#include <optional>
#include <string_view>

namespace logger {

enum class LogLevel {
  kDebug = 0,
  kInfo = 1,
  kError = 2,
};

// Возвращает строковое имя уровня.
std::string_view ToString(LogLevel level) noexcept;
// Разбирает уровень без учета регистра.
std::optional<LogLevel> ParseLogLevel(std::string_view value) noexcept;

}  // namespace logger
