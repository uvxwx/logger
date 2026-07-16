#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <string>

#include "logger/serialization.hpp"
#include "logger/udp_sink.hpp"
#include "test_framework.hpp"
#include "test_utils.hpp"

namespace {

// RAII-обертка для файлового дескриптора сокета в тесте.
class SocketHandle {
 public:
  // Сохраняет файловый дескриптор тестового сокета.
  explicit SocketHandle(const int fd) : fd_(fd) {}
  // Закрывает тестовый сокет при уничтожении.
  ~SocketHandle() {
    if (fd_ >= 0) {
      ::close(fd_);
    }
  }

  SocketHandle(const SocketHandle&) = delete;
  SocketHandle& operator=(const SocketHandle&) = delete;

  int Get() const noexcept { return fd_; }

 private:
  int fd_ = -1;
};

}  // namespace

// Проверяет отправку сериализованной UDP-датаграммы на loopback.
TEST(UdpSinkSendsSerializedDatagram) {
  const int server_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
  EXPECT_TRUE(server_fd >= 0);
  SocketHandle socket_handle(server_fd);

  ::sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = ::htons(0);
  address.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
  EXPECT_EQ(::bind(socket_handle.Get(), reinterpret_cast<::sockaddr*>(&address),
                   sizeof(address)),
            0);

  ::socklen_t address_length = sizeof(address);
  EXPECT_EQ(
      ::getsockname(socket_handle.Get(),
                    reinterpret_cast<::sockaddr*>(&address), &address_length),
      0);
  const std::uint16_t port = ::ntohs(address.sin_port);

  auto sink = logger::UdpSink::Make("127.0.0.1", port);
  EXPECT_TRUE(sink);
  EXPECT_TRUE(sink.Value()->Write(
      test::MakeRecord("udp-message", logger::LogLevel::kInfo)));

  ::pollfd poll_fd{};
  poll_fd.fd = socket_handle.Get();
  poll_fd.events = POLLIN;
  EXPECT_EQ(::poll(&poll_fd, 1, 1000), 1);

  char buffer[logger::kMaxUdpMessageSize + 32U];
  const ::ssize_t received = ::recvfrom(socket_handle.Get(), buffer,
                                        sizeof(buffer), 0, nullptr, nullptr);
  EXPECT_TRUE(received > 0);

  auto record = logger::DeserializeRecord(
      std::string_view(buffer, static_cast<std::size_t>(received)));
  EXPECT_TRUE(record);
  EXPECT_EQ(record.Value().message, std::string("udp-message"));
  EXPECT_EQ(record.Value().level, logger::LogLevel::kInfo);
}
