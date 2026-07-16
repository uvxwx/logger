#include <iostream>
#include <string>
#include <string_view>

#include "demo_support.hpp"
#include "logger/charconv_utils.hpp"
#include "logger/error.hpp"
#include "logger/udp_sink.hpp"

namespace {

// Печатает формат запуска программы.
int PrintUsage(std::ostream& error) {
  error << "Usage: udp_logger_demo <host> <port> <minimum-level>\n";
  return 1;
}

// Разбирает UDP-порт, который не должен быть равен нулю.
logger::Expected<std::uint16_t, logger::Error> ParseNonZeroPort(
    std::string_view text) {
  auto port = logger::ParseInteger<std::uint16_t>(text, "port");
  if (!port) {
    return port.Error();
  }

  if (port.Value() == 0) {
    return logger::Error{
        logger::LogError::kInvalidArgument,
        "invalid port",
    };
  }

  return port.Value();
}

}  // namespace

// Разбирает аргументы и запускает UDP демо-логгер.
int main(int argc, char** argv) {
  if (argc != 4) {
    return PrintUsage(std::cerr);
  }

  auto port = ParseNonZeroPort(argv[2]);
  if (!port) {
    std::cerr << logger::DescribeError(port.Error()) << ": " << argv[2] << '\n';
    return 1;
  }

  auto min_level = logger::ParseLogLevel(std::string_view{argv[3]});
  if (!min_level.has_value()) {
    std::cerr << "invalid minimum level: " << argv[3] << '\n';
    return 1;
  }

  auto sink = logger::UdpSink::Make(argv[1], port.Value());
  if (!sink) {
    std::cerr << "failed to initialize UDP sink: "
              << logger::DescribeError(sink.Error()) << '\n';
    return 1;
  }

  return logger::RunConsoleLogger(std::cin, std::cout, std::cerr,
                                  std::move(sink.Value()), *min_level);
}
