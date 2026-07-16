#include <iostream>
#include <string_view>

#include "demo_support.hpp"
#include "logger/error.hpp"
#include "logger/file_sink.hpp"

namespace {

// Печатает формат запуска программы.
int PrintUsage(std::ostream& error) {
  error << "Usage: logger_demo <log-file> <minimum-level>\n";
  return 1;
}

}  // namespace

// Разбирает аргументы и запускает файловый демо-логгер.
int main(int argc, char** argv) {
  if (argc != 3) {
    return PrintUsage(std::cerr);
  }

  auto min_level = logger::ParseLogLevel(std::string_view{argv[2]});
  if (!min_level.has_value()) {
    std::cerr << "invalid minimum level: " << argv[2] << '\n';
    return 1;
  }

  auto sink = logger::FileSink::Make(argv[1]);
  if (!sink) {
    std::cerr << "failed to initialize log sink: "
              << logger::DescribeError(sink.Error()) << '\n';
    return 1;
  }

  return logger::RunConsoleLogger(std::cin, std::cout, std::cerr,
                                  std::move(sink.Value()), *min_level);
}
