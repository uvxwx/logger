#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "logger/sink.hpp"

namespace logger {

// Пишет форматированные записи в файл в режиме добавления.
class FileSink final : public Sink {
 public:
  // Создает sink и открывает файл в режиме append.
  static Expected<std::unique_ptr<Sink>, Error> Make(
      const std::filesystem::path& path);

  ~FileSink() override = default;

  FileSink(const FileSink&) = delete;
  FileSink& operator=(const FileSink&) = delete;
  FileSink(FileSink&&) = delete;
  FileSink& operator=(FileSink&&) = delete;

  // Пишет одну запись в файл.
  Expected<void, Error> Write(const LogRecord& record) noexcept override;

 private:
  // Инициализирует sink уже открытым потоком.
  FileSink(std::filesystem::path path, std::ofstream stream);

  std::filesystem::path path_;
  std::ofstream stream_;
};

}  // namespace logger
