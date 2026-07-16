#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include "logger/charconv_utils.hpp"
#include "logger/log_record.hpp"

namespace test {

// Строит уникальный временный путь для теста.
inline std::filesystem::path MakeTempPath(const std::string& prefix) {
  static std::atomic<unsigned long long> counter{0};
  auto id = counter.fetch_add(1);
  auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  return std::filesystem::temp_directory_path() /
         (prefix + "_" + logger::ToCharsString(now) + "_" +
          logger::ToCharsString(id));
}

// Читает весь файл в строку.
inline std::string ReadFile(const std::filesystem::path& path) {
  std::ifstream input(path);
  return std::string((std::istreambuf_iterator<char>(input)),
                     std::istreambuf_iterator<char>());
}

// Создает тестовую запись с фиксированным временем.
inline logger::LogRecord MakeRecord(
    std::string message,
    const logger::LogLevel level = logger::LogLevel::kInfo) {
  return {std::chrono::system_clock::time_point(
              std::chrono::milliseconds(1'721'138'371'482)),
          level, std::move(message)};
}

}  // namespace test
