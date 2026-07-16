#include "logger/record_formatter.hpp"

#include <iomanip>
#include <sstream>

#include "logger/log_level.hpp"

namespace logger {

// Форматирует время в UTC с миллисекундами.
std::string FormatTimestampUtc(
    const std::chrono::system_clock::time_point timestamp) noexcept {
  const auto milliseconds_since_epoch =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          timestamp.time_since_epoch());
  const auto millisecond_part =
      milliseconds_since_epoch % std::chrono::seconds{1};
  const std::time_t unix_time = std::chrono::system_clock::to_time_t(timestamp);

  std::tm utc_time{};
  gmtime_r(&unix_time, &utc_time);

  std::ostringstream stream{};
  stream << std::setfill('0') << std::setw(4) << (utc_time.tm_year + 1900)
         << '-' << std::setw(2) << (utc_time.tm_mon + 1) << '-' << std::setw(2)
         << utc_time.tm_mday << 'T' << std::setw(2) << utc_time.tm_hour << ':'
         << std::setw(2) << utc_time.tm_min << ':' << std::setw(2)
         << utc_time.tm_sec << '.' << std::setw(3) << millisecond_part.count()
         << 'Z';
  return stream.str();
}

// Экранирует управляющие символы для однострочного вывода.
std::string EscapeMessage(const std::string_view message) {
  std::string escaped{};
  escaped.reserve(message.size());

  for (const char ch : message) {
    switch (ch) {
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      case '\\':
        escaped += "\\\\";
        break;
      default:
        escaped.push_back(ch);
        break;
    }
  }

  return escaped;
}

// Собирает итоговую текстовую строку записи.
std::string FormatLogRecordLine(const LogRecord& record) {
  std::ostringstream stream{};
  stream << FormatTimestampUtc(record.timestamp) << " ["
         << ToString(record.level) << "] " << EscapeMessage(record.message);
  return stream.str();
}

}  // namespace logger
