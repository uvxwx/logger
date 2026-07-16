#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

#include "logger/charconv_utils.hpp"
#include "logger/error.hpp"
#include "logger/record_formatter.hpp"
#include "logger/serialization.hpp"
#include "logger/statistics.hpp"
#include "statistics_receiver_state.hpp"

namespace {

// RAII-обертка для файлового дескриптора сокета.
class SocketHandle {
 public:
  // Сохраняет файловый дескриптор сокета.
  explicit SocketHandle(int fd) : fd_(fd) {}
  // Закрывает дескриптор при уничтожении.
  ~SocketHandle() {
    if (fd_ >= 0) {
      ::close(fd_);
    }
  }

  SocketHandle(const SocketHandle&) = delete;
  SocketHandle& operator=(const SocketHandle&) = delete;

  // Переносит владение файловым дескриптором.
  SocketHandle(SocketHandle&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
  }

  // Переносит владение при присваивании.
  SocketHandle& operator=(SocketHandle&& other) noexcept {
    if (this != &other) {
      if (fd_ >= 0) {
        ::close(fd_);
      }
      fd_ = other.fd_;
      other.fd_ = -1;
    }
    return *this;
  }

  int Get() const noexcept { return fd_; }

 private:
  int fd_ = -1;
};

// Разбирает 16-битное значение, которое не должно быть равно нулю.
logger::Expected<std::uint16_t, logger::Error> ParseNonZeroUint16(
    std::string_view text, std::string_view field_name) {
  auto value = logger::ParseInteger<std::uint16_t>(text, field_name);
  if (!value) {
    return value.Error();
  }

  if (value.Value() == 0) {
    return logger::Error{
        logger::LogError::kInvalidArgument,
        "invalid " + std::string{field_name},
    };
  }

  return value.Value();
}

// Разбирает 64-битное значение, которое не должно быть равно нулю.
logger::Expected<std::uint64_t, logger::Error> ParseNonZeroUint64(
    std::string_view text, std::string_view field_name) {
  auto value = logger::ParseInteger<std::uint64_t>(text, field_name);
  if (!value) {
    return value.Error();
  }

  if (value.Value() == 0) {
    return logger::Error{
        logger::LogError::kInvalidArgument,
        "invalid " + std::string{field_name},
    };
  }

  return value.Value();
}

// Создает и привязывает UDP-сокет к одному из подходящих адресов.
logger::Expected<SocketHandle, logger::Error> BindUdpSocket(
    std::string_view host, std::uint16_t port) {
  ::addrinfo hints{};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  ::addrinfo* result = nullptr;
  std::string service{logger::ToCharsString(port)};
  int getaddrinfo_result = ::getaddrinfo(std::string{host}.c_str(),
                                         service.c_str(), &hints, &result);
  if (getaddrinfo_result != 0) {
    return logger::Error{
        logger::LogError::kSystemError,
        std::string{"getaddrinfo failed: "} +
            ::gai_strerror(getaddrinfo_result),
    };
  }

  // Перебираем все адреса, которые вернул resolver, пока один из bind не
  // сработает.
  std::optional<logger::Error> last_error;
  for (::addrinfo* entry = result; entry != nullptr; entry = entry->ai_next) {
    int fd = ::socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
    if (fd < 0) {
      last_error = logger::Error{
          logger::LogError::kSystemError,
          std::string{"socket failed: "} + std::strerror(errno),
      };
      continue;
    }

    SocketHandle candidate{fd};
    if (::bind(candidate.Get(), entry->ai_addr, entry->ai_addrlen) == 0) {
      ::freeaddrinfo(result);
      return candidate;
    }

    last_error = logger::Error{
        logger::LogError::kSystemError,
        std::string{"bind failed: "} + std::strerror(errno),
    };
  }

  ::freeaddrinfo(result);

  if (last_error.has_value()) {
    return *last_error;
  }

  return logger::Error{
      logger::LogError::kSystemError,
      "unable to bind UDP socket",
  };
}

// Печатает формат запуска программы.
void PrintUsage(std::ostream& error) {
  error << "Usage: statistics_receiver <bind-address> <port> <N> <T-seconds>\n";
}

// Печатает текущую статистику в stdout.
void PrintStatistics(std::ostream& output,
                     const logger::Statistics& statistics) {
  output << logger::FormatStatistics(statistics);
}

}  // namespace

// Принимает UDP-пакеты и периодически печатает статистику.
int main(int argc, char** argv) {
  if (argc != 5) {
    PrintUsage(std::cerr);
    return 1;
  }

  auto port = ParseNonZeroUint16(argv[2], "port");
  if (!port) {
    std::cerr << logger::DescribeError(port.Error()) << '\n';
    return 1;
  }

  auto report_every_n = ParseNonZeroUint64(argv[3], "N");
  if (!report_every_n) {
    std::cerr << logger::DescribeError(report_every_n.Error()) << '\n';
    return 1;
  }

  auto timeout_seconds = ParseNonZeroUint64(argv[4], "T-seconds");
  if (!timeout_seconds) {
    std::cerr << logger::DescribeError(timeout_seconds.Error()) << '\n';
    return 1;
  }

  auto socket_handle = BindUdpSocket(argv[1], port.Value());
  if (!socket_handle) {
    std::cerr << logger::DescribeError(socket_handle.Error()) << '\n';
    return 1;
  }

  logger::Statistics statistics;
  auto timeout = std::chrono::seconds{timeout_seconds.Value()};
  auto next_deadline = std::chrono::steady_clock::now() + timeout;
  std::array<char, logger::kMaxUdpMessageSize + 32U> buffer{};
  logger::StatisticsReportState report_state;

  while (true) {
    auto now = std::chrono::steady_clock::now();
    int timeout_ms = 0;
    if (now < next_deadline) {
      auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
          next_deadline - now);
      timeout_ms = static_cast<int>(remaining.count());
    }

    ::pollfd poll_fd{};
    poll_fd.fd = socket_handle.Value().Get();
    poll_fd.events = POLLIN;

    int poll_result = -1;
    do {
      poll_result = ::poll(&poll_fd, 1, timeout_ms);
    } while (poll_result < 0 && errno == EINTR);

    if (poll_result < 0) {
      std::cerr << "poll failed: " << std::strerror(errno) << '\n';
      return 1;
    }

    if (poll_result == 0) {
      statistics.ExpireOld(std::chrono::system_clock::now());
      if (statistics.IsDirty()) {
        PrintStatistics(std::cout, statistics);
        statistics.ClearDirty();
        report_state.OnReport();
      }
      next_deadline = std::chrono::steady_clock::now() + timeout;
      continue;
    }

    if ((poll_fd.revents & POLLIN) == 0) {
      continue;
    }

    ::ssize_t received = -1;
    do {
      received = ::recvfrom(socket_handle.Value().Get(), buffer.data(),
                            buffer.size(), 0, nullptr, nullptr);
    } while (received < 0 && errno == EINTR);

    if (received < 0) {
      std::cerr << "recvfrom failed: " << std::strerror(errno) << '\n';
      continue;
    }

    auto payload =
        std::string_view{buffer.data(), static_cast<std::size_t>(received)};
    auto result = logger::DeserializeRecord(payload);
    if (!result) {
      std::cerr << "malformed datagram: "
                << logger::DescribeError(result.Error()) << '\n';
      continue;
    }

    std::cout << logger::FormatLogRecordLine(result.Value()) << '\n';
    statistics.AddMessage(result.Value(), std::chrono::system_clock::now());
    statistics.ExpireOld(std::chrono::system_clock::now());
    report_state.OnMessage();

    if (report_state.ShouldReportByCount(report_every_n.Value())) {
      PrintStatistics(std::cout, statistics);
      statistics.ClearDirty();
      report_state.OnReport();
      next_deadline = std::chrono::steady_clock::now() + timeout;
    }
  }
}
