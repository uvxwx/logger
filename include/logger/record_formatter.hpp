#pragma once

#include <string>

#include "logger/log_record.hpp"

namespace logger {

// Форматирует время в UTC для текстового лога.
std::string FormatTimestampUtc(
    std::chrono::system_clock::time_point timestamp) noexcept;
// Экранирует управляющие символы сообщения.
std::string EscapeMessage(std::string_view message);
// Собирает полную строку записи лога.
std::string FormatLogRecordLine(const LogRecord& record);

}  // namespace logger
