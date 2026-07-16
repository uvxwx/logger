#include "logger/udp_sink.hpp"

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <memory>
#include <utility>

#include "logger/charconv_utils.hpp"
#include "logger/serialization.hpp"

namespace logger {
namespace {

// Преобразует порт в строку для getaddrinfo.
std::string MakeServiceName(const std::uint16_t port) {
  return ToCharsString(port);
}

}  // namespace

// Сохраняет адрес назначения и готовый сокет отправки.
UdpSink::UdpSink(std::string host, const std::uint16_t port,
                 const int socket_fd, std::string address_bytes)
    : host_(std::move(host)),
      port_(port),
      socket_fd_(socket_fd),
      address_bytes_(std::move(address_bytes)) {}

// Создает UDP sink и выбирает первый доступный адрес назначения.
Expected<std::unique_ptr<Sink>, Error> UdpSink::Make(std::string host,
                                                     const std::uint16_t port) {
  ::addrinfo hints{};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  ::addrinfo* result = nullptr;
  std::string service = MakeServiceName(port);
  int getaddrinfo_result =
      ::getaddrinfo(host.c_str(), service.c_str(), &hints, &result);
  if (getaddrinfo_result != 0) {
    return Error{
        LogError::kSystemError,
        std::string("getaddrinfo failed: ") +
            ::gai_strerror(getaddrinfo_result),
    };
  }

  int socket_fd = -1;
  std::string address_bytes;
  for (::addrinfo* entry = result; entry != nullptr; entry = entry->ai_next) {
    int fd = ::socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
    if (fd < 0) {
      continue;
    }

    socket_fd = fd;
    address_bytes.assign(reinterpret_cast<const char*>(entry->ai_addr),
                         static_cast<std::size_t>(entry->ai_addrlen));
    break;
  }

  ::freeaddrinfo(result);

  if (socket_fd < 0) {
    return Error{
        LogError::kSystemError,
        "unable to create UDP socket",
    };
  }

  return std::unique_ptr<Sink>(
      new UdpSink(std::move(host), port, socket_fd, std::move(address_bytes)));
}

// Закрывает сокет отправки.
UdpSink::~UdpSink() {
  if (socket_fd_ >= 0) {
    ::close(socket_fd_);
  }
}

// Сериализует запись и отправляет ее одной датаграммой.
Expected<void, Error> UdpSink::Write(const LogRecord& record) noexcept {
  Expected<std::string, Error> serialized = SerializeRecord(record);
  if (!serialized) {
    return serialized.Error();
  }

  auto* address = reinterpret_cast<const ::sockaddr*>(address_bytes_.data());
  auto address_length = static_cast<::socklen_t>(address_bytes_.size());
  ::ssize_t sent =
      ::sendto(socket_fd_, serialized.Value().data(), serialized.Value().size(),
               0, address, address_length);
  if (sent < 0 || static_cast<std::size_t>(sent) != serialized.Value().size()) {
    return Error{
        LogError::kWriteFailed,
        std::string("sendto failed: ") + std::strerror(errno),
    };
  }

  return {};
}

}  // namespace logger
