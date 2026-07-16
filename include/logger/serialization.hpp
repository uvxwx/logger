#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "logger/error.hpp"
#include "logger/expected.hpp"
#include "logger/log_record.hpp"

namespace logger {

constexpr std::size_t kMaxUdpMessageSize = 60U * 1024U;

// Сериализует запись в бинарный формат.
Expected<std::string, Error> SerializeRecord(const LogRecord& record) noexcept;
// Десериализует запись из бинарного формата.
Expected<LogRecord, Error> DeserializeRecord(std::string_view payload) noexcept;

}  // namespace logger
