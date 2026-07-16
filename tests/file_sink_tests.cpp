#include <filesystem>

#include "logger/file_sink.hpp"
#include "test_framework.hpp"
#include "test_utils.hpp"

// Проверяет создание файла и запись одной строки.
TEST(FileSinkCreatesFileAndWritesRecord) {
  auto path = test::MakeTempPath("logger_file_sink");
  {
    auto sink = logger::FileSink::Make(path);
    EXPECT_TRUE(sink);
    EXPECT_TRUE(sink.Value()->Write(test::MakeRecord("Program started")));
  }

  const std::string content = test::ReadFile(path);
  std::filesystem::remove(path);

  EXPECT_CONTAINS(content, "[INFO] Program started");
  EXPECT_CONTAINS(content, "T");
  EXPECT_CONTAINS(content, "Z");
}

// Проверяет режим append при повторном открытии файла.
TEST(FileSinkAppendsToExistingFile) {
  auto path = test::MakeTempPath("logger_file_append");
  {
    auto sink = logger::FileSink::Make(path);
    EXPECT_TRUE(sink);
    EXPECT_TRUE(sink.Value()->Write(test::MakeRecord("first")));
  }
  {
    auto sink = logger::FileSink::Make(path);
    EXPECT_TRUE(sink);
    EXPECT_TRUE(sink.Value()->Write(test::MakeRecord("second")));
  }

  const std::string content = test::ReadFile(path);
  std::filesystem::remove(path);

  EXPECT_CONTAINS(content, "[INFO] first");
  EXPECT_CONTAINS(content, "[INFO] second");
}

// Проверяет экранирование управляющих символов.
TEST(FileSinkEscapesControlCharacters) {
  auto path = test::MakeTempPath("logger_file_escape");
  {
    auto sink = logger::FileSink::Make(path);
    EXPECT_TRUE(sink);
    EXPECT_TRUE(sink.Value()->Write(test::MakeRecord("line1\nline2\t\\")));
  }

  const std::string content = test::ReadFile(path);
  std::filesystem::remove(path);

  EXPECT_CONTAINS(content, "line1\\nline2\\t\\\\");
}

// Проверяет, что записи остаются на отдельных строках.
TEST(FileSinkKeepsMultipleRecordsOnSeparateLines) {
  auto path = test::MakeTempPath("logger_file_lines");
  {
    auto sink = logger::FileSink::Make(path);
    EXPECT_TRUE(sink);
    EXPECT_TRUE(sink.Value()->Write(test::MakeRecord("one")));
    EXPECT_TRUE(sink.Value()->Write(test::MakeRecord("two")));
  }

  const std::string content = test::ReadFile(path);
  std::filesystem::remove(path);

  auto first_newline = content.find('\n');
  EXPECT_TRUE(first_newline != std::string::npos);
  EXPECT_TRUE(content.find('\n', first_newline + 1) != std::string::npos);
}

// Проверяет ошибку при передаче каталога вместо файла.
TEST(FileSinkReportsInvalidPath) {
  auto directory = test::MakeTempPath("logger_dir");
  std::filesystem::create_directories(directory);

  auto sink = logger::FileSink::Make(directory);

  std::filesystem::remove_all(directory);

  EXPECT_FALSE(sink);
  EXPECT_EQ(sink.Error().code, logger::LogError::kInvalidArgument);
  EXPECT_FALSE(sink.Error().message->empty());
}

// Проверяет сохранение UTF-8 байтов без изменения.
TEST(FileSinkPreservesUtf8Bytes) {
  auto path = test::MakeTempPath("logger_utf8");
  const std::string message =
      "\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82";
  {
    auto sink = logger::FileSink::Make(path);
    EXPECT_TRUE(sink);
    EXPECT_TRUE(sink.Value()->Write(test::MakeRecord(message)));
  }

  const std::string content = test::ReadFile(path);
  std::filesystem::remove(path);

  EXPECT_CONTAINS(content, message);
}
