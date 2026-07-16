#pragma once

#include <chrono>
#include <string>

#include "logger/log_level.hpp"

namespace logger {

struct LogRecord {
  std::chrono::system_clock::time_point timestamp;
  LogLevel level = LogLevel::kInfo;
  std::string message;
};

}  // namespace logger
