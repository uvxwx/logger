#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace logger {

enum class LogError {
  kNone = 0,
  kInvalidArgument,
  kInvalidMessage,
  kWriteFailed,
  kSerializationFailed,
  kSystemError,
};

struct Error {
  // Создает пустую ошибку без текста.
  Error() = default;
  // Создает ошибку только с кодом.
  explicit Error(LogError error_code) : code(error_code) {}
  // Создает ошибку с кодом и пояснением.
  Error(LogError error_code, std::string error_message)
      : code(error_code), message(std::move(error_message)) {}

  LogError code = LogError::kNone;
  std::optional<std::string> message;
};

inline bool operator==(const Error& left, const Error& right) {
  return left.code == right.code && left.message == right.message;
}

// Возвращает стандартное имя кода ошибки.
inline std::string_view ToString(const LogError error) noexcept {
  constexpr std::array<std::string_view, 6> kErrorNames = {
      "none",         "invalid argument",     "invalid message",
      "write failed", "serialization failed", "system error",
  };
  return kErrorNames[static_cast<std::size_t>(error)];
}

// Возвращает подробный текст ошибки.
inline std::string DescribeError(const Error& error) {
  if (error.message.has_value()) {
    return *error.message;
  }

  return std::string{ToString(error.code)};
}

}  // namespace logger
